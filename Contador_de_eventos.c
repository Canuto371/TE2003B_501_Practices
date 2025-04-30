#include <MKL25Z4.H>
#include <stdio.h>

#define RS 0x04
#define RW 0x10
#define EN 0x20
#define LED_RED 18

void delayMs(int n);
void LCD_nibble(unsigned char nibble, unsigned char control);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void LCD_print_num(uint32_t num);
void generate_test_pulse(void);
void LED_init(void);
void LED_toggle(void);
void LCD_clear_line(unsigned char line);

int main(void) {
    SystemCoreClockUpdate();

    LED_init(); // Inicializar LED

    /* Configuración de PTA13 como salida GPIO para señal de prueba */
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;       // Habilita reloj para PORTA
    PORTA->PCR[13] = PORT_PCR_MUX(1);         // PTA13 como GPIO
    PTA->PDDR |= (1 << 13);                   // PTA13 como salida

    /* Configuración del pin PTC12 */
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;       // Habilita reloj para PORTC
    PORTC->PCR[12] = PORT_PCR_MUX(3);         // PTC12 como TPM0_CH2 (ALT3)

    /* Configurar TPM0 */
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;        // Habilitar reloj a TPM0
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);        // Fuente de reloj para TPM

    TPM0->SC = 0;                  // Deshabilita TPM0 para configuración
    TPM0->CNT = 0;                 // Limpia contador
    TPM0->MOD = 0xFFFF;            // Conteo máximo

    // Configurar canal 2 para entrada de contador externo
    TPM0->CONTROLS[2].CnSC = 0;    // Deshabilitar captura/comparación

    // Configurar TPM en modo contador externo (TPM0_CH2 como entrada)
    TPM0->SC = TPM_SC_CMOD(2) |    // Modo contador externo (TPM0_CH2)
               TPM_SC_PS(0);       // Sin prescaler

    LCD_init();
    LCD_command(0x01);             // Limpiar pantalla
    delayMs(10);

    uint32_t last_count = 0;
    uint32_t pulse_counter = 0;

    while (1) {
        // Generar pulso de prueba
        LED_toggle();               // Encender LED
        generate_test_pulse();
        LED_toggle();               // Apagar LED
        pulse_counter++;

        uint32_t current_count = TPM0->CNT; // Leer valor del contador

        // Actualizar pantalla
        if (current_count != last_count || pulse_counter % 1 == 0)
        	{
            // Mostrar pulsos generados
            LCD_command(0x01);
            LCD_print_num(pulse_counter);

            last_count = current_count;
        }

        delayMs(1000);
    }
}

// Función para generar un pulso en PTA13
void generate_test_pulse(void) {
    PTA->PSOR = (1 << 13);         // Poner PTA13 en alto
    delayMs(10);
    PTA->PCOR = (1 << 13);         // Poner PTA13 en bajo
    delayMs(10);
}

// LED Rojo que muestra los pulsos generados
void LED_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    PORTB->PCR[LED_RED] = PORT_PCR_MUX(1);
    PTB->PDDR |= (1 << LED_RED);
    PTB->PSOR = (1 << LED_RED);
}

// Cambia estado del LED
void LED_toggle(void) {
    PTB->PTOR = (1 << LED_RED);
}

// imprimir número en LCD
void LCD_print_num(uint32_t num) {
	 char buffer[10];
	    sprintf(buffer, "%lu", num);
	    for (int i = 0; buffer[i] != '\0'; i++) {
	        LCD_data(buffer[i]);
	    }
}

void LCD_init(void) {
    SIM->SCGC5 |= 0x1000; // Port D
    for (int i = 0; i <= 7; i++)
        PORTD->PCR[i] = 0x100;
    PTD->PDDR = 0xFF;

    SIM->SCGC5 |= 0x0200; // Port A
    PORTA->PCR[2] = 0x100;
    PORTA->PCR[4] = 0x100;
    PORTA->PCR[5] = 0x100;
    PTA->PDDR |= 0x34;

    delayMs(30);
    LCD_nibble(0x30, 0);
    delayMs(10);
    LCD_nibble(0x30, 0);
    delayMs(1);
    LCD_nibble(0x30, 0);
    delayMs(1);
    LCD_nibble(0x20, 0);
    delayMs(1);
    LCD_command(0x28);
    LCD_command(0x06);
    LCD_command(0x01);
    LCD_command(0x0F);
}

void LCD_nibble(unsigned char nibble, unsigned char control) {
    PTD->PDOR = (PTD->PDOR & 0x0F) | (nibble & 0xF0);
    PTA->PCOR = RS | RW | EN;
    PTA->PSOR = control;
    PTA->PSOR = EN;
    delayMs(0);
    PTA->PCOR = EN;
}

void LCD_command(unsigned char command) {
    LCD_nibble(command & 0xF0, 0);
    LCD_nibble((command << 4) & 0xF0, 0);
    delayMs((command < 4) ? 4 : 1);
}

void LCD_data(unsigned char data) {
    LCD_nibble(data & 0xF0, RS);
    LCD_nibble((data << 4) & 0xF0, RS);
    delayMs(1);
}

void delayMs(int n) {
	 for (volatile int i = 0; i < n * 3500; i++) {}
}
