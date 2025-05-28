#include "MKL25Z4.h"
#include <stdint.h>
#include <stdbool.h>

//pins y const hcsr04 (sens dist)
#define TRIG_PIN 1
#define ECHO_PIN 12
#define UMBRAL_CM 10

//LCD 4 bit
#define RS 0x04
#define RW 0x10
#define EN 0x20

//hcsr04
void SysTick_Init(void);
uint32_t micros(void);
void PORT_Configure(void);
uint32_t medir_distancia_cm(void);
void TPM0_Init(void);
void delayUs(int n);
void delayMs(int n);

//LCD y teclado
void keypad_init(void);
char keypad_getkey(void);
char convert_key(unsigned char key);
void LCD_init(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void send_nibble(unsigned char nibble);

int main(void) {
    uint32_t dist;
    unsigned char key;
    char keyChar;

    //init
    SystemCoreClockUpdate();
    SysTick_Init();
    PORT_Configure();
    TPM0_Init();
    keypad_init();
    LCD_init();

    while (1) {
        key = keypad_getkey();
        if (key) {
            keyChar = convert_key(key);
            if (keyChar == '*') {
                LCD_command(0x01); //clear
                delayMs(10);
                LCD_command(0x80); //cursor
                const char *msg = "RESET";
                for (int i = 0; msg[i] != '\0'; i++) {
                    LCD_data((unsigned char)msg[i]);
                }
                while (keypad_getkey() == key) {
                    delayMs(50);
                }
                continue;
            }
        }

        dist = medir_distancia_cm();

        char buffer[6] = { '0','0',' ','c','m','\0' };
        if (dist != 0xFFFFFFFFU) {
            if (dist >= 100) {
                buffer[0] = '0' + (dist / 100) % 10;
                buffer[1] = '0' + (dist / 10) % 10;
                buffer[2] = '0' + (dist % 10);
                buffer[3] = 'c';
                buffer[4] = 'm';
                buffer[5] = '\0';
            } else if (dist >= 10) {
                buffer[0] = '0' + (dist / 10);
                buffer[1] = '0' + (dist % 10);
                buffer[2] = ' ';
                buffer[3] = 'c';
                buffer[4] = 'm';
                buffer[5] = '\0';
            } else {
                buffer[0] = '0';
                buffer[1] = '0' + (dist % 10);
                buffer[2] = ' ';
                buffer[3] = 'c';
                buffer[4] = 'm';
                buffer[5] = '\0';
            }
        } else {
            buffer[0] = '-';
            buffer[1] = '-';
            buffer[2] = ' ';
            buffer[3] = 'c';
            buffer[4] = 'm';
            buffer[5] = '\0';
        }

        LCD_command(0x01);
        delayMs(10);
        LCD_command(0x80);
        for (int i = 0; buffer[i] != '\0'; i++) {
            LCD_data((unsigned char)buffer[i]);
        }

        delayMs(300);
    }
    return 0;
}

void SysTick_Init(void) {
    SysTick->LOAD = (SystemCoreClock / 1000000U) - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk //48MHz
                  | SysTick_CTRL_ENABLE_Msk;
}

//t en us
uint32_t micros(void) {
    static volatile uint32_t usTicks = 0;
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        usTicks++;
    }
    return usTicks;
}

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

}

uint32_t medir_distancia_cm(void) {
    uint32_t startTime = 0, endTime = 0, pulseWidth = 0;
    uint32_t distance_cm = 0;

    //PORTD->PCR[TRIG_PIN] = PORT_PCR_MUX(1);
    //GPIOD->PDDR |= (1U << TRIG_PIN); //salida
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
            /*PORTD->PCR[TRIG_PIN] = PORT_PCR_MUX(1)
                                 | PORT_PCR_PE_MASK
                                 | PORT_PCR_PS(0);
            GPIOD->PDDR &= ~(1U << TRIG_PIN);*/
            return 0xFFFFFFFFU;
        }
    }
    startTime = micros();

    //esperar negedge
    timeout = micros();
    while (GPIOA->PDIR & (1U << ECHO_PIN)) {
        if ((micros() - timeout) > 30000U) {
            /*PORTD->PCR[TRIG_PIN] = PORT_PCR_MUX(1)
                                 | PORT_PCR_PE_MASK
                                 | PORT_PCR_PS(0);
            GPIOD->PDDR &= ~(1U << TRIG_PIN);*/
            return 0xFFFFFFFFU;
        }
    }
    endTime = micros();

    pulseWidth = endTime - startTime; //ancho de pulso en us
    //distance_cm = pulseWidth / 58U; //dist en cm (t/58) -> data sheet hcsr04
    distance_cm = (uint32_t)(2.2f * (pulseWidth / 58.0f) + 2.5f);

    /*PORTD->PCR[TRIG_PIN] = PORT_PCR_MUX(1)
                         | PORT_PCR_PE_MASK
                         | PORT_PCR_PS(0);
    GPIOD->PDDR &= ~(1U << TRIG_PIN);*/

    return distance_cm;
}

void TPM0_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); //48 MHz
    TPM0->SC = 0;
    TPM0->CNT = 0;
    TPM0->SC = TPM_SC_PS(0) | TPM_SC_CMOD(1);
}

//medir counter de TPM0 (48 MHz -> 48 ticks = 1 us)
void delayUs(int n) {
    uint16_t start = (uint16_t)TPM0->CNT;
    uint16_t target = start + (uint16_t)(n * 48);
    while ((uint16_t)(TPM0->CNT - start) < (uint16_t)(n * 48)) {}
}

//1 ms = 1000 us
void delayMs(int n) {
    for (int i = 0; i < n; i++) {
        delayUs(1000);
    }
}

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

char convert_key(unsigned char key) {
    switch (key) {
        case 1:  return '1';
        case 2:  return '2';
        case 3:  return '3';
        case 4:  return 'A';
        case 5:  return '4';
        case 6:  return '5';
        case 7:  return '6';
        case 8:  return 'B';
        case 9:  return '7';
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

void LCD_command(unsigned char command) {
    PTA->PCOR = RS | RW;
    send_nibble(command >> 4);
    send_nibble(command & 0x0F);
    if (command < 4)
        delayMs(4);
    else
        delayMs(1);
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
    PTD->PDOR |= ((nibble & 0x0F) << 4);
    PTA->PSOR = EN;
    delayUs(1);
    PTA->PCOR = EN;
}
