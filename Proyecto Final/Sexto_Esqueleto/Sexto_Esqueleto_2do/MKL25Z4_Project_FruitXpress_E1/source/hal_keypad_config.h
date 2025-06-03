#ifndef HAL_KEYPAD_CONFIG_H
#define HAL_KEYPAD_CONFIG_H

#include "hal_keypad.h"
#include "hal_gpio.h"

extern const HAL_Keypad_Config_t Keypad_Config = {
    .port = &GPIOC_Info,            // Puerto C
    .row_pins = {0, 1, 2, 3},       // PTC0-PTC3 (filas)
    .col_pins = {4, 5, 6, 7},       // PTC4-PTC7 (columnas)
    .num_rows = 4,
    .num_cols = 4
};

#endif // HAL_KEYPAD_CONFIG_H
