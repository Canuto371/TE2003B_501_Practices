#ifndef HAL_LCD_CONFIG_H
#define HAL_LCD_CONFIG_H

#include "hal_lcd.h"
#include "hal_gpio.h"

extern const HAL_LCD_Config_t LCD_Config = {
    .data_port = &GPIOD_Info,    // Puerto D para datos D4-D7
    .ctrl_port = &GPIOA_Info,    // Puerto A para control
    .data_pins = {4, 5, 6, 7},   // PTD4-D7
    .rs_pin = 2,                 // PTA2
    .rw_pin = 4,                 // PTA4
    .en_pin = 5                  // PTA5
};

#endif // HAL_LCD_CONFIG_H
