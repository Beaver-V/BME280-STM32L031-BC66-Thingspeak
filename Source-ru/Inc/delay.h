#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>

extern volatile uint32_t ticks_delay;

extern void delay(uint32_t milliseconds);

#endif /* DELAY_H_ */
