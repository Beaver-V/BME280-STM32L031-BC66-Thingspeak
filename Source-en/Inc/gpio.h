#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

extern void initGPIOA(void);
// extern void initGPIOB(void);
extern void quickLEDblink(void);
extern void slowLEDblink(void);

#endif /* GPIO_H_ */
