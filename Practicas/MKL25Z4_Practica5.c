#include <MKL25Z4.h>

/* LCD 4 bit */
#define RS 0x04
#define RW 0x10
#define EN 0x20

#define MENU_RESET_TIME 120000
#define FAN_NORMAL_DURATION 10000  // 10 segundos
#define FAN_ALARM_DURATION 60000   // 1 minuto


volatile uint32_t lastKeyPressTime = 0;
volatile float desiredTemperature = 0.0;
volatile uint8_t inputPosition = 0;
volatile uint8_t temperatureInput[2] = {0};
volatile uint8_t systemState = 0;  // 0=normal, 1=alarm
volatile uint8_t idleMode = 0;
volatile uint8_t fanActive = 0;
volatile uint32_t fanStartTime = 0;
volatile uint32_t millis = 0;
volatile uint8_t idleMessageShown = 0;


void delayMs(int n);
void delayUs(int n);
void keypad_init(void);
char keypad_getkey(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void LCD_print(char *str);
void send_nibble(unsigned char nibble);
char convert_key(unsigned char key);
void LED_init(void);
void show_main_menu();
void process_temperature_input(char key);
void update_temperature_display();
void ADC_init(void);
void update_actual_temperature_display();
void start_fan(uint32_t duration);
void stop_fan(void);
void check_fan_timer(void);
void SysTick_Handler(void);
uint16_t read_potentiometer(void);

int main(void)
{
    SIM->SCGC6 |= 0x01000000;
    SIM->SOPT2 |= 0x01000000;
    TPM0->SC = 0;
    TPM0->SC = 0x02;
    TPM0->MOD = 0x2000;
    TPM0->SC |= 0x80;
    TPM0->CNT = 0;
    TPM0->SC |= 0x08;
    SysTick->LOAD = 20970 - 1; // 48MHz / 1000 = 48000, pero como F_BUS=21 MHz -> 21000 ciclos ≈ 1ms
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07; // Enable SysTick with interrupt


    keypad_init();
    LCD_init();
    LED_init();
    ADC_init();

    show_main_menu();

    while (1) {
        unsigned char key = keypad_getkey();
        char keyChar = convert_key(key);

        if (key) {
            lastKeyPressTime = TPM0->CNT;

            if (idleMode) {
				if (keyChar == 'B') {
					LCD_command(1);
					LCD_command(0x80);
					LCD_print("IDLE MODE OFF");
					idleMode = 0;
					idleMessageShown = 0;
					delayMs(500);
					stop_fan();  // Por si acaso
					show_main_menu();  // Mostrar menú principal
				}
				continue;  
			}

            if (keyChar == '*') {
                temperatureInput[0] = 0;
                temperatureInput[1] = 0;
                inputPosition = 0;
                desiredTemperature = 0.0;
                systemState = 0;
                idleMode = 0;
                stop_fan();
                show_main_menu();
            }
            else if (keyChar >= '0' && keyChar <= '9') {
                process_temperature_input(keyChar);
            }
            else if (keyChar == 'A') {
                idleMode = 1;
                LCD_command(1);
                idleMessageShown = 0;
                stop_fan();
            }
            else if (keyChar == '#') {
                desiredTemperature = temperatureInput[0] * 10 + temperatureInput[1];
                if (desiredTemperature >= 0 && desiredTemperature <= 50) {
                    LCD_command(1);
                    LCD_command(0x80);
                    LCD_print("Set Temp: ");
                    LCD_data(temperatureInput[0] + '0');
                    LCD_data(temperatureInput[1] + '0');
                    LCD_print(" C     ");
                }
            }
        }

        if (idleMode && !idleMessageShown) {
			LCD_command(1);
			LCD_command(0x80);
			LCD_print("IDLE MODE ON");
			update_actual_temperature_display();
			idleMessageShown = 1;
			continue;
		}

        // Actualización de temperatura y ventilador
        if (!idleMode && desiredTemperature > 0) {
            update_actual_temperature_display();
            float currentTemp = (read_potentiometer() / 1023.0f) * 50.0f;
            systemState = (currentTemp > desiredTemperature) ? 1 : 0;

            if (!fanActive) {
				uint32_t requiredDuration = (systemState == 1) ? FAN_ALARM_DURATION : FAN_NORMAL_DURATION;
				start_fan(requiredDuration);
			}

            check_fan_timer();
        }

        if ((millis - lastKeyPressTime) > MENU_RESET_TIME) {
            show_main_menu();
            lastKeyPressTime = millis;
        }

        delayMs(100);
    }

}

void SysTick_Handler(void) {
    millis++;
}

void start_fan(uint32_t duration) {
    if (idleMode) return;

    PTB->PCOR = 0x40000; // Encender LED
    fanActive = 1;
    fanStartTime = millis;
}

void stop_fan(void) {
    PTB->PSOR = 0x40000; // Apagar LED
    fanActive = 0;
}

void check_fan_timer(void) {
    if (fanActive) {
        uint32_t duration = (systemState == 1) ? FAN_ALARM_DURATION : FAN_NORMAL_DURATION;
        if ((millis - fanStartTime) >= duration) {
            stop_fan();
        }
    }
}

void process_temperature_input(char key) {
    if (inputPosition < 2) {
        temperatureInput[inputPosition] = key - '0';
        inputPosition++;
        update_temperature_display();
    }
}

void update_temperature_display() {
    LCD_command(0x80);
    LCD_print("Set Temp: ");

    if (inputPosition > 0) {
        LCD_data(temperatureInput[0] + '0');
        delayMs(100);
    } else {
        LCD_data('0');
    }

    if (inputPosition > 1) {
        LCD_data(temperatureInput[1] + '0');
        delayMs(100);
    } else {
        LCD_data('0');
    }

    LCD_print(" C      ");
}

void show_main_menu() {
    LCD_command(1);
    delayMs(10);

    LCD_command(0x80);
    update_temperature_display();
}

void update_actual_temperature_display() {
    uint16_t adcValue = read_potentiometer();
    #define ADC_MIN 0    // Valor ADC a 0°C
    #define ADC_MAX 1023 // Valor ADC a 50°C

    float currentTemp = ((adcValue - ADC_MIN) / (float)(ADC_MAX - ADC_MIN)) * 50.0;
	
    if(currentTemp < 0) currentTemp = 0;
    if(currentTemp > 50) currentTemp = 50;

    LCD_command(0xC0);  
    LCD_print("Actual:    ");  

    int tempInt = (int)(currentTemp + 0.5);

    LCD_command(0xC7); 

    if(tempInt < 10) {
        LCD_data('0');
        LCD_data(tempInt + '0');
    }
    
    else {
        LCD_data((tempInt/10) + '0');
        LCD_data((tempInt%10) + '0');
    }

    LCD_print(" C    ");
}

void ADC_init(void) {
    // Enable clock to ADC0 and PORT A
    SIM->SCGC6 |= 0x8000000;  // ADC0 clock
    SIM->SCGC5 |= 0x200;      // PORT A clock

    // Configure ADC0
    ADC0->CFG1 = 0x40;        // 10-bit mode, bus clock/2
    ADC0->SC2 = 0;            // Software trigger

    // Configure PTB0 (ADC0_SE8) as analog input
    PORTA->PCR[0] = 0;        // Disable digital pin function
}

uint16_t read_potentiometer(void) {
    // Select ADC channel 8 (PTA0) con promedio de 4 muestras
    uint32_t sum = 0;
    for(int i=0; i<4; i++) {
        ADC0->SC1[0] = 0x08;  // Channel 8
        while(!(ADC0->SC1[0] & 0x80)) {} // Wait for conversion
        sum += ADC0->R[0];
        delayMs(1);  
    }
    return sum; 
}

void LED_init(void)
{
    SIM->SCGC5 |= 0x400; /* enable clock to Port B */
    SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
    PORTB->PCR[18] = 0x100; /* make PTB18 pin as GPIO */
    PTB->PDDR |= 0x40000; /* make PTB18 as output pin */
    PTB->PSOR |= 0x40000; /* turn off red LED */
    PORTB->PCR[19] = 0x100; /* make PTB19 pin as GPIO */
    PTB->PDDR |= 0x80000; /* make PTB19 as output pin */
    PTB->PSOR |= 0x80000; /* turn off green LED */
    PORTD->PCR[1] = 0x100; /* make PTD1 pin as GPIO */
    PTD->PDDR |= 0x02; /* make PTD1 as output pin */
    PTD->PSOR |= 0x02; /* turn off blue LED */
}

void LCD_print(char *str) {
    while (*str) LCD_data(*str++);
}

char convert_key(unsigned char key)
{
    switch (key)
    {
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return 'A';
    case 5: return '4';
    case 6: return '5';
    case 7: return '6';
    case 8: return 'B';
    case 9: return '7';
    case 10: return '8';
    case 11: return '9';
    case 12: return 'C';
    case 13: return '*';
    case 14: return '0';
    case 15: return '#';
    case 16: return 'D';
    default: return ' ';
    }
}

void keypad_init(void)
{
    SIM->SCGC5 |= 0x0800;

    for (int i = 0; i < 8; i++)
        PORTC->PCR[i] = 0x103;

    PTC->PDDR |= 0x0F;
    PTC->PSOR = 0x0F;
}

char keypad_getkey(void)
{
    int row, col;
    const char row_select[] = {0x01, 0x02, 0x04, 0x08};

    PTC->PSOR = 0xFF;
    delayUs(2);
    col = PTC->PDIR & 0xF0;
    if (col == 0x0F) return 0;

    for (row = 0; row < 4; row++)
    {
        PTC->PSOR = 0xFF;
        PTC->PCOR = row_select[row];
        delayUs(2);
        col = PTC->PDIR & 0xF0;
        if (col != 0xF0) break;
    }

    PTC->PSOR = 0xFF;
    if (row == 4) return 0;

    if (col == 0xE0) return row * 4 + 1;
    if (col == 0xD0) return row * 4 + 2;
    if (col == 0xB0) return row * 4 + 3;
    if (col == 0x70) return row * 4 + 4;

    return 0;
}

void LCD_init(void)
{
    SIM->SCGC5 |= 0x1000;
    for (int i = 4; i <= 7; i++)
        PORTD->PCR[i] = 0x100;
    PTD->PDDR |= 0xF0;

    SIM->SCGC5 |= 0x0200;
    PORTA->PCR[2] = 0x100;
    PORTA->PCR[4] = 0x100;
    PORTA->PCR[5] = 0x100;
    PTA->PDDR |= 0x34;

    delayMs(30);

    PTA->PCOR = RS | RW;
    send_nibble(0x03);
    delayMs(10);
    send_nibble(0x03);
    delayMs(1);
    send_nibble(0x03);
    send_nibble(0x02);

    LCD_command(0x28);
    LCD_command(0x06);
    LCD_command(0x01);
    LCD_command(0x0F);
}

void LCD_command(unsigned char command)
{
    PTA->PCOR = RS | RW;
    send_nibble(command >> 4);
    send_nibble(command & 0x0F);
    if (command < 4)
        delayMs(4);
    else
        delayMs(1);
}

void LCD_data(unsigned char data)
{
    PTA->PSOR = RS;
    PTA->PCOR = RW;
    send_nibble(data >> 4);
    send_nibble(data & 0x0F);
    delayMs(1);
}

void send_nibble(unsigned char nibble)
{
    PTD->PDOR &= ~0xF0;
    PTD->PDOR |= (nibble & 0x0F) << 4;
    PTA->PSOR = EN;
    delayMs(1);
    PTA->PCOR = EN;
}

void delayMs(int n)
{
    for (int i = 0; i < n; i++)
    {
        while ((TPM0->SC & 0x80) == 0) {}
        TPM0->SC |= 0x80;
    }
}

void delayUs(int n)
{
    while ((TPM0->SC & 0x80) == 0) {}
    TPM0->SC |= 0x80;
}
