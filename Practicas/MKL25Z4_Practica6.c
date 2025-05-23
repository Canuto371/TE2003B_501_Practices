/*
 * Práctica 6: Parte 3
 * Sistema de motores PWM
 * Periféricos: LCD, teclado, pot en PTB0 y LED ext en PTA12
*/

#include <MKL25Z4.H>
#include <stdio.h>

// LCD
#define RS   (1 << 2)
#define RW   (1 << 4)
#define EN   (1 << 5)

// push buttons para interrupts
#define MENU_BTN_PIN   2
#define EMERG_BTN_PIN  3

// PWM
#define PWM_CHANNEL    0
#define PWM_MODULO     43702   // 60 Hz

// ADC
#define ADC_CHANNEL    8


void initPorts(void);
void initLCD(void);
void LCD_command(uint8_t cmd);
void LCD_data(uint8_t data);
void send_nibble(uint8_t nibble);
void LCD_print(char *str);
void delayMs(int n);
void delayUs(int n);

void keypad_init(void);
char keypad_getkey(void);
char convert_key(uint8_t key);

void initPWM(void);
void setPWMDuty(uint16_t cnv_value);

void initADC(void);
uint16_t readADC(void);

void PORTD_IRQHandler(void);


volatile uint8_t mode;
volatile uint8_t return_to_menu;
volatile uint8_t emergency_stop;


int main(void) {
    SystemCoreClockUpdate();
    initPorts();
    initLCD();
    keypad_init();
    initPWM();
    initADC();

    char key;
    uint16_t adc_val;
    float voltage;
    uint16_t cnv_val;

    while (1) {
        LCD_command(0x01);                // Clear display
        LCD_command(0x80);                // Cursor lin 1 col 0
        LCD_print("Set input mode");
        LCD_command(0xC0);                // lin 2 col 0
        LCD_print("A:Auto  B:Manual");

        return_to_menu = 0;
        emergency_stop = 0;

        mode = 0;
        while (mode == 0) {
            key = keypad_getkey();
            if (key == 'B') {
                mode = 'M';
            } else if (key == 'A') {
                mode = 'A';
            }
        }

        while (keypad_getkey() != '#') {}

        // manual
        if (mode == 'M') {
            LCD_command(0x01);
            LCD_command(0x80);
            LCD_print("Select Speed");
            LCD_command(0xC0);
            LCD_print("1:L 2:M 3:MH 4:H");

            while (!return_to_menu && !emergency_stop) {
                key = 0;
                int keyBef = 0;
                while ((key < '1' || key > '4') && !return_to_menu && !emergency_stop) {
                    key = keypad_getkey();
                    if (keyBef != key) {
                        keyBef = key;
                        switch (key) {
                           case '1': cnv_val = (PWM_MODULO + 1) * 25 / 100; break;
                           case '2': cnv_val = (PWM_MODULO + 1) * 50 / 100; break;
                           case '3': cnv_val = (PWM_MODULO + 1) * 75 / 100; break;
                           case '4': cnv_val = PWM_MODULO + 1;                break; // ~100%
                           default:  cnv_val = 0; break;
                       }
                       setPWMDuty(cnv_val);
                   }
                }
            }
        }

        // auto
        else if (mode == 'A') {
			while (!return_to_menu && !emergency_stop) {
				LCD_command(0x01);
				LCD_command(0x80);
				LCD_print("Auto mode ON");
				adc_val = readADC();
				voltage = (adc_val * 3.3f) / 255.0f;

				if (voltage <= 0.75f) {
					cnv_val = (PWM_MODULO + 1) * 25 / 100;
				} else if (voltage <= 1.5f) {
					cnv_val = (PWM_MODULO + 1) * 50 / 100;
				} else if (voltage <= 2.25f) {
					cnv_val = (PWM_MODULO + 1) * 75 / 100;
				} else {
					cnv_val = PWM_MODULO + 1;
				}
				setPWMDuty(cnv_val);

				delayMs(100);
			}
		}

        // interrupt
        if (emergency_stop) {
            TPM1->SC &= ~TPM_SC_CMOD_MASK;
            initPWM();
            LCD_command(0x01);
            LCD_command(0x80);
            LCD_print("EMERGENCY");

            while (!return_to_menu) {}
            emergency_stop = 0;
        }

        TPM1->SC &= ~TPM_SC_CMOD_MASK;
        initPWM();
    }
}


void initPorts(void) {
    // clks
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK
                | SIM_SCGC5_PORTB_MASK
                | SIM_SCGC5_PORTC_MASK
                | SIM_SCGC5_PORTD_MASK
                | SIM_SCGC5_PORTE_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK
                | SIM_SCGC6_ADC0_MASK;

    // PWM
    PORTA->PCR[12] = PORT_PCR_MUX(3);

    // LCD
    PORTA->PCR[2] = PORT_PCR_MUX(1);
    PTA->PDDR |= RS;
    PORTA->PCR[4] = PORT_PCR_MUX(1);
    PTA->PDDR |= RW;
    PTA->PCOR |= RW;
    PORTA->PCR[5] = PORT_PCR_MUX(1);
    PTA->PDDR |= EN;
    for (int i = 4; i <= 7; i++) {
        PORTD->PCR[i] = PORT_PCR_MUX(1);
    }
    PTD->PDDR |= (0xF << 4);

    // pot (ADC)
    PORTB->PCR[0] = PORT_PCR_MUX(0);

    // push buttons
    PORTD->PCR[MENU_BTN_PIN] = PORT_PCR_MUX(1)
                             | PORT_PCR_PE_MASK
                             | PORT_PCR_PS_MASK
                             | PORT_PCR_IRQC(0xA);
    PORTD->PCR[EMERG_BTN_PIN] = PORT_PCR_MUX(1)
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_IRQC(0xA);
    PTD->PDDR &= ~((1 << MENU_BTN_PIN) | (1 << EMERG_BTN_PIN));

    PORTD->ISFR = (1 << MENU_BTN_PIN) | (1 << EMERG_BTN_PIN);
    NVIC_EnableIRQ(PORTD_IRQn);
    __enable_irq();
}


/*
 * LCD
*/

void initLCD(void) {
    delayMs(20);

    PTA->PCOR = RS | RW;

    send_nibble(0x03);
    delayMs(5);
    send_nibble(0x03);
    delayUs(200);
    send_nibble(0x03);
    delayUs(200);
    send_nibble(0x02);
    delayUs(200);

    LCD_command(0x28);
    LCD_command(0x0C);
    LCD_command(0x01);
    delayMs(2);
    LCD_command(0x06);
}

void LCD_command(uint8_t cmd) {
    PTA->PCOR = RS;
    PTA->PCOR = RW;
    send_nibble(cmd >> 4);
    send_nibble(cmd & 0x0F);
    if (cmd == 0x01 || cmd == 0x02) {
        delayMs(2);
    } else {
        delayUs(100);
    }
}

void LCD_data(uint8_t data) {
    PTA->PSOR = RS;
    PTA->PCOR = RW;
    send_nibble(data >> 4);
    send_nibble(data & 0x0F);
    delayUs(100);
}

void send_nibble(uint8_t nibble) {
    PTD->PDOR &= ~(0xF << 4);
    PTD->PDOR |= ((nibble & 0x0F) << 4);
    PTA->PSOR = EN;
    delayUs(2);
    PTA->PCOR = EN;
}

void LCD_print(char *str) {
    while (*str) {
        LCD_data(*str++);
    }
}

/*
 * Delays
*/

void delayMs(int n) {
    for (int i = 0; i < n; i++) {
        for (volatile int j = 0; j < 7000; j++) {
            __NOP();
        }
    }
}

void delayUs(int n) {
    for (int i = 0; i < n; i++) {
        __NOP(); __NOP(); __NOP(); __NOP();
    }
}


/*
 * Teclado
 */

void keypad_init(void) {
    for (int i = 0; i < 4; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1);
    }
    for (int i = 4; i < 8; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1)
                      | PORT_PCR_PE_MASK
                      | PORT_PCR_PS_MASK;
    }
    PTC->PDDR |= 0x0F;   // PC0..PC3 salida
    PTC->PSOR |= 0x0F;   // filas en ‘1’ por defecto
}

char keypad_getkey(void) {
    uint8_t row, col;
    const uint8_t row_mask[4] = {1, 2, 4, 8};

    for (row = 0; row < 4; row++) {
        PTC->PSOR = 0x0F;
        PTC->PCOR = row_mask[row];
        delayUs(5);
        col = (PTC->PDIR & 0xF0) >> 4;
        if (col != 0x0F) {
            for (uint8_t c = 0; c < 4; c++) {
                if (((col >> c) & 0x01) == 0) {
                    return convert_key(row * 4 + c + 1);
                }
            }
        }
    }
    return 0;
}

char convert_key(uint8_t key) {
    switch (key) {
        case 1:  return '1';  case 2:  return '2';
        case 3:  return '3';  case 4:  return 'A';
        case 5:  return '4';  case 6:  return '5';
        case 7:  return '6';  case 8:  return 'B';
        case 9:  return '7';  case 10: return '8';
        case 11: return '9';  case 12: return 'C';
        case 13: return '*';  case 14: return '0';
        case 15: return '#';  case 16: return 'D';
        default: return 0;
    }
}


/*
 * PWM
*/

void initPWM(void) {
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // TPM1 usa MCGFLLCLK (≈41.94 MHz)
    TPM1->SC = 0;
    TPM1->MOD = PWM_MODULO;

    TPM1->CONTROLS[PWM_CHANNEL].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM1->CONTROLS[PWM_CHANNEL].CnV  = 0;

    TPM1->SC = TPM_SC_PS(1) | TPM_SC_CMOD(1);  // prescaler /16, iniciar conteo
}

void setPWMDuty(uint16_t cnv_value) {
    TPM1->CONTROLS[PWM_CHANNEL].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK;
    TPM1->CONTROLS[PWM_CHANNEL].CnV = cnv_value;
}


// ADC

void initADC(void) {
    ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;
    ADC0->SC3 |= ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);
    ADC0->SC1[0] = ADC_SC1_ADCH(ADC_CHANNEL);
}

uint16_t readADC(void) {
    ADC0->SC1[0] = ADC_SC1_ADCH(ADC_CHANNEL);
    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK)) {}
    return ADC0->R[0];
}


/*
 * Interrupt handler
 */

void PORTD_IRQHandler(void) {
    uint32_t isfr = PORTD->ISFR;

    if (isfr & (1 << MENU_BTN_PIN)) {
        PORTD->ISFR = (1 << MENU_BTN_PIN);
        initPWM();
        return_to_menu = 1;
    }
    if (isfr & (1 << EMERG_BTN_PIN)) {
        PORTD->ISFR = (1 << EMERG_BTN_PIN);
        initPWM();
        emergency_stop = 1;
    }
}
