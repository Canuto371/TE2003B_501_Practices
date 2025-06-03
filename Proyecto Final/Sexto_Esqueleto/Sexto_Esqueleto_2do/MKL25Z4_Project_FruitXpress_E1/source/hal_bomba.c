#include "hal_bomba.h"
#include "mcal_gpio.h"

static const HAL_Bomba_Config_t *bombas = bombas_config;

void HAL_Bomba_Init(const HAL_Bomba_Config_t *config) {
	for(int i = 0; i < 3; i++) {
	        MCAL_GPIO_EnableClock(bombas[i].port);
	        MCAL_GPIO_ConfigOutput(bombas[i].port, bombas[i].pin);
	        MCAL_GPIO_WritePin(bombas[i].port, bombas[i].pin, 0); // Inicia apagada
	}
}

void HAL_Bomba_Activar(uint8_t tanque_idx, bool estado) {
    if(tanque_idx >= 3) return;

    for(int i = 0; i < 3; i++) {
        if(bombas[i].tanque_asociado == tanque_idx) {
            MCAL_GPIO_WritePin(bombas[i].port, bombas[i].pin, estado ? 1 : 0);
            return;
        }
    }
}

bool HAL_Bomba_Estado(uint8_t tanque_idx) {
    if(tanque_idx >= 3) return false;

    for(int i = 0; i < 3; i++) {
        if(bombas[i].tanque_asociado == tanque_idx) {
            return MCAL_GPIO_ReadPin(bombas[i].port, bombas[i].pin);
        }
    }
    return false;
}
