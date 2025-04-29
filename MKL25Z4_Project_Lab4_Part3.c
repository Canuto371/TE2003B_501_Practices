/* Parte 3 - Contador de eventos con interrupción y pausa visualizada en LCD */

#include <MKL25Z4.H>
#include <stdio.h>

#define RS 0x04
#define RW 0x10
#define EN 0x20

void delayMs(int n);
void delayUs(int n);
void keypad_init(void);
char keypad_getkey(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void send_nibble(unsigned char nibble);
char convert_key(unsigned char key);
void LCD_print(char *str);

volatile int paused = 0;
volatile int display_paused = 0;
int count = 0;

int main(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // clk TPM0

    //timer
    TPM0->SC = 0;
    TPM0->MOD = 0xFFFF;
    TPM0->SC |= TPM_SC_PS(1); // Prescaler /4
    TPM0->SC |= TPM_SC_TOF_MASK; // Clear flag
    TPM0->SC |= TPM_SC_CMOD(1);  // LPTPM counter increments on every TPM counter clock

    //interrupt
    PORTA->PCR[1] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
    PTA->PDDR &= ~(1 << 1);
    NVIC->ISER[0] |= (1 << 30);
    __enable_irq();

    keypad_init();
    LCD_init();
    LCD_command(1);

    while (1) {
        if (!paused) {
        	char buffer[20];
            LCD_command(1);
            LCD_command(0x80);
            snprintf(buffer, sizeof(buffer), "%d", count++);
            LCD_print(buffer);
            delayMs(1000000);
        } else {
            if (!display_paused) {
                LCD_command(1);
                LCD_command(0x80);
                LCD_print("PAUSED");
                display_paused = 1;
            }
            char key = keypad_getkey();
            if (key && convert_key(key) == '*') {
                paused = 0;
                display_paused = 0;
                LCD_command(1);
            }
        }
    }
}

void PORTA_IRQHandler(void) {
    if (PORTA->ISFR & (1 << 1)) {
        paused = 1;
        display_paused = 0;
        PORTA->ISFR = (1 << 1);
    }
}

void LCD_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTA_MASK;

    for (int i = 4; i <= 7; i++) PORTD->PCR[i] = 0x100;
    PTD->PDDR |= 0xF0;

    PORTA->PCR[2] = 0x100;
    PORTA->PCR[4] = 0x100;
    PORTA->PCR[5] = 0x100;
    PTA->PDDR |= RS | RW | EN;

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

void LCD_command(unsigned char command) {
    PTA->PCOR = RS | RW;
    send_nibble(command >> 4);
    send_nibble(command & 0x0F);
    if (command < 4) delayMs(4); else delayMs(1);
}

void LCD_data(unsigned char data) {
    PTA->PSOR = RS;
    PTA->PCOR = RW;
    send_nibble(data >> 4);
    send_nibble(data & 0x0F);
    delayMs(1);
}

void send_nibble(unsigned char nibble) {
    PTD->PDOR &= ~0xF0;
    PTD->PDOR |= (nibble & 0x0F) << 4;
    PTA->PSOR = EN;
    delayUs(1);
    PTA->PCOR = EN;
}

void LCD_print(char *str) {
    while (*str) LCD_data(*str++);
}

void delayMs(int n) {
    for (int i = 0; i < n; i++) while ((TPM0->SC & TPM_SC_TOF_MASK) == 0) {}
    TPM0->SC |= TPM_SC_TOF_MASK;
}

void delayUs(int n) {
    while ((TPM0->SC & TPM_SC_TOF_MASK) == 0) {}
    TPM0->SC |= TPM_SC_TOF_MASK;
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
