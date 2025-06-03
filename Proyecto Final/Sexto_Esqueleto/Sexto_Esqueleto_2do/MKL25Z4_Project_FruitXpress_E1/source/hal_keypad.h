#ifndef HAL_KEYPAD_H
#define HAL_KEYPAD_H

#include <stdint.h>
#include "hal_gpio.h"

// Estructura de configuración del keypad
typedef struct {
    const MCAL_GPIO_Port_t *port;       // Puerto GPIO
    uint8_t row_pins[4];                // Pines para filas
    uint8_t col_pins[4];                // Pines para columnas
    uint8_t num_rows;                   // Número de filas
    uint8_t num_cols;                   // Número de columnas
} HAL_Keypad_Config_t;

// Prototipos de funciones
void HAL_Keypad_Init(const HAL_Keypad_Config_t *config);
char HAL_Keypad_GetKey(void);
char HAL_Keypad_WaitForKey(void);

// Configuración predefinida
extern const HAL_Keypad_Config_t Keypad_Config;

#endif // HAL_KEYPAD_H
