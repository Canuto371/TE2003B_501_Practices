// mcal_ultrasonic.c
#include "mcal_ultrasonic.h"
#include "mcal_gpio.h"

void MCAL_Ultrasonic_Init(const MCAL_Ultrasonic_Config_t *config) {
    // Configurar trigger como salida
    MCAL_GPIO_ConfigOutput(config->trigger_port, config->trigger_pin);
    MCAL_GPIO_WritePin(config->trigger_port, config->trigger_pin, 0);

    // Configurar echo como entrada
    MCAL_GPIO_ConfigInput(config->echo_port, config->echo_pin);
}

uint32_t MCAL_Ultrasonic_MeasureDistance(const MCAL_Ultrasonic_Config_t *config) {
    uint32_t start_time = 0, end_time = 0;

    // Generar pulso de trigger (10us)
    MCAL_GPIO_WritePin(config->trigger_port, config->trigger_pin, 1);
    MCAL_DelayUs(10);
    MCAL_GPIO_WritePin(config->trigger_port, config->trigger_pin, 0);

    // Esperar flanco de subida en echo
    while(!MCAL_GPIO_ReadPin(config->echo_port, config->echo_pin));
    start_time = MCAL_GetMicroseconds();

    // Esperar flanco de bajada en echo
    while(MCAL_GPIO_ReadPin(config->echo_port, config->echo_pin));
    end_time = MCAL_GetMicroseconds();

    // Calcular distancia en cm (us/58 según datasheet)
    return (end_time - start_time) / 58;
}
