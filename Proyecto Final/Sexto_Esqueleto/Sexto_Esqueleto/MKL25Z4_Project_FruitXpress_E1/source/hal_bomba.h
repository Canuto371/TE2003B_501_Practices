#ifndef HAL_BOMBA_H
#define HAL_BOMBA_H

#include <stdbool.h>
#include "mcal_gpio.h"

typedef struct {
    const MCAL_GPIO_Port_t *port;
    uint32_t pin;
} HAL_Bomba_Config_t;

void HAL_Bomba_Init(const HAL_Bomba_Config_t *config);
void HAL_Bomba_Activar(const HAL_Bomba_Config_t *config, bool estado);

#endif
