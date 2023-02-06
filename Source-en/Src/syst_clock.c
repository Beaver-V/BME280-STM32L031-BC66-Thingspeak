#include "stm32l031xx.h"
#include "syst_clock.h"


//-------------- Init HSI16 and PLL as the System Clock -------------------
//
// HSI16 -> 32MHz the system clock ((16MHz/4)*16)/2=32MHz
//
void setHSIClock32M()
{
	// Enable power interface clock
	// RCC->APB1ENR |= (RCC_APB1ENR_PWREN);
	// After reset VOS[1:0]=10 so voltage range is 2 (1.5 V)
	// Set VOS[1:0]=01 for voltage range 1 (1.8 V)
	// PWR->CR = (PWR->CR & ~(PWR_CR_VOS)) | PWR_CR_VOS_0;

  /* test if the current clock source is something else than HSI */
  if ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
  {
    /* enable HSI */
    RCC->CR |= RCC_CR_HSION;
    /* wait until HSI becomes ready */
    while ( (RCC->CR & RCC_CR_HSIRDY) == 0 );

    /* enable the HSI "divide by 4" bit */
    RCC->CR |= (uint32_t)(RCC_CR_HSIDIVEN);
    /* wait until the "divide by 4" flag is enabled */
    while((RCC->CR & RCC_CR_HSIDIVF) == 0);

    /* then use the HSI clock */
    RCC->CFGR = (RCC->CFGR & (uint32_t) (~RCC_CFGR_SW)) | RCC_CFGR_SW_HSI;

    /* wait until HSI clock is used */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
  }

  /* disable PLL */
  RCC->CR &= (uint32_t)(~RCC_CR_PLLON);
  /* wait until PLL is inactive */
  while((RCC->CR & RCC_CR_PLLRDY) != 0);

  /* set latency to 1 wait state */
  FLASH->ACR |= FLASH_ACR_LATENCY;

  /* At this point the HSI runs with 4 MHz */
  /* Multiply by 16 devide by 2 --> 32 MHz */
  RCC->CFGR = (RCC->CFGR & (~(RCC_CFGR_PLLMUL| RCC_CFGR_PLLDIV ))) | (RCC_CFGR_PLLMUL16 | RCC_CFGR_PLLDIV2);

  /* enable PLL */
  RCC->CR |= RCC_CR_PLLON;

  /* wait until the PLL is ready */
  while ((RCC->CR & RCC_CR_PLLRDY) == 0);

  /* use the PLL has clock source */
  RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL);

  /* wait until the PLL source is active */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}


//--------------------- Init HSI16 as the System Clock -------------------
//
// HSI16 -> 16MHz the system clock
//
void setHSIClock16M()
{
	// Enable power interface clock
	// RCC->APB1ENR |= (RCC_APB1ENR_PWREN);
	// Select voltage scale 1 (1.65V - 1.95V) (01)  for VOS bits in PWR_CR
	// PWR->CR = (PWR->CR & ~(PWR_CR_VOS)) | PWR_CR_VOS_0;

  /* test if the current clock source is something else than HSI */
  if ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
  {
    /* enable HSI */
    RCC->CR |= RCC_CR_HSION;
    /* wait until HSI becomes ready */
    while ( (RCC->CR & RCC_CR_HSIRDY) == 0 );

    /* then use the HSI clock */
    RCC->CFGR = (RCC->CFGR & (uint32_t) (~RCC_CFGR_SW)) | RCC_CFGR_SW_HSI;

    /* wait until HSI clock is used */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
  }
}
