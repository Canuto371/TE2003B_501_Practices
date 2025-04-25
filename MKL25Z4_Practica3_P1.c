#include <MKL25Z4.h>

/* LCD 4 bit */
#define RS 0x04
#define RW 0x10
#define EN 0x20

#define MENU_RESET_TIME 3000

volatile uint32_t lastKeyPressTime = 0;

void delayMs(int n);
void delayUs(int n);
void keypad_init(void);
char keypad_getkey(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void send_nibble(unsigned char nibble);
char convert_key(unsigned char key);
void LED_init(void);
void show_main_menu();

int main(void)
{
	SIM->SCGC6 |= 0x01000000;
	    SIM->SOPT2 |= 0x01000000;
	    TPM0->SC = 0;
	    TPM0->SC = 0x02;
	    TPM0->MOD = 0x2000;
	    TPM0->SC |= 0x80;
	    TPM0->SC |= 0x08;

	    keypad_init();
	    LCD_init();
	    LED_init();
	    show_main_menu();

	        while (1) {
	            unsigned char key = keypad_getkey();

	            if (key) {
	                lastKeyPressTime = TPM0->CNT;  // Guardar tiempo de la última tecla
	                convert_key(key);              // Mostrar mensaje del LED
	            }

	            // Si pasan 3 segundos sin pulsar teclas, vuelve al menú
	            if ((TPM0->CNT - lastKeyPressTime) > MENU_RESET_TIME) {
	                show_main_menu();
	                lastKeyPressTime = TPM0->CNT;  // Resetear contador
	            }
	        }
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

void show_main_menu() {
    LCD_command(1);
    delayMs(10);

    LCD_command(0x80);
    LCD_data('P');
    LCD_data('R');
    LCD_data('E');
    LCD_data('S');
    LCD_data('S');
    LCD_data(' ');
    LCD_data('B');
    LCD_data('U');
    LCD_data('T');
    LCD_data('T');
    LCD_data('O');
    LCD_data('N');

    LCD_command(0xC0);
    LCD_data('R');
    LCD_data(':');
    LCD_data('1');
    LCD_data(' ');
    LCD_data('B');
    LCD_data(':');
    LCD_data('2');
    LCD_data(' ');
    LCD_data('G');
    LCD_data(':');
    LCD_data('3');
}

char convert_key(unsigned char key)
{
	LCD_command(1);
    switch (key)
    {
    case 1:
			delayMs(500);
			PTB->PCOR = 0x40000;
			LCD_command(0x80);
			LCD_data('R');
			LCD_data('E');
			LCD_data('D');
			LCD_command(0xC0);
			LCD_data('L');
			LCD_data('E');
			LCD_data('D');
			LCD_data(' ');
			LCD_data('I');
			LCD_data('S');
			LCD_data(' ');
			LCD_data('O');
			LCD_data('N');
			delayMs(1000);
			LCD_command(1);
			PTB->PSOR = 0x40000;
			break;
    case 2:
    		delayMs(500);
    		PTD->PCOR = 0x02;
    		LCD_command(0x80);
    		LCD_data('B');
    		LCD_data('L');
   			LCD_data('U');
   			LCD_data('E');
   			LCD_command(0xC0);
			LCD_data('L');
			LCD_data('E');
			LCD_data('D');
			LCD_data(' ');
			LCD_data('I');
			LCD_data('S');
			LCD_data(' ');
			LCD_data('O');
			LCD_data('N');
			delayMs(1000);
			LCD_command(1);
			PTD->PSOR = 0x02;
			break;
    case 3:
        	delayMs(500);
			PTB->PCOR = 0x80000;
        	LCD_command(0x80);
        	LCD_data('G');
        	LCD_data('R');
       		LCD_data('E');
       		LCD_data('E');
       		LCD_data('N');
       		LCD_command(0xC0);
			LCD_data('L');
			LCD_data('E');
			LCD_data('D');
			LCD_data(' ');
			LCD_data('I');
			LCD_data('S');
			LCD_data(' ');
			LCD_data('O');
			LCD_data('N');
			delayMs(1000);
			LCD_command(1);
			PTB->PSOR = 0x80000;
			break;
    default: return ' ';
    }
    delayMs(500);
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
