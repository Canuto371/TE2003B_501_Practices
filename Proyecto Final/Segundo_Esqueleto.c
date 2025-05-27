//Segundo esqueleto Proyecto Final FruitXpress
//Interfáz de usuario 
//Con interrupciones

#include <MKL25Z4.h>

//Puertos LCD
#define RS 0x04
#define RW 0x10
#define EN 0x20

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
void process_temperature_input(char key);
void update_temperature_display();
void clear_line(uint8_t line);
uint16_t read_potentiometer(void);
void update_actual_temperature_display();
void show_actual_temp();
void check_temperature_comparison();

//Interrupciones
volatile int paused = 0;
volatile int display_paused = 0;
int count = 0;


//Función main
int main(void){
	SIM->SCGC6 |= 0x01000000;
	SIM->SOPT2 |= 0x01000000;
	TPM0->SC = 0;
	TPM0->SC = 0x02;
	TPM0->MOD = 0x2000;
	TPM0->SC |= 0x80;
	TPM0->SC |= 0x08;

	//Interrupciones
	PORTA->PCR[1] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
	PTA->PDDR &= ~(1 << 1);
	NVIC->ISER[0] |= (1 << 30);
	__enable_irq();

	//Inicializaciones
	keypad_init();
	LCD_init();
	LED_init();
	ADC_init();

	//Menú principal
	show_main_menu();
}

//Inicializaciones ////////////////////////////////////////////////
void keypad_init(void)
{
    SIM->SCGC5 |= 0x0800;

    for (int i = 0; i < 8; i++)
        PORTC->PCR[i] = 0x103;

    PTC->PDDR |= 0x0F;
    PTC->PSOR = 0x0F;
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

//Delays ////////////////////////////////////////////////
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

//Keypad ////////////////////////////////////////////////
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

//Main Menu ////////////////////////////////////////////////
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
					break;
				}
				else if (keyChar == '2') {
					LCD_command(1);
					LCD_print("Uva seleccionada");
					delayMs(1000);
					break;
				}
				else if (keyChar == '3') {
					LCD_command(1);
					LCD_print("Fresa seleccionada");
					delayMs(1000);
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