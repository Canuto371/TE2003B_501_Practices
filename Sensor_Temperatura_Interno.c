#include <MKL25Z4.H>

#define RS 0x04
#define RW 0x10
#define EN 0x20

// Parámetros del sensor de temperatura
#define V_TEMP25  0.716f    // Voltaje a 25°C (716 mV)
#define M_SLOPE   0.00162f  // Pendiente (1.62 mV/°C)

void delayMs(int n);
void LCD_nibble(unsigned char nibble, unsigned char control);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void LCD_string(char *str);
float read_temp(void);
void display_temp(float temp);

int main(void) {

    SIM->SCGC6 |= 0x01000000; /* habilitar reloj a TPM0 */
    SIM->SOPT2 |= 0x01000000; /* usar 32.768 kHz como reloj */
    TPM0->SC = 0x0; /* deshabilitar mientras se configura */
    TPM0->SC = 0x02; /* prescaler /4 */
    TPM0->MOD = 0x2000; /* valor máximo*/
    TPM0->SC |= 0x80; /* limpiar TOF */
    TPM0->SC |= 0x08; /* habilitar modo libre */

    /* Habilitar reloj para ADC */
    SIM->SCGC6 |= (1 << 27);

    LCD_init();
    LCD_command(1); /* limpiar pantalla */

    LCD_command(0x80); /* primera línea */
    LCD_string("Temp: ");

    for (;;) {
        float temperature = read_temp();
        display_temp(temperature);
        delayMs(1000); /* actualizar cada segundo */
    }
}

float read_temp(void) {
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK; // Habilitar ADC

    ADC0->CFG1 = ADC_CFG1_MODE(2) | ADC_CFG1_ADICLK(1) | ADC_CFG1_ADIV(2);
    ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK; // Conversión por software

    ADC0->SC1[0] = ADC_SC1_ADCH(26); // Canal 26: temperatura interna

    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK)); // Esperar fin de conversión

    uint16_t adc_value = ADC0->R[0];

    float voltage = (float)adc_value * 3.3f / 1023.0f;
    float tempC = 25.0f - ((voltage - V_TEMP25) / M_SLOPE);

    return tempC;
}

void display_temp(float temp) {
    char buffer[16];
    int temp_int = (int)temp;
    int temp_frac = (int)((temp - temp_int) * 10);

    if(temp_frac < 0) temp_frac = -temp_frac;

    // Posicionar cursor
    LCD_command(0x80 | 0x06);

    // Mostrar temperatura
    if(temp >= 0) {
        sprintf(buffer, "%2d.%1d C  ", temp_int, temp_frac);
    }
    LCD_string(buffer);
}


void LCD_init(void) {
    SIM->SCGC5 |= 0x1000;
    PORTD->PCR[0] = 0x100;
    PORTD->PCR[1] = 0x100;
    PORTD->PCR[2] = 0x100;
    PORTD->PCR[3] = 0x100;
    PORTD->PCR[4] = 0x100;
    PORTD->PCR[5] = 0x100;
    PORTD->PCR[6] = 0x100;
    PORTD->PCR[7] = 0x100;
    PTD->PDDR = 0xFF;
    SIM->SCGC5 |= 0x0200;
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
    LCD_command(0x0C);
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
    if (command < 4)
        delayMs(4);
    else
        delayMs(1);
}

void LCD_data(unsigned char data) {
    LCD_nibble(data & 0xF0, RS);
    LCD_nibble((data << 4) & 0xF0, RS);
    delayMs(1);
}

void LCD_string(char *str) {
    while (*str) {
        LCD_data(*str++);
    }
}

void delayMs(int n) {
    int i;
    for (i = 0; i < n; i++) {
        while((TPM0->SC & 0x80) == 0) { }
        TPM0->SC |= 0x80;
    }
}