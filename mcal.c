
#include <mcal.h>


//gpio
void MCAL_GPIO_EnableClock(const MCAL_GPIO_Port_t *port_info) {
    SIM->SCGC5 |= port_info->clk_mask;
}

void MCAL_GPIO_ConfigOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    port_info->port->PCR[pin] = PORT_PCR_MUX(1);
    port_info->gpio->PDDR |= (1UL << pin);
}

void MCAL_GPIO_ConfigInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    port_info->port->PCR[pin] = PORT_PCR_MUX(1)
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_PS(1);
    port_info->gpio->PDDR &= ~(1UL << pin);
}

void MCAL_GPIO_ConfigAlt(const MCAL_GPIO_Port_t *port_info, uint32_t pin, GPIO_Direction_t direction, uint32_t mux) {
    if (mux == GPIO_ALT_1) {
        if (direction == GPIO_DIR_OUTPUT) {
            MCAL_GPIO_ConfigOutput(port_info, pin);
        } else {
            MCAL_GPIO_ConfigInput(port_info, pin);
        }
    } else {
        port_info->port->PCR[pin] = PORT_PCR_MUX(mux);
        port_info->gpio->PDDR &= ~(1UL << pin);
    }
}

void MCAL_GPIO_WritePin(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t value) {
    if (value) {
        port_info->gpio->PSOR = (1UL << pin);
    } else {
    	port_info->gpio->PCOR = (1UL << pin);
    }
}

uint8_t MCAL_GPIO_ReadPin(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    return (uint8_t)((port_info->gpio->PDIR >> pin) & 0x1U);
}

//adc
void MCAL_ADC_Init(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    SIM->SCGC5 |= port_info->clk_mask;
    port_info->port->PCR[pin] = PORT_PCR_MUX(0);
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
    ADC0->SC2 &= ~(1U << 6);
    ADC0->CFG1 = (1U << 7)
               | (1U << 5)
               | (1U << 3)
               | (0U << 1)
               | (0U << 0);
}

void MCAL_ADC_StartConversion(uint8_t channel) {
    ADC0->SC1[0] = (channel & 0x1FU);
}

void MCAL_ADC_WaitConversion(void) {
    while (!(ADC0->SC1[0] & (1U << 7))) { }
}

uint16_t MCAL_ADC_GetResult(void) {
    return (uint16_t)(ADC0->R[0]);
}
