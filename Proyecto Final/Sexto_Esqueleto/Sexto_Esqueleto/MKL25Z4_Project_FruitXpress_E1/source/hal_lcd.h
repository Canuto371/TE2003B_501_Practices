#ifndef HAL_LCD_H
#define HAL_LCD_H

#include <stdint.h>
#include "hal_gpio.h"

// Definición de la estructura de configuración del LCD
typedef struct {
    const MCAL_GPIO_Port_t *data_port;  // Puerto para datos D4-D7
    const MCAL_GPIO_Port_t *ctrl_port;  // Puerto para señales de control
    uint8_t data_pins[4];               // Pines para datos (D4-D7)
    uint8_t rs_pin;                     // Pin RS
    uint8_t rw_pin;                     // Pin RW
    uint8_t en_pin;                     // Pin EN
} HAL_LCD_Config_t;

// Prototipos de funciones
void HAL_LCD_Init(const HAL_LCD_Config_t *config);
void HAL_LCD_Clear(void);
void HAL_LCD_Home(void);
void HAL_LCD_SetCursor(uint8_t row, uint8_t col);
void HAL_LCD_Print(const char *str);
void HAL_LCD_PrintChar(char c);
void HAL_LCD_Command(uint8_t cmd);

// Configuración predefinida (deberás definir esto en hal_lcd_config.h)
extern const HAL_LCD_Config_t LCD_Config;

#endif // HAL_LCD_H
