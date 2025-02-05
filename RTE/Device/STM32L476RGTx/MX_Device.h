/******************************************************************************
 * File Name   : MX_Device.h
 * Date        : 05/02/2025 14:41:28
 * Description : STM32Cube MX parameter definitions
 * Note        : This file is generated by STM32CubeMX (DO NOT EDIT!)
 ******************************************************************************/

#ifndef __MX_DEVICE_H
#define __MX_DEVICE_H

/*---------------------------- Clock Configuration ---------------------------*/

#define MX_LSI_VALUE                            32000
#define MX_LSE_VALUE                            32768
#define MX_HSI_VALUE                            16000000
#define MX_HSE_VALUE                            8000000
#define MX_SYSCLKFreq_VALUE                     40000000
#define MX_HCLKFreq_Value                       40000000
#define MX_FCLKCortexFreq_Value                 40000000
#define MX_CortexFreq_Value                     40000000
#define MX_AHBFreq_Value                        40000000
#define MX_APB1Freq_Value                       40000000
#define MX_APB2Freq_Value                       40000000
#define MX_APB1TimFreq_Value                    40000000
#define MX_APB2TimFreq_Value                    40000000
#define MX_PWRFreq_Value                        40000000
#define MX_RTCFreq_Value                        32000
#define MX_USBFreq_Value                        16000000
#define MX_WatchDogFreq_Value                   32000
#define MX_MCO1PinFreq_Value                    40000000

/*-------------------------------- SYS        --------------------------------*/

#define MX_SYS                                  1

/* GPIO Configuration */

/* Pin PA13 (JTMS-SWDIO) */
#define MX_SYS_JTMS_SWDIO_Pin                   PA13_JTMS_SWDIO

/* Pin PA14 (JTCK-SWCLK) */
#define MX_SYS_JTCK_SWCLK_Pin                   PA14_JTCK_SWCLK

/* Pin PB3 (JTDO-TRACESWO) */
#define MX_SYS_JTDO_SWO_Pin                     PB3_JTDO_TRACESWO

/*-------------------------------- TIM1       --------------------------------*/

#define MX_TIM1                                 1

/* GPIO Configuration */

/* Pin PA8 */
#define MX_S_TIM1_CH1_GPIO_ModeDefaultPP        GPIO_MODE_AF_PP
#define MX_S_TIM1_CH1_GPIO_Speed                GPIO_SPEED_FREQ_LOW
#define MX_S_TIM1_CH1_Pin                       PA8
#define MX_S_TIM1_CH1_GPIOx                     GPIOA
#define MX_S_TIM1_CH1_GPIO_PuPd                 GPIO_NOPULL
#define MX_S_TIM1_CH1_GPIO_Pin                  GPIO_PIN_8
#define MX_S_TIM1_CH1_GPIO_AF                   GPIO_AF1_TIM1

/* Pin PA9 */
#define MX_S_TIM1_CH2_GPIO_ModeDefaultPP        GPIO_MODE_AF_PP
#define MX_S_TIM1_CH2_GPIO_Speed                GPIO_SPEED_FREQ_LOW
#define MX_S_TIM1_CH2_Pin                       PA9
#define MX_S_TIM1_CH2_GPIOx                     GPIOA
#define MX_S_TIM1_CH2_GPIO_PuPd                 GPIO_NOPULL
#define MX_S_TIM1_CH2_GPIO_Pin                  GPIO_PIN_9
#define MX_S_TIM1_CH2_GPIO_AF                   GPIO_AF1_TIM1

/*-------------------------------- TIM3       --------------------------------*/

#define MX_TIM3                                 1

/* GPIO Configuration */

/* Pin PB4 (NJTRST) */
#define MX_S_TIM3_CH1_GPIO_ModeDefaultPP        GPIO_MODE_AF_PP
#define MX_S_TIM3_CH1_GPIO_Speed                GPIO_SPEED_FREQ_LOW
#define MX_S_TIM3_CH1_Pin                       PB4_NJTRST
#define MX_S_TIM3_CH1_GPIOx                     GPIOB
#define MX_S_TIM3_CH1_GPIO_PuPd                 GPIO_NOPULL
#define MX_S_TIM3_CH1_GPIO_Pin                  GPIO_PIN_4
#define MX_S_TIM3_CH1_GPIO_AF                   GPIO_AF2_TIM3

/* Pin PA7 */
#define MX_S_TIM3_CH2_GPIO_ModeDefaultPP        GPIO_MODE_AF_PP
#define MX_S_TIM3_CH2_GPIO_Speed                GPIO_SPEED_FREQ_LOW
#define MX_S_TIM3_CH2_Pin                       PA7
#define MX_S_TIM3_CH2_GPIOx                     GPIOA
#define MX_S_TIM3_CH2_GPIO_PuPd                 GPIO_NOPULL
#define MX_S_TIM3_CH2_GPIO_Pin                  GPIO_PIN_7
#define MX_S_TIM3_CH2_GPIO_AF                   GPIO_AF2_TIM3

/*-------------------------------- NVIC       --------------------------------*/

#define MX_NVIC                                 1

/*-------------------------------- GPIO       --------------------------------*/

#define MX_GPIO                                 1

/* GPIO Configuration */

/* Pin PA6 */
#define MX_PA6_GPIO_Speed                       GPIO_SPEED_FREQ_LOW
#define MX_PA6_Pin                              PA6
#define MX_PA6_GPIOx                            GPIOA
#define MX_PA6_PinState                         GPIO_PIN_RESET
#define MX_PA6_GPIO_PuPd                        GPIO_NOPULL
#define MX_PA6_GPIO_Pin                         GPIO_PIN_6
#define MX_PA6_GPIO_ModeDefaultOutputPP         GPIO_MODE_OUTPUT_PP

/* Pin PA5 */
#define MX_PA5_GPIO_Speed                       GPIO_SPEED_FREQ_LOW
#define MX_PA5_Pin                              PA5
#define MX_PA5_GPIOx                            GPIOA
#define MX_PA5_PinState                         GPIO_PIN_RESET
#define MX_PA5_GPIO_PuPd                        GPIO_NOPULL
#define MX_PA5_GPIO_Pin                         GPIO_PIN_5
#define MX_PA5_GPIO_ModeDefaultOutputPP         GPIO_MODE_OUTPUT_PP

#endif  /* __MX_DEVICE_H */

