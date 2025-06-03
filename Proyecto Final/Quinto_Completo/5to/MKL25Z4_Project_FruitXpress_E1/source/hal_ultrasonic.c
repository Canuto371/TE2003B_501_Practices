// hal_ultrasonic.c
#include "hal_ultrasonic.h"
#include "ultrasonic_config.h"

static const Ultrasonic_Config_t *sensor_configs = NULL;
static uint32_t num_sensors = 0;

void HAL_Ultrasonic_Init(const Ultrasonic_Config_t *config, uint32_t num_sensors) {
    sensor_configs = config;
    num_sensors = num_sensors;

    for(uint32_t i = 0; i < num_sensors; i++) {
        MCAL_Ultrasonic_Init(&config[i]);
    }
}

uint32_t HAL_Ultrasonic_GetDistance(uint8_t sensor_index) {
    if(sensor_index >= num_sensors || sensor_configs == NULL) {
        return 0xFFFFFFFF; // Valor de error
    }
    return MCAL_Ultrasonic_MeasureDistance(&sensor_configs[sensor_index]);
}
