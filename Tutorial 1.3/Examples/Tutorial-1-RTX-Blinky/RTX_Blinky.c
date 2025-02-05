/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2016 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/


#include "cmsis_os2.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons

#include "stm32l4xx.h"                  // Device header
#include <stdio.h>
#include "EventRecorder.h"

/*----------------------------------------------------------------------------
 * SystemCoreClockConfigure: configure SystemCoreClock using HSI
                             (HSE is not populated on Nucleo board)
 *----------------------------------------------------------------------------*/
void SystemCoreClockConfigure(void)
{

  RCC->CR |= ((uint32_t)RCC_CR_HSION);                     /* Enable HSI */
  while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  /* Wait for HSI Ready */

  RCC->CFGR = RCC_CFGR_SW_HSI;                             /* HSI is system clock */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  /* Wait for HSI used as system clock */

  FLASH->ACR  = (FLASH_ACR_PRFTEN     |                    /* Enable Prefetch Buffer */
                 FLASH_ACR_ICEN       |                    /* Instruction cache enable */
                 FLASH_ACR_DCEN       |                    /* Data cache enable */
                 FLASH_ACR_LATENCY_4WS );                  /* Flash 4 wait state */

  RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |                      /* HCLK = SYSCLK */
                RCC_CFGR_PPRE1_DIV2 |                      /* APB1 = HCLK/2 */
                RCC_CFGR_PPRE2_DIV1  );                    /* APB2 = HCLK/1 */

  RCC->CR &= ~RCC_CR_PLLON;                                /* Disable PLL */

  /* PLL configuration:  VCO = HSI/M * N,  Sysclk = VCO/R */
  RCC->PLLCFGR = (  0ul <<  4             |                /* PLL_M =   1 */
                 ( 10ul <<  8)            |                /* PLL_N =  10 */
                 (RCC_PLLCFGR_PLLSRC_HSI) |                /* PLL_SRC = HSI */
                 (  1ul << 21)            |                /* PLL_Q =   4 */
                 (RCC_PLLCFGR_PLLREN)     |
                 (  1ul << 25)             );              /* PLL_R =   4 */

  RCC->CR |= RCC_CR_PLLON;                                 /* Enable PLL */
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           /* Wait till PLL is ready */

  RCC->CFGR &= ~RCC_CFGR_SW;                               /* Select PLL as system clock source */
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  /* Wait till PLL is system clock src */
}

/*----------------------------------------------------------------------------
 * blinkLED: blink LED and check button state
 *----------------------------------------------------------------------------*/
void blinkLED(void *argument)
{
  int32_t max_num = LED_GetCount();
  int32_t num = 0;

  for (;;)
	{
    LED_On(num);                                           /* Turn specified LED on */
    osThreadFlagsWait(0x0001, osFlagsWaitAll, osWaitForever);
    LED_Off(num);                                          /* Turn specified LED off */
    osThreadFlagsWait(0x0001, osFlagsWaitAll, osWaitForever);

    num++;                                                 /* Change LED number */
    if (num >= max_num)
		{
      num = 0;                                             /* Restart with first LED */
    }
  }
}

osThreadId_t tid_blinkLED;

/*----------------------------------------------------------------------------
 * app_main: the main thread of the application
 *----------------------------------------------------------------------------*/
unsigned int counter = 0; /* Global variable */

void app_main (void *argument)
{
	uint32_t button_msk = (1U << Buttons_GetCount()) - 1;

  for (;;)
	{  
		counter++;
		if (counter > 0x10) counter = 0; /* Reset counter */
		printf("Counter equals %d\n", counter);
		/* main must not be terminated! */
    osDelay(500);
    while (Buttons_GetState() & (button_msk));             /* Wait while holding USER button */
    osThreadFlagsSet(tid_blinkLED, 0x0001);
  }
}

osThreadId_t tid_app_main;

/*----------------------------------------------------------------------------
 * main: initialize and start the system
 *----------------------------------------------------------------------------*/
int main (void)
{
  osKernelInitialize();                                   /* initialize CMSIS-RTOS */

  /* initialize peripherals */
  SystemCoreClockConfigure();                              /* configure System Clock */
  SystemCoreClockUpdate();

  LED_Initialize();                                        /* LED Initialization */
  Buttons_Initialize();                                    /* Buttons Initialization */

  /* create threads */
  tid_blinkLED = osThreadNew(blinkLED, NULL, NULL);
	tid_app_main = osThreadNew(app_main, NULL, NULL);

  osKernelStart();      

	EventRecorderInitialize(EventRecordAll, 1);	/* start thread execution */
}
