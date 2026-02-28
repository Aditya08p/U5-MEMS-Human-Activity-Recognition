/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define T_SWO_Pin GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define User_Button_Pin GPIO_PIN_13
#define User_Button_GPIO_Port GPIOC
#define User_Button_EXTI_IRQn EXTI13_IRQn
#define LED_RED_Pin GPIO_PIN_6
#define LED_RED_GPIO_Port GPIOH
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOH
#define T_VCP_RX_Pin GPIO_PIN_10
#define T_VCP_RX_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_VCP_TX_Pin GPIO_PIN_9
#define T_VCP_TX_GPIO_Port GPIOA
#define Mems_VL53_xshut_Pin GPIO_PIN_1
#define Mems_VL53_xshut_GPIO_Port GPIOH
#define Mems_VLX_GPIO_Pin GPIO_PIN_5
#define Mems_VLX_GPIO_GPIO_Port GPIOG
#define Mems_INT_IIS2MDC_Pin GPIO_PIN_10
#define Mems_INT_IIS2MDC_GPIO_Port GPIOD
#define Mems_INT_IIS2MDC_EXTI_IRQn EXTI10_IRQn
#define Mems_INT_LPS22HH_Pin GPIO_PIN_2
#define Mems_INT_LPS22HH_GPIO_Port GPIOG
#define Mems_INT_LPS22HH_EXTI_IRQn EXTI2_IRQn
#define Mems_ISM330DLC_INT1_Pin GPIO_PIN_11
#define Mems_ISM330DLC_INT1_GPIO_Port GPIOE
#define Mems_ISM330DLC_INT1_EXTI_IRQn EXTI11_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
