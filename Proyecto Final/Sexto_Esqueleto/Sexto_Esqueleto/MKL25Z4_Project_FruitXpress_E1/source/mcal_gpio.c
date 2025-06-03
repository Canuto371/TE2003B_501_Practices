/*
 * File: mcal_gpio.c
 * Description: MCAL implementation
 */
#include "mcal_gpio.h"

/**
 * Enable clock for given port
 */
void MCAL_GPIO_EnableClock(const MCAL_GPIO_Port_t *port_info) {
    SIM->SCGC5 |= port_info->clk_mask;
}

/**
 * Configure pin as digital output
 */
void MCAL_GPIO_ConfigOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    // PCR como GPIO
    port_info->port->PCR[pin] = PORT_PCR_MUX(1);
    // PDDR: bit pin -> 1 (salida)
    port_info->gpio->PDDR |= (1UL << pin);
}

/**
 * Configure pin as digital input (con pull-up)
 */
void MCAL_GPIO_ConfigInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    // PCR: GPIO + pullenable + pull-up
    port_info->port->PCR[pin] = PORT_PCR_MUX(1)     /* GPIO  */
                             | PORT_PCR_PE_MASK     /* Enable pull */
                             | PORT_PCR_PS(1);      /* Pull-up  */
    // PDDR: bit pin -> 0 (entrada)
    port_info->gpio->PDDR &= ~(1UL << pin);
}

/**
 * Write pin: value=1 sets pin, value=0 clears pin
 */
void MCAL_GPIO_WritePin(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t value) {
    if (value) {
        port_info->gpio->PSOR = (1UL << pin); // Set pin
    } else {
        port_info->gpio->PCOR = (1UL << pin); // Clear pin
    }
}

/**
 * Read pin: returns 1 si el pin está en alto, 0 si está en bajo
 */
uint8_t MCAL_GPIO_ReadPin(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    // Leemos PDIR y desplazamos para obtener solo el bit 'pin'
    return (uint8_t)((port_info->gpio->PDIR >> pin) & 0x1U);
}


