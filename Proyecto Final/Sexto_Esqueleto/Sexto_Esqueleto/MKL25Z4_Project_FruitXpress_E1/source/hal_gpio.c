/*
 * File: hal_gpio.c
 * Description: HAL implementation
 */
#include "hal_gpio.h"
#include "mcal_gpio.h"

/**
 * Inicializa un pin como salida digital
 */
void HAL_GPIO_InitOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigOutput(port_info, pin);
}

/**
 * Inicializa un pin como entrada digital (pull-up)
 */
void HAL_GPIO_InitInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigInput(port_info, pin);
}

/**
 * Escribe en el pin: state=1 -> pone el pin en alto; state=0 -> lo deja en bajo
 */
void HAL_GPIO_Write(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t state) {
    MCAL_GPIO_WritePin(port_info, pin, state);
}

/**
 * Lee el estado del pin: devuelve 1 si está en alto, 0 si está en bajo
 */
uint8_t HAL_GPIO_Read(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    return MCAL_GPIO_ReadPin(port_info, pin);
}

/* === Definición de las estructuras de cada puerto que vamos a usar === */
const MCAL_GPIO_Port_t GPIOA_Info = {
    .port     = PORTA,
    .gpio     = GPIOA,
    .clk_mask = MCAL_PORTA
};

const MCAL_GPIO_Port_t GPIOD_Info = {
    .port     = PORTD,
    .gpio     = GPIOD,
    .clk_mask = MCAL_PORTD
};
