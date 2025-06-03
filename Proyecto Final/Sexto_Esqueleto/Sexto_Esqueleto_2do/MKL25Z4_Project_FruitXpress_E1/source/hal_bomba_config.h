#ifndef HAL_BOMBA_CONFIG_H
#define HAL_BOMBA_CONFIG_H

#include "hal_bomba.h"
#include "hal_gpio.h"

const HAL_Bomba_Config_t bombas_config[3] = {
    { // Bomba para tanque de Naranja (índice 0)
        .port = &GPIOA_Info,
        .pin = 1,          // PTA1
        .tanque_asociado = 0
    },
    { // Bomba para tanque de Uva (índice 1)
        .port = &GPIOA_Info,
        .pin = 2,          // PTA2
        .tanque_asociado = 1
    },
    { // Bomba para tanque de Fresa (índice 2)
        .port = &GPIOA_Info,
        .pin = 3,          // PTA3
        .tanque_asociado = 2
    }
};

#endif // HAL_BOMBA_CONFIG_H
