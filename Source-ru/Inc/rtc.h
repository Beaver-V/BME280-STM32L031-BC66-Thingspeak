#ifndef RTC_H_
#define RTC_H_

/*************************	FUNCTION PROTOTYPE	******************************/
void initRTC(void);
void initWakeup(uint32_t wakeup_period); // 30.01.2022
void RTC_IRQHandler(void);

#endif /* RTC_H_ */
