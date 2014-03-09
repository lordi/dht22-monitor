#include "timer.h"
//#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32_ub_lcd_ili9341.h"
#include "stm32_ub_graphic2d.h"
#include "stm32_ub_font.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static __IO uint32_t uwTimingTimer;
static __IO uint32_t uwTimingDelay;

uint32_t timer_get()
{
  return uwTimingTimer;
}
void timer_start(__IO uint32_t nTime)
{
  uwTimingTimer = nTime;
}

void timer_block()
{
  while(uwTimingTimer != 0);
}

void timer_delay(__IO uint32_t nTime) {

  uwTimingDelay = nTime;
  while(uwTimingDelay != 0);
}

void timer_decrement(void) // called by SysTick_Handler
{
  if (uwTimingDelay != 0)
  {
    uwTimingDelay--;
  }
  if (uwTimingTimer != 0)
  {
    uwTimingTimer--;
  }
}

void timer_init(void) {
  if (SysTick_Config(SystemCoreClock / 1000000))
  {
    /* Capture error */ 
    while (1);
  }
}
