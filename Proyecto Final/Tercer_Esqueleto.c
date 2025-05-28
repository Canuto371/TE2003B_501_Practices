//Primer esqueleto Proyecto Final FruitXpress
//Interfáz de usuario

#include <MKL25Z4.h>
#include <stdint.h>
#include <stdbool.h>

//Puertos LCD
#define RS 0x04
#define RW 0x10
#define EN 0x20

//pins y const hcsr04 (sens dist)
#define TRIG_PIN 1
#define ECHO_PIN 12
#define UMBRAL_CM 10
#define NUM_SENSORS 3

//Tiempo para reset
#define MENU_RESET_TIME 10000

//Variables volátiles
volatile uint32_t lastKeyPressTime = 0;
volatile float desiredTemperature = 0.0;
volatile uint8_t inputPosition = 0;
volatile uint8_t temperatureInput[2] = {0};
volatile uint8_t displayMode = 0;
volatile uint8_t invalid_input = 0;

//Tiempos delays
void delayMs(int n);
void delayUs(int n);

//Funciones keypad
void keypad_init(void);
char keypad_getkey(void);
char convert_key(unsigned char key);

//Sensor Ultrasónico HCSR04
void SysTick_Init(void);
void PORT_Configure(void);
void TPM0_Init(void);
uint32_t micros(void);
uint32_t medir_distancia_cm(void);

//Funciones LCD
void LCD_init(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_print(char *str);
void send_nibble(unsigned char nibble);

//Funciones ADC
void ADC_init(void);

//Funciones LED
void LED_init(void);

//Funciones extra
void show_main_menu();
void show_selection_menu();

//Interrupciones
volatile int paused = 0;
volatile int display_paused = 0;
int count = 0;

//Manejo nivel de tanques
uint8_t check_level(int n);
void Ultrasonic_Init(void);

//Definición y configuración pines : Sensor Ultrasónico
typedef struct {
    uint8_t trig_pin;
    uint8_t echo_pin;
    PORT_Type *echo_port;
    GPIO_Type *echo_gpio;
} UltrasonicSensor;

// Configuración de los 3 sensores
UltrasonicSensor sensors[3] = {
    {1, 12, PORTA, GPIOA},  // Sensor 1 (Naranja)
    {2, 13, PORTA, GPIOA},  // Sensor 2 (Uva)
    {3, 14, PORTA, GPIOA}   // Sensor 3 (Fresa)
};

//Función main
int main(void){

	//Interrupciones
	__enable_irq();

	//Inicializaciones
	keypad_init();
	LCD_init();
	LED_init();
	ADC_init();
	SystemCoreClockUpdate();
	SysTick_Init();
	PORT_Configure();
	TPM0_Init();

	//Menú principal
	show_main_menu();
}

//Inicializaciones ////////////////////////////////////////////////
void keypad_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    for (int i = 0; i < 4; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1);
    }
    for (int i = 4; i < 8; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1)
                       | PORT_PCR_PE_MASK
                       | PORT_PCR_PS(1);
    }

    PTC->PDDR |= 0x0F;
    PTC->PSOR |= 0x0F;
}

void LCD_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTA_MASK;

    for (int i = 4; i <= 7; i++) {
        PORTD->PCR[i] = PORT_PCR_MUX(1);
    }
    PTD->PDDR |= 0xF0;

    PORTA->PCR[2] = PORT_PCR_MUX(1);
    PORTA->PCR[4] = PORT_PCR_MUX(1);
    PORTA->PCR[5] = PORT_PCR_MUX(1);
    PTA->PDDR   |= (RS | RW | EN);

    delayMs(30);

    PTA->PCOR = RS | RW;
    send_nibble(0x03);
    delayMs(10);
    send_nibble(0x03);
    delayUs(100);
    send_nibble(0x03);
    delayUs(100);
    send_nibble(0x02);

    LCD_command(0x28);
    LCD_command(0x06);
    LCD_command(0x01);
    LCD_command(0x0F);
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

void SysTick_Init(void) {
    SysTick->LOAD = (SystemCoreClock / 1000000U) - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk //48MHz
                  | SysTick_CTRL_ENABLE_Msk;
}

void TPM0_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); //48 MHz
    TPM0->SC = 0;
    TPM0->CNT = 0;
    TPM0->SC = TPM_SC_PS(0) | TPM_SC_CMOD(1);
}

void Ultrasonic_Init(void) {
    // Habilitar reloj para los puertos
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;

    for (int i = 0; i < NUM_SENSORS; i++) {
        // Configurar pin de trigger como salida
        PORTD->PCR[sensors[i].trig_pin] = PORT_PCR_MUX(1);
        PTD->PDDR |= (1U << sensors[i].trig_pin);

        // Configurar pin de echo como entrada con interrupción
        sensors[i].echo_port->PCR[sensors[i].echo_pin] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    }

    PORTA->PCR[sensors[0].echo_pin] |= PORT_PCR_IRQC(0xA); // Interrupción en flanco de bajada
    NVIC->ISER[0] |= (1 << 30); // Habilitar interrupción PORTA
}

//Delays y Tiempos////////////////////////////////////////////////
void delayMs(int n) {
    for (int i = 0; i < n; i++) {
        delayUs(1000);
    }
}

void delayUs(int n) {
    uint16_t start = (uint16_t)TPM0->CNT;
    uint16_t target = start + (uint16_t)(n * 48);
    while ((uint16_t)(TPM0->CNT - start) < (uint16_t)(n * 48)) {}
}

uint32_t micros(void) {
    static volatile uint32_t usTicks = 0;
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        usTicks++;
    }
    return usTicks;
}

//Keypad ////////////////////////////////////////////////
char keypad_getkey(void) {
    int row;
    uint8_t col;
    const uint8_t row_select[4] = {0x01, 0x02, 0x04, 0x08};

    PTC->PSOR = 0x0F;
    delayUs(2);
    col = PTC->PDIR & 0xF0;
    if (col == 0xF0) return 0;

    for (row = 0; row < 4; row++) {
        PTC->PSOR = 0x0F;
        PTC->PCOR = row_select[row];
        delayUs(2);
        col = PTC->PDIR & 0xF0;
        if (col != 0xF0) break;
    }
    PTC->PSOR = 0x0F;

    if (row == 4) return 0;
    if (col == 0xE0) return row * 4 + 1;
    if (col == 0xD0) return row * 4 + 2;
    if (col == 0xB0) return row * 4 + 3;
    if (col == 0x70) return row * 4 + 4;
    return 0;
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

//LCD ////////////////////////////////////////////////
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

void LCD_print(char *str) {
    while (*str) LCD_data(*str++);
}

//Manejo de Interrupciones
void PORTA_IRQHandler(void) {
    if (PORTA->ISFR & (1 << 1)) {
        invalid_input = 1;
        PORTA->ISFR = (1 << 1);
    }
}

//Checkeo de tanques
uint8_t check_level(int n){
	if(n==1){
		//naranja
	} else if(n==2){
		//uva
	} else if(n==3){
		//fresa
	} else {

	}
};

void PORT_Configure(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK
                | SIM_SCGC5_PORTB_MASK
                | SIM_SCGC5_PORTC_MASK
                | SIM_SCGC5_PORTD_MASK;

    PORTD->PCR[TRIG_PIN] = PORT_PCR_MUX(1)
                         | PORT_PCR_PE_MASK
                         | PORT_PCR_PS(0);
    GPIOD->PDDR |= (1U << TRIG_PIN); //salida
    //GPIOD->PDDR &= ~(1U << TRIG_PIN); //entrada

    PORTA->PCR[ECHO_PIN] = PORT_PCR_MUX(1)
                         | PORT_PCR_PE_MASK
                         | PORT_PCR_PS(0);
    GPIOA->PDDR &= ~(1U << ECHO_PIN); //entrada

    PORTA->PCR[1] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
	PTA->PDDR &= ~(1 << 1);
	NVIC->ISER[0] |= (1 << 30);

}

uint32_t medir_distancia_cm(void) {
    uint32_t startTime = 0, endTime = 0, pulseWidth = 0;
    uint32_t distance_cm = 0;

    GPIOD->PCOR |= (1U << TRIG_PIN);
    delayUs(2);

    //pulso (10us) -> data sheet hcsr04
    GPIOD->PSOR |= (1U << TRIG_PIN);//1
    delayUs(10);
    GPIOD->PCOR |= (1U << TRIG_PIN);//0

    //esperar posedge en echo
    uint32_t timeout = micros();
    while (!(GPIOA->PDIR & (1U << ECHO_PIN))) {
        if ((micros() - timeout) > 30000U) {
            return 0xFFFFFFFFU;
        }
    }
    startTime = micros();

    //esperar negedge
    timeout = micros();
    while (GPIOA->PDIR & (1U << ECHO_PIN)) {
        if ((micros() - timeout) > 30000U) {
            return 0xFFFFFFFFU;
        }
    }
    endTime = micros();

    pulseWidth = endTime - startTime; //ancho de pulso en us
    //distance_cm = pulseWidth / 58U; //dist en cm (t/58) -> data sheet hcsr04
    distance_cm = (uint32_t)(2.2f * (pulseWidth / 58.0f) + 2.5f);

    return distance_cm;
}

//Menues ////////////////////////////////////////////////
void show_main_menu() {
    LCD_command(1);
    delayMs(10);

    LCD_command(0x80);
    LCD_print("¡FruitXpress!");
    LCD_command(0xC0);
    LCD_print("Oprima # para continuar");
	while (1){
		unsigned char key = keypad_getkey();
		char keyChar = convert_key(key);
    	if (key){
    		lastKeyPressTime = TPM0->CNT;
    		if (keyChar == '#'){
    			LCD_command(1);
				delayMs(10);

				LCD_command(0x80);
				LCD_print("Elige un sabor: ");
				delayMs(1000);
    			show_selection_menu();
    		} else if (keyChar == '*') {
    			//Reseteo de variables********
    			PTB->PSOR = 0x40000;
    			show_main_menu();
    		} else {
    			show_main_menu();
    		}
		}
	}
}

void show_selection_menu(){
    LCD_command(1);
    delayMs(10);

    while (1){
    	if(!paused){
    		LCD_command(0x80);
			LCD_print("1: Naranja");
			LCD_command(0xC0);
			LCD_print("2: Uva");
			delayMs(1000);
			LCD_command(1);
			LCD_command(0x80);
			LCD_print("3: Fresa");
			LCD_command(0xC0);
			LCD_print("4: Cancelar");
			delayMs(1000);

			unsigned char key = keypad_getkey();
			if (key) {
				char keyChar = convert_key(key);
				lastKeyPressTime = TPM0->CNT;

				if (keyChar == '1') {
					LCD_command(1);
					LCD_print("Naranja seleccionada");
					delayMs(1000);
					check_level(1);
					break;
				}
				else if (keyChar == '2') {
					LCD_command(1);
					LCD_print("Uva seleccionada");
					delayMs(1000);
					check_level(2);
					break;
				}
				else if (keyChar == '3') {
					LCD_command(1);
					LCD_print("Fresa seleccionada");
					delayMs(1000);
					check_level(3);
					break;
				}
				else if (keyChar == '4') {
					show_main_menu();
					return;
				}
				else {
					// Disparar interrupción por entrada inválida
					NVIC->ICPR[0] = (1 << 30);
					NVIC->ISER[0] |= (1 << 30);
					invalid_input = 1;
				}

				// Manejo de entrada inválida
				if(invalid_input) {
					LCD_command(1);
					LCD_command(0x80);
					LCD_print("¡Entrada invalida!");
					LCD_command(0xC0);
					LCD_print("Intente de nuevo");
					delayMs(2000);
					invalid_input = 0;
					NVIC->ICER[0] |= (1 << 30);
				}
			}
    	} else {
    		if (!display_paused) {
				LCD_command(1);
				LCD_command(0x80);
				LCD_print("PAUSADO");
				LCD_command(0xC0);
				LCD_print("* para continuar");
				display_paused = 1;
			}

			unsigned char key = keypad_getkey();
			if (key) {
				char keyChar = convert_key(key);
				if (keyChar == '*') {
					paused = 0;
					display_paused = 0;
					LCD_command(1);
				}
			}
    	}
    }
}

//Manejo de bombas
//************************************************************










