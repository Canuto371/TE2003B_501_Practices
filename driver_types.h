
#ifndef DRIVER_TYPES_H
#define DRIVER_TYPES_H

#include <mcal.h>
#include <stdint.h>

typedef enum {
    PIN_FUNC_GPIO_INPUT,
    PIN_FUNC_GPIO_OUTPUT,
    PIN_FUNC_ADC
} PinFunc_t;

typedef struct {
    PinFunc_t func;
    union {
        struct {
            const MCAL_GPIO_Port_t *port_info;
            uint32_t pin;
            GPIO_Direction_t direction;
            uint32_t mux;
        } gpio;
        struct {
            const MCAL_GPIO_Port_t *port_info;
            uint32_t pin;
            uint8_t channel;
        } adc;
    };
} Driver_Config_t;

#endif // DRIVER_TYPES_H
