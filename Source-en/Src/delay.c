#include "delay.h"

volatile uint32_t ticks_delay = 0;

void __attribute__ ((interrupt, used)) SysTick_Handler(void) // ?
{
  ticks_delay++;
}

void delay(uint32_t milliseconds)
{
  uint32_t start = ticks_delay;
  while((ticks_delay - start) < milliseconds);
}
