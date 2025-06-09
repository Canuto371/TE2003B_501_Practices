
#include <hal.h>
#include <MKL25Z4.h>
#include "driver_types.h"

//puertos
static const MCAL_GPIO_Port_t PORTB_INFO = { PORTB, PTB, MCAL_PORTB };
static const MCAL_GPIO_Port_t PORTD_INFO = { PORTD, PTD, MCAL_PORTD };
static const MCAL_GPIO_Port_t PORTE_INFO = { PORTE, PTE, MCAL_PORTE };

//pines
#define RED_LED_PIN    18  // PTB18
#define GREEN_LED_PIN  19  // PTB19
#define BLUE_LED_PIN    1  // PTD1

#define ADC0_CHANNEL    0  // canal 0
#define ADC0_PIN       20  // PTE20

static const Driver_Config_t config_table[] = {
    {
        .func = PIN_FUNC_GPIO_OUTPUT,
        .gpio = {
            .port_info = &PORTD_INFO,
            .pin        = BLUE_LED_PIN,
            .direction  = GPIO_DIR_OUTPUT,
            .mux        = GPIO_ALT_1
        }
    },
    {
        .func = PIN_FUNC_GPIO_OUTPUT,
        .gpio = {
            .port_info = &PORTB_INFO,
            .pin        = RED_LED_PIN,
            .direction  = GPIO_DIR_OUTPUT,
            .mux        = GPIO_ALT_1
        }
    },
    {
        .func = PIN_FUNC_GPIO_OUTPUT,
        .gpio = {
            .port_info = &PORTB_INFO,
            .pin        = GREEN_LED_PIN,
            .direction  = GPIO_DIR_OUTPUT,
            .mux        = GPIO_ALT_1
        }
    },
    {
        .func = PIN_FUNC_ADC,
        .adc = {
            .port_info = &PORTE_INFO,
            .pin        = ADC0_PIN,
            .channel    = ADC0_CHANNEL
        }
    }
};

void LED_set(int s) {

	if (s & 1)
		HAL_GPIO_Write(&PORTB_INFO, RED_LED_PIN, 0);
	else
		HAL_GPIO_Write(&PORTB_INFO, RED_LED_PIN, 1);

	if (s & 2)
		HAL_GPIO_Write(&PORTB_INFO, GREEN_LED_PIN, 0);
	else
		HAL_GPIO_Write(&PORTB_INFO, GREEN_LED_PIN, 1);

	if (s & 4)
		HAL_GPIO_Write(&PORTD_INFO, BLUE_LED_PIN, 0);
	else
		HAL_GPIO_Write(&PORTD_INFO, BLUE_LED_PIN, 1);
}

int main(void) {

    Driver_Init(config_table, sizeof(config_table)/sizeof(Driver_Config_t));

    while (1) {
        short int adc_result = HAL_ADC_Read(ADC0_CHANNEL);

		LED_set(adc_result >> 7);
    }

    return 0;
}
