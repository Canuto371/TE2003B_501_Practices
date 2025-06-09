
#include <hal.h>

//gpio
void HAL_GPIO_InitOutput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigOutput(port_info, pin);
}

void HAL_GPIO_InitInput(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigInput(port_info, pin);
}

void HAL_GPIO_InitAlt(const MCAL_GPIO_Port_t *port_info, uint32_t pin, GPIO_Direction_t direction, uint32_t mux) {
    MCAL_GPIO_EnableClock(port_info);
    MCAL_GPIO_ConfigAlt(port_info, pin, direction, mux);
}

void HAL_GPIO_Write(const MCAL_GPIO_Port_t *port_info, uint32_t pin, uint8_t state) {
    MCAL_GPIO_WritePin(port_info, pin, state);
}

uint8_t HAL_GPIO_Read(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    return MCAL_GPIO_ReadPin(port_info, pin);
}

//adc
void HAL_ADC_Init(const MCAL_GPIO_Port_t *port_info, uint32_t pin) {
    HAL_GPIO_InitAlt(port_info, pin, GPIO_DIR_INPUT, GPIO_ALT_0);
    MCAL_ADC_Init(port_info, pin);
}

uint16_t HAL_ADC_Read(uint8_t channel) {
    MCAL_ADC_StartConversion(channel);
    MCAL_ADC_WaitConversion();
    return MCAL_ADC_GetResult();
}

//inicializacion
void Driver_Init(const Driver_Config_t *config_table, uint32_t num_entries) {
    for (uint32_t i = 0; i < num_entries; i++) {
        switch (config_table[i].func) {
            case PIN_FUNC_GPIO_INPUT:
                HAL_GPIO_InitInput(
                    config_table[i].gpio.port_info,
                    config_table[i].gpio.pin
                );
                break;

            case PIN_FUNC_GPIO_OUTPUT:
                HAL_GPIO_InitOutput(
                    config_table[i].gpio.port_info,
                    config_table[i].gpio.pin
                );
                break;

            case PIN_FUNC_ADC:
                HAL_ADC_Init(
                    config_table[i].adc.port_info,
                    config_table[i].adc.pin
                );
                break;

            default:
                break;
        }
    }
}
