#include "hal_delay.h"
#include "MKL25Z4.h"

void HAL_Delay_Init(void) {
    // Inicializar SysTick para delays precisos
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;  // 1ms por tick
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_ENABLE_Msk;
}

void HAL_DelayMs(uint32_t ms) {
    while(ms--) {
        // 1ms basado en SysTick
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }
}

void HAL_DelayUs(uint32_t us) {
    // Implementación básica para delays cortos
    us = us * (SystemCoreClock / 1000000) / 4;
    while(us--);
}
