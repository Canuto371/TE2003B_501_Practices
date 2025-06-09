
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <mcal.h>
#include <MKL25Z4.h>
#include "driver_types.h"
#include <stdint.h>

//gpio
void HAL_GPIO_InitOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);
void HAL_GPIO_InitInput (const MCAL_GPIO_Port_t *port_info, uint32_t pin);

void HAL_GPIO_InitAlt(const MCAL_GPIO_Port_t *port_info, uint32_t pin, GPIO_Direction_t direction, uint32_t mux);

void HAL_GPIO_Write(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t state);
uint8_t HAL_GPIO_Read(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

//adc
void HAL_ADC_Init(const MCAL_GPIO_Port_t *port_info, uint32_t pin);
uint16_t HAL_ADC_Read(uint8_t channel);

//recorrer config_table
void Driver_Init(const Driver_Config_t *config_table, uint32_t num_entries);

#endif // HAL_GPIO_H
