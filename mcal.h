
#ifndef MCAL_GPIO_H
#define MCAL_GPIO_H

#include <MKL25Z4.h>
#include <stdint.h>

// MUX values (as defined in KL25Z datasheet)
#define GPIO_ALT_0 0 // Disabled
#define GPIO_ALT_1 1 // GPIO
#define GPIO_ALT_2 2 // Alternative 2 (e.g., UART0_RX/TX, TPM1_CH0, etc.)
#define GPIO_ALT_3 3 // Alternative 3 (e.g., I2C, TPM, etc.)
#define GPIO_ALT_4 4 // Alternative 4
#define GPIO_ALT_5 5 // Alternative 5
#define GPIO_ALT_6 6 // Alternative 6
#define GPIO_ALT_7 7 // Alternative 7 (Reserved or special function)

//clks
typedef enum {
    MCAL_PORTA = SIM_SCGC5_PORTA_MASK,
    MCAL_PORTB = SIM_SCGC5_PORTB_MASK,
    MCAL_PORTC = SIM_SCGC5_PORTC_MASK,
    MCAL_PORTD = SIM_SCGC5_PORTD_MASK,
    MCAL_PORTE = SIM_SCGC5_PORTE_MASK
} MCAL_PortClockMask_t;

//gpio port
typedef struct {
    PORT_Type           *port;
    GPIO_Type           *gpio;
    MCAL_PortClockMask_t clk_mask;
} MCAL_GPIO_Port_t;

//gpio dir / alt
typedef enum {
    GPIO_DIR_INPUT,
    GPIO_DIR_OUTPUT,
    GPIO_DIR_ALT
} GPIO_Direction_t;

//gpio
void MCAL_GPIO_EnableClock(const MCAL_GPIO_Port_t *port_info);

void MCAL_GPIO_ConfigOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

void MCAL_GPIO_ConfigInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

void MCAL_GPIO_ConfigAlt(const MCAL_GPIO_Port_t *port_info, uint32_t pin, GPIO_Direction_t direction, uint32_t mux);

void MCAL_GPIO_WritePin(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t value);

uint8_t MCAL_GPIO_ReadPin(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

//adc
void MCAL_ADC_Init(const MCAL_GPIO_Port_t *port_info, uint32_t pin);

void MCAL_ADC_StartConversion(uint8_t channel);

void MCAL_ADC_WaitConversion(void);

uint16_t MCAL_ADC_GetResult(void);

#endif // MCAL_GPIO_H
