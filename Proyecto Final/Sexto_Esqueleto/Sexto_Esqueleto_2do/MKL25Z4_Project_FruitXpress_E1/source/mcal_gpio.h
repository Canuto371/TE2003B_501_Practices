/*
 * File: mcal_gpio.h
 * Description: Generic MCAL layer for GPIO on KL25Z
 */
#ifndef MCAL_GPIO_H
#define MCAL_GPIO_H
#include <MKL25Z4.h>
#include <stdint.h>

/** Mask identifiers for SIM->SCGC5 clocks */
typedef enum {
    MCAL_PORTA = SIM_SCGC5_PORTA_MASK,
    MCAL_PORTB = SIM_SCGC5_PORTB_MASK,
    MCAL_PORTC = SIM_SCGC5_PORTC_MASK,
    MCAL_PORTD = SIM_SCGC5_PORTD_MASK,
    MCAL_PORTE = SIM_SCGC5_PORTE_MASK
} MCAL_PortClockMask_t;

/** Generic port structure */
typedef struct {
    PORT_Type *port;    // PCR registers
    GPIO_Type *gpio;    // PDDR/PSOR/PCOR/PDIR registers
    MCAL_PortClockMask_t clk_mask;
} MCAL_GPIO_Port_t;

/**
 * Enable clock for given port
 */
void MCAL_GPIO_EnableClock(const MCAL_GPIO_Port_t *port_info);

/**
 * Configure pin as digital output
 */
void MCAL_GPIO_ConfigOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

/**
 * Configure pin as digital input (with pull-up)
 */
void MCAL_GPIO_ConfigInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

/**
 * Write pin: value=1 sets pin, value=0 clears pin
 */
void MCAL_GPIO_WritePin(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t value);

/**
 * Read pin: returns 1 if high, 0 if low
 */
uint8_t MCAL_GPIO_ReadPin(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

#endif // MCAL_GPIO_H
