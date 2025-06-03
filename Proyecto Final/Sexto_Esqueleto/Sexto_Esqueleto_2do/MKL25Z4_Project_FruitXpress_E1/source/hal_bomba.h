#ifndef HAL_BOMBA_H
#define HAL_BOMBA_H

#include <stdbool.h>
#include "mcal_gpio.h"

typedef MCAL_Bomba_Config_t Bomba_Config_t;

typedef struct {
    const MCAL_GPIO_Port_t *port;
    uint32_t pin;
    uint8_t tanque_asociado;
} HAL_Bomba_Config_t;

void HAL_Bomba_Init(const HAL_Bomba_Config_t *config);
void HAL_Bomba_Activar(const HAL_Bomba_Config_t *config, bool estado);
bool HAL_Bomba_Estado(uint8_t tanque_idx);

extern const HAL_Bomba_Config_t bombas_config[3];

#endif
