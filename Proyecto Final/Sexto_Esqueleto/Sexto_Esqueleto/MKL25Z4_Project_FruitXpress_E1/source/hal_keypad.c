#include "hal_keypad.h"
#include "hal_gpio.h"
#include "hal_delay.h"

static const HAL_Keypad_Config_t *keypad_config = &Keypad_Config;
static const char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void HAL_Keypad_Init(const HAL_Keypad_Config_t *config) {
    keypad_config = config;

    // Configurar pines de fila como salidas
    for(uint8_t i = 0; i < keypad_config->num_rows; i++) {
        MCAL_GPIO_ConfigOutput(keypad_config->port, keypad_config->row_pins[i]);
        MCAL_GPIO_WritePin(keypad_config->port, keypad_config->row_pins[i], 1); // Inicialmente en alto
    }

    // Configurar pines de columna como entradas con pull-up
    for(uint8_t i = 0; i < keypad_config->num_cols; i++) {
        MCAL_GPIO_ConfigInput(keypad_config->port, keypad_config->col_pins[i]);
    }
}

char HAL_Keypad_GetKey(void) {
    for(uint8_t row = 0; row < keypad_config->num_rows; row++) {
        // Activar solo la fila actual
        for(uint8_t r = 0; r < keypad_config->num_rows; r++) {
            MCAL_GPIO_WritePin(keypad_config->port, keypad_config->row_pins[r], (r == row) ? 0 : 1);
        }

        HAL_DelayUs(10); // Pequeño delay para estabilización

        // Leer columnas
        for(uint8_t col = 0; col < keypad_config->num_cols; col++) {
            if(!MCAL_GPIO_ReadPin(keypad_config->port, keypad_config->col_pins[col])) {
                // Anti-rebote
                HAL_DelayMs(20);
                while(!MCAL_GPIO_ReadPin(keypad_config->port, keypad_config->col_pins[col]));
                HAL_DelayMs(20);

                return keymap[row][col];
            }
        }
    }

    return 0;
}

char HAL_Keypad_WaitForKey(void) {
    char key = 0;
    while((key = HAL_Keypad_GetKey()) == 0);
    return key;
}
