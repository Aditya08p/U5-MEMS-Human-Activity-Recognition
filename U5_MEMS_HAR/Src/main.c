/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "icache.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "b_u585i_iot02a_bus.h"
#include "app_ism330dhcx.h"

/* AI BEGIN Includes */
#include "stai.h"
#include "network.h"
#include "network_data.h"
/* AI END Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t dataRdyIntReceived = 0;
volatile uint32_t inference_enable = 0;

STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)
float aiInData[STAI_NETWORK_IN_1_SIZE];
float aiOutData[STAI_NETWORK_OUT_1_SIZE];

#if defined ( __ICCARM__ )
#define AI_RAM   _Pragma("location=\"AI_RAM\"")
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#define AI_RAM   __attribute__((section(".AI_RAM")))
#else
#define AI_RAM
#endif

STAI_ALIGNED(32) AI_RAM static uint8_t activations[STAI_NETWORK_ACTIVATION_1_SIZE_BYTES];
stai_ptr data_activations[] = { activations };

static stai_size in_length, out_length;
static stai_ptr stai_input[STAI_NETWORK_IN_NUM];
static stai_ptr stai_output[STAI_NETWORK_OUT_NUM];

const char* activities[STAI_NETWORK_OUT_1_SIZE] = {
  "stationary", "left_right", "up_down"
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
/* USER CODE BEGIN PFP */
static void AI_Init(void);
static void AI_Run(float *pIn, float *pOut);
static uint32_t argmax(const float * values, uint32_t len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
	return len;
}

static void AI_Init(void) {
	stai_return_code ret_code;

	ret_code = stai_runtime_init();
	if (ret_code != STAI_SUCCESS) {
		printf("stai_runtime_init error\r\n");
		Error_Handler();
	}

	ret_code = stai_network_init(network_context);
	if (ret_code != STAI_SUCCESS) {
		printf("stai_network_init error\r\n");
		Error_Handler();
	}

	ret_code = stai_network_set_activations(network_context, data_activations, STAI_NETWORK_ACTIVATIONS_NUM);
	if (ret_code != STAI_SUCCESS) {
		printf("stai_network_set_activations error\r\n");
		Error_Handler();
	}

	ret_code = stai_network_get_inputs(network_context, stai_input, &in_length);
	ret_code = stai_network_get_outputs(network_context, stai_output, &out_length);
}

static void AI_Run(float *pIn, float *pOut) {
	stai_return_code ret_code;

	stai_input[0] = (stai_ptr)pIn;
	stai_output[0] = (stai_ptr)pOut;

	ret_code = stai_network_set_inputs(network_context, stai_input, STAI_NETWORK_IN_NUM);
	ret_code = stai_network_set_outputs(network_context, stai_output, STAI_NETWORK_OUT_NUM);

	ret_code = stai_network_run(network_context, STAI_MODE_SYNC);
	if (ret_code != STAI_SUCCESS) {
		ret_code = stai_network_get_error(network_context);
		printf("AI stai_network_run error\r\n");
		Error_Handler();
	}
}

static uint32_t argmax(const float *values, uint32_t len) {
	float max_value = values[0];
	uint32_t max_index = 0;
	for (uint32_t i = 1; i < len; i++) {
		if (values[i] > max_value) {
			max_value = values[i];
			max_index = i;
		}
	}
	return max_index;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  BSP_I2C2_Init();
  BSP_I2C2_IsReady(ISM330DHCX_I2C_ADD_H, 10);
  ism330dhcx_Init();
  AI_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t write_index = 0;
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	/* ISM330DHCX is in DRDY Interrupt mode, IRQ will handle sensor data. */
	if (dataRdyIntReceived) {
	  dataRdyIntReceived = 0U;
	  ism330dhcx_read_data_drdy();

	  if (inference_enable) {
		  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,GPIO_PIN_RESET);
		  /* Normalize data to [-1; 1] and accumulate into input buffer */
		  /* Note: window overlapping can be managed here */
		  aiInData[write_index + 0] = (float) acceleration_mg[0]
															  / 4000.0f;
		  aiInData[write_index + 1] = (float) acceleration_mg[1]
															  / 4000.0f;
		  aiInData[write_index + 2] = (float) acceleration_mg[2]
															  / 4000.0f;
		  aiInData[write_index + 3] = (float) angular_rate_mdps[0]
																/ 1000000.0f;
		  aiInData[write_index + 4] = (float) angular_rate_mdps[1]
																/ 1000000.0f;
		  aiInData[write_index + 5] = (float) angular_rate_mdps[2]
																/ 1000000.0f;
		  write_index += 6;

		  if (write_index == STAI_NETWORK_IN_1_SIZE) {
			  write_index = 0;

			  printf("Running inference\r\n");
			  AI_Run(aiInData, aiOutData);

			  /* Output results */
			  for (uint32_t i = 0; i < STAI_NETWORK_OUT_1_SIZE; i++) {
				  printf("%8.6f ", aiOutData[i]);
			  }
			  uint32_t class = argmax(aiOutData, STAI_NETWORK_OUT_1_SIZE);
			  printf(": %d - %s\r\n", (int) class, activities[class]);
			  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
		  }
	  } else {
			  printf("%lu %.2f %.2f %.2f %.2f %.2f %.2f\r\n", HAL_GetTick(),
					  acceleration_mg[0], acceleration_mg[1],
					  acceleration_mg[2], angular_rate_mdps[0],
					  angular_rate_mdps[1], angular_rate_mdps[2]);
			  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,GPIO_PIN_SET);
		  }
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_0;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
