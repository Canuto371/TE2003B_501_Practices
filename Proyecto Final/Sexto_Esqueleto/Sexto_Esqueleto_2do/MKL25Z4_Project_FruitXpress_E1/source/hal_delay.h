#ifndef HAL_DELAY_H
#define HAL_DELAY_H

#include <stdint.h>

void HAL_Delay_Init(void);
void HAL_DelayMs(uint32_t ms);
void HAL_DelayUs(uint32_t us);

#endif // HAL_DELAY_H
