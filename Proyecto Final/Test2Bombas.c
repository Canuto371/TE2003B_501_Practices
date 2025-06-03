// Proyecto Final FruitXpress: menú + 2 sensores ultrasónicos + 2 bombas
// Se necesitan los archivos "hal_gpio.h", "hal_gpio.c", "mcal_gpio.h" y, "mcal_gpio.c".

#include <MKL25Z4.h>
#include "hal_gpio.h"
#include "mcal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

// LCD 4-bit
#define RS  0x04   // PTA2
#define RW  0x10   // PTA4
#define EN  0x20   // PTA5

// HC-SR04
#define TRIG_PIN_1	1	// PTD1
#define ECHO_PIN_1	12	// PTA12

#define TRIG_PIN_2	11	// PTC11
#define ECHO_PIN_2	10	// PTC10

#define UMBRAL_CM  14   // umbral de disponibilidad (14 cm)

// Pump
#define BOMB_PIN_1	1U	// PTA1
#define BOMB_PIN_2	12U	// PTC12

/* --- DELAYS --- */
void delayMs(int n);
void delayUs(int n);

void SysTick_Init(void);
uint32_t micros(void);

void TPM0_Init(void);

/* --- KEYPAD --- */
void keypad_init(void);
char keypad_getkey(void);
char convert_key(unsigned char key);

/* --- LCD --- */
void LCD_init(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_print(char *str);
void send_nibble(unsigned char nibble);

/* --- HC-SR04 --- */
uint32_t medir_distancia_cm(const MCAL_GPIO_Port_t *PortTrig_info, const MCAL_GPIO_Port_t *PortEcho_info, uint32_t pinTrig, uint32_t pinEcho);

/* --- MENÚS --- */
void show_main_menu(void);
void show_selection_menu(void);

/* --- MANEJO NIVEL DE TANQUE Y BOMBA --- */
uint8_t check_level(int n);

/* ==================== MAIN ==================== */
int main(void) {
    __enable_irq();
  
    SystemCoreClockUpdate();
    SysTick_Init();
    TPM0_Init();
    keypad_init();
    LCD_init();

    HAL_GPIO_InitOutput(&GPIOD_Info, TRIG_PIN_1);
    HAL_GPIO_InitInput(&GPIOA_Info, ECHO_PIN_1);
    HAL_GPIO_InitOutput(&GPIOC_Info, TRIG_PIN_2);
    HAL_GPIO_InitInput(&GPIOC_Info, ECHO_PIN_2);
  
    HAL_GPIO_InitOutput(&GPIOA_Info, BOMB_PIN_1);
    HAL_GPIO_Write(&GPIOA_Info, BOMB_PIN_1, 0);
    HAL_GPIO_InitOutput(&GPIOC_Info, BOMB_PIN_2);
    HAL_GPIO_Write(&GPIOC_Info, BOMB_PIN_2, 0);

    show_main_menu();

    while (1) { /* no debe llegar aquí */ }
    return 0;
}

/* ==================== MENÚ PRINCIPAL ==================== */
void show_main_menu(void) {
    while (1) {
        LCD_command(0x01); // clear
        delayMs(10);

        LCD_command(0x80); // primera línea
        LCD_print("!FruitXpress!");
        LCD_command(0xC0); // segunda línea
        LCD_print("# para continuar");

        // Esperar '#'
        unsigned char key = 0;
        while ((key = keypad_getkey()) == 0) {
            ; 
        }
        char keyChar = convert_key(key);
       
        while (keypad_getkey() == key) {
            delayMs(50);
        }

        if (keyChar == '#') {
            LCD_command(0x01);
            delayMs(10);
            LCD_command(0x80);
            LCD_print("Elige un sabor:");
            delayMs(1000);
            show_selection_menu();
        }
        else {
            continue;
        }
    }
}

/* ==================== MENÚ DE SELECCIÓN ==================== */
void show_selection_menu(void) {
	LCD_command(0x01);
	delayMs(10);

	LCD_command(0x80);
	LCD_print("1:Coco 2:Pina");
	LCD_command(0xC0);
	LCD_print("3:Fresa 4:Cancelar");
	delayMs(500);

    while (1) {
        unsigned char key = keypad_getkey();
        if (key) {
            char keyChar = convert_key(key);
           
            while (keypad_getkey() == key) {
                delayMs(50);
            }

            if (keyChar == '1') {
                LCD_command(0x01);
                LCD_print("Coco seleccion.");
                delayMs(1000);
                check_level(1);
                return; 
            }
            else if (keyChar == '2') {
                LCD_command(0x01);
                LCD_print("Pina seleccion");
                delayMs(1000);
                check_level(2);
                return;
            }
            else if (keyChar == '3') {
                // Por ahora, sin funcionalidad
                LCD_command(0x01);
                LCD_print("Fresa: Sin implementar");
                delayMs(1000);
                return;
            }
            else if (keyChar == '4') {
                // Cancelar
                return;
            }
            else {
                // Entrada inválida
                LCD_command(0x01);
                LCD_print("Entrada invalida");
                delayMs(1000);
                return;
            }
        }
    }
}

/* ==================== FUNCIÓN check_level ====================
 * lee distancia con HC-SR04; si < UMBRAL_CM, enciende
 * la bomba del jugo elegido 3 seg, muestra mensaje en LCD,
 * luego regresa al menú principal.
 *
 * FALTA n=3
 */
uint8_t check_level(int n) {

	uint32_t dist_cm=0;

    if (n == 1) {
        dist_cm = medir_distancia_cm(&GPIOD_Info, &GPIOA_Info, TRIG_PIN_1, ECHO_PIN_1);
    }
    else if (n == 2) {
    	dist_cm = medir_distancia_cm(&GPIOC_Info, &GPIOC_Info, TRIG_PIN_2, ECHO_PIN_2);
    }

	char buffer[7] = {'0','0',' ','c','m','\0','\0'};
	if (dist_cm != 0xFFFFFFFFU) {
		if (dist_cm >= 100) {
			buffer[0] = '0' + ((dist_cm / 100) % 10);
			buffer[1] = '0' + ((dist_cm / 10) % 10);
			buffer[2] = '0' + (dist_cm % 10);
			buffer[3] = 'c';
			buffer[4] = 'm';
			buffer[5] = '\0';
		}
		else if (dist_cm >= 10) {
			buffer[0] = '0' + (dist_cm / 10);
			buffer[1] = '0' + (dist_cm % 10);
			buffer[2] = ' ';
			buffer[3] = 'c';
			buffer[4] = 'm';
			buffer[5] = '\0';
		}
		else {
			buffer[0] = '0';
			buffer[1] = '0' + (dist_cm % 10);
			buffer[2] = ' ';
			buffer[3] = 'c';
			buffer[4] = 'm';
			buffer[5] = '\0';
		}
	}
	else {
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

	LCD_command(0xC0);
	if ((dist_cm != 0xFFFFFFFFU) && (dist_cm < UMBRAL_CM)) {
		LCD_print(" Disponible ");

    //ON
		if (n == 1) {
			HAL_GPIO_Write(&GPIOA_Info, BOMB_PIN_1, 1);
		} else if (n == 2) {
			HAL_GPIO_Write(&GPIOC_Info, BOMB_PIN_2, 1);
		}

		uint32_t t_start = micros();
	
		while ((micros() - t_start) < 3000000U) {
			;
		}

		//OFF
		if (n == 1) {
			HAL_GPIO_Write(&GPIOA_Info, BOMB_PIN_1, 0);
		} else if (n == 2) {
			HAL_GPIO_Write(&GPIOC_Info, BOMB_PIN_2, 0);
		}

		LCD_command(0x01);
		delayMs(20);
		LCD_command(0x80);
		LCD_print("Bomba apagada");
		delayMs(1000);
	}
	else {
		LCD_command(0x01);
		delayMs(20);
		LCD_command(0x80);
		LCD_print("Lo sentimos!");
		LCD_command(0xC0);
		LCD_print("No disponible");
		delayMs(2000);
	}
  
    return 0;
}

/* ==================== IMPLEMENTACIÓN HC-SR04 ==================== */
uint32_t medir_distancia_cm(const MCAL_GPIO_Port_t *PortTrig_info, const MCAL_GPIO_Port_t *PortEcho_info, uint32_t pinTrig, uint32_t pinEcho) {
    uint32_t startTime = 0, endTime = 0, pulseWidth = 0;
    uint32_t distance_cm = 0;

    HAL_GPIO_Write(PortTrig_info, pinTrig, 0);
    delayUs(2);

    HAL_GPIO_Write(PortTrig_info, pinTrig, 1);
    delayUs(10);
    HAL_GPIO_Write(PortTrig_info, pinTrig, 0);

    uint32_t timeout = micros();
    while (HAL_GPIO_Read(PortEcho_info, pinEcho) == 0) {
        if ((micros() - timeout) > 30000U) {
            return 0xFFFFFFFFU; 
        }
    }
    startTime = micros();

    timeout = micros();
    while (HAL_GPIO_Read(PortEcho_info, pinEcho) == 1) {
        if ((micros() - timeout) > 30000U) {
            return 0xFFFFFFFFU;
        }
    }
    endTime = micros();

    pulseWidth = endTime - startTime;
    distance_cm = (uint32_t)(4.0f * (pulseWidth / 58.0f) + 4.3f); //ajuste de distancia

    return distance_cm;
}

/* ==================== FUNCIONES LCD ==================== */
void LCD_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK;
    for (int i = 4; i <= 7; i++) {
        PORTD->PCR[i] = PORT_PCR_MUX(1);
    }
    PTD->PDDR |= 0xF0;

    PORTA->PCR[2] = PORT_PCR_MUX(1);
    PORTA->PCR[4] = PORT_PCR_MUX(1);
    PORTA->PCR[5] = PORT_PCR_MUX(1);
    PTA->PDDR |= (RS | RW | EN);

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
    if (command < 4) delayMs(4);
    else          delayMs(1);
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

void LCD_print(char *str) {
    while (*str) {
        LCD_data((unsigned char)(*str));
        str++;
    }
}

/* ==================== FUNCIONES KEYPAD ==================== */
void keypad_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    for (int i = 0; i < 4; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1);
    }
    for (int i = 4; i < 8; i++) {
        PORTC->PCR[i] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS(1);
    }
    PTC->PDDR |= 0x0F;
    PTC->PSOR |= 0x0F;
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

char convert_key(unsigned char key) {
    switch (key) {
        case  1: return '1';
        case  2: return '2';
        case  3: return '3';
        case  4: return 'A';
        case  5: return '4';
        case  6: return '5';
        case  7: return '6';
        case  8: return 'B';
        case  9: return '7';
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

/* ==================== TIEMPOS ==================== */
void SysTick_Init(void) {
    SysTick->LOAD = (SystemCoreClock / 1000000U) - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

uint32_t micros(void) {
    static volatile uint32_t usTicks = 0;
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        usTicks++;
    }
    return usTicks;
}

void TPM0_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
    TPM0->SC = 0;
    TPM0->CNT = 0;
    TPM0->SC = TPM_SC_PS(0) | TPM_SC_CMOD(1);
}

void delayUs(int n) {
    uint16_t start = (uint16_t)TPM0->CNT;
    uint16_t ticks = (uint16_t)(n * (SystemCoreClock / 1000000U));
    while ((uint16_t)(TPM0->CNT - start) < ticks) {
        ;
    }
}

void delayMs(int n) {
    for (int i = 0; i < n; i++) {
        delayUs(1000);
    }
}
