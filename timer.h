#include "stm32f4xx.h"

void timer_start(__IO uint32_t nTime);
void timer_block();
void timer_delay(__IO uint32_t nTime);
void timer_decrement(void); // called by SysTick_Handler
uint32_t timer_get();
void timer_init(void);
