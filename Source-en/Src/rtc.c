/*
 * Code from here:
 * http://stefanfrings.de/stm32/stm32l0.html#rtc
 * and from here:
 * https://forum.digikey.com/t/low-power-modes-on-the-stm32l0-series/13306
 * and more from here:
 * D:\...\STM32L0xx_Snippets_Package_V1.2.0\Projects\RTC\02_ProgrammingTheWakeUpTimer
 */

#include "stm32l0xx.h"
#include "delay.h"
#include "rtc.h"
#include "syst_clock.h"
#include <lpuart.h>


void initRTC()
{
    // Enable the power interface
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Enable access to the backup domain
    PWR->CR |= PWR_CR_DBP;

    // Enable LSE oscillator with medium driver power
    RCC->CSR &= ~RCC_CSR_LSEDRV_Msk;	/* lowest drive */
    RCC->CSR  |= RCC_CSR_LSEDRV_1;		/* medium drive */
    RCC->CSR |= RCC_CSR_LSEON;			/* enable low speed external clock */

    // Wait until LSE oscillator is ready
    while (!(RCC->CSR & RCC_CSR_LSERDY));

    // Disable access to the backup domain
    // PWR->CR &=~ PWR_CR_DBP;

    // Select LSE as clock source for the RTC
    RCC->CSR &= ~RCC_CSR_RTCSEL_Msk;	/* no clock selection for RTC */
    RCC->CSR |= RCC_CSR_RTCSEL_LSE;		/* select LSE */

    // Enable the RTC
    RCC->CSR |= RCC_CSR_RTCEN;			/* enable RTC */
}

void initWakeup(uint32_t wakeup_period)
{
    /* disable RTC write protection */
    RTC->WPR = 0x0ca;
    RTC->WPR = 0x053;

    // Stop the wakeup timer to allow configuration update
    RTC->CR &=~ RTC_CR_WUTE; 			/* disable wakeup timer for reprogramming */

    // Wait until the wakeup timer is ready for configuration update
    while((RTC->ISR & RTC_ISR_WUTWF) != RTC_ISR_WUTWF);

    // Clock source of the wakeup timer is 1 Hz
    RTC->CR &= ~RTC_CR_WUCKSEL;			/* clear selection register */
    RTC->CR |= RTC_CR_WUCKSEL_2;		/* select the 1Hz clock */

    // The wakeup period is 0+1 clock pulses
    RTC->WUTR = wakeup_period; // 30.01.2022

    // Enable the wakeup timer with interrupts
    RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE;

    // Switch the write protection back on
    RTC->WPR = 0;						/* enable RTC write protection */
    RTC->WPR = 0;

    // Enable EXTI20 interrupt on rising edge
    /* wake up IRQ is connected to line 20 */
    EXTI->RTSR |= EXTI_RTSR_RT20;		/* rising edge for wake up line */
    EXTI->IMR |= EXTI_IMR_IM20;			/* interrupt enable */

    NVIC_EnableIRQ(RTC_IRQn);

    // Clear (old) pending interrupt flag
    RTC->ISR &= ~RTC_ISR_WUTF;
    EXTI->PR |= EXTI_PR_PR20;
}

void RTC_IRQHandler()
{
    // Clear interrupt flag
    RTC->ISR &= ~RTC_ISR_WUTF;
    EXTI->PR |= EXTI_PR_PR20;
}
