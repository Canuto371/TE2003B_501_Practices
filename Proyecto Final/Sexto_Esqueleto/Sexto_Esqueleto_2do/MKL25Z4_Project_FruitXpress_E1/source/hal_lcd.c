#include "hal_lcd.h"
#include "hal_gpio.h"
#include "hal_delay.h"

static const HAL_LCD_Config_t *lcd_config = &LCD_Config;

// Función interna para enviar nibble
static void send_nibble(uint8_t nibble) {
    for(int i = 0; i < 4; i++) {
        MCAL_GPIO_WritePin(lcd_config->data_port, lcd_config->data_pins[i], (nibble >> i) & 0x01);
    }

    // Pulso en EN
    MCAL_GPIO_WritePin(lcd_config->ctrl_port, lcd_config->en_pin, 1);
    HAL_DelayUs(1);
    MCAL_GPIO_WritePin(lcd_config->ctrl_port, lcd_config->en_pin, 0);
    HAL_DelayUs(100);
}

void HAL_LCD_Init(const HAL_LCD_Config_t *config) {
    lcd_config = config;

    // Inicializar pines
    for(int i = 0; i < 4; i++) {
        MCAL_GPIO_ConfigOutput(lcd_config->data_port, lcd_config->data_pins[i]);
    }

    MCAL_GPIO_ConfigOutput(lcd_config->ctrl_port, lcd_config->rs_pin);
    MCAL_GPIO_ConfigOutput(lcd_config->ctrl_port, lcd_config->en_pin);

    // Secuencia de inicialización
    HAL_DelayMs(50);

    // Modo 4 bits
    MCAL_GPIO_WritePin(lcd_config->ctrl_port, lcd_config->rs_pin, 0);
    send_nibble(0x03);
    HAL_DelayMs(5);
    send_nibble(0x03);
    HAL_DelayUs(100);
    send_nibble(0x03);
    send_nibble(0x02);  // Pasar a modo 4 bits

    // Configuración básica
    HAL_LCD_Command(0x28);
    HAL_LCD_Command(0x0C);
    HAL_LCD_Command(0x06);
    HAL_LCD_Clear();
}

void HAL_LCD_Clear(void) {
    HAL_LCD_Command(0x01);
    HAL_DelayMs(2);
}

void HAL_LCD_Home(void) {
    HAL_LCD_Command(0x02);
    HAL_DelayMs(2);
}

void HAL_LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? 0x80 : 0xC0;
    address += col;
    HAL_LCD_Command(address);
}

void HAL_LCD_Print(const char *str) {
    while(*str) {
        HAL_LCD_PrintChar(*str++);
    }
}

void HAL_LCD_PrintChar(char c) {
    MCAL_GPIO_WritePin(lcd_config->ctrl_port, lcd_config->rs_pin, 1);
    send_nibble(c >> 4);
    send_nibble(c & 0x0F);
    HAL_DelayUs(50);
}

void HAL_LCD_Command(uint8_t cmd) {
    MCAL_GPIO_WritePin(lcd_config->ctrl_port, lcd_config->rs_pin, 0);
    send_nibble(cmd >> 4);
    send_nibble(cmd & 0x0F);

    if(cmd == 0x01 || cmd == 0x02) {
        HAL_DelayMs(2);
    }
}
