/*
 * File: hal_gpio.h
 * Description: HAL layer for generic GPIO operations
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H
#include "mcal_gpio.h"
#include <stdint.h>

/**
 * Initialize a GPIO pin for digital output
 */
void HAL_GPIO_InitOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

/**
 * Initialize a GPIO pin for digital input
 */
void HAL_GPIO_InitInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

/**
 * Set the GPIO pin state
 */
void HAL_GPIO_Write(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t state);

/**
 * Read the GPIO pin state
 */
uint8_t HAL_GPIO_Read(const MCAL_GPIO_Port_t *port_info, uint32_t pin);


#endif // HAL_GPIO_H
