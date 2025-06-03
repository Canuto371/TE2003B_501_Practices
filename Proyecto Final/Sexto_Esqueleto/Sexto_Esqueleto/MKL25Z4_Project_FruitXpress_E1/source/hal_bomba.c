#include "hal_bomba.h"

void HAL_Bomba_Init(const HAL_Bomba_Config_t *config) {
    MCAL_GPIO_EnableClock(config->port);
    MCAL_GPIO_ConfigOutput(config->port, config->pin);
    MCAL_GPIO_WritePin(config->port, config->pin, 0); // Inicia apagada
}

void HAL_Bomba_Activar(const HAL_Bomba_Config_t *config, bool estado) {
    MCAL_GPIO_WritePin(config->port, config->pin, estado ? 1 : 0);
}
