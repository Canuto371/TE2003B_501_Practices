/*
 * File: hal_gpio.c
 * Description: HAL implementation
 */
#include "hal_gpio.h"

void HAL_GPIO_InitOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigOutput(port_info, pin);
}

void HAL_GPIO_InitInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
	MCAL_GPIO_EnableClock(port_info);
	MCAL_GPIO_ConfigInput(port_info, pin);
}

void HAL_GPIO_Write(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t state) {
	MCAL_GPIO_WritePin(port_info, pin, state);
}

uint8_t HAL_GPIO_Read(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
	MCAL_GPIO_ReadPin(port_info, pin);
}
