/*
 ******************************************************************************
 * @file    app_hts221.c
 * @author  System Research and Applications Team
 * @brief   This file shows how to get data from HTS221 sensor.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* ATTENTION: By default the driver is little endian. If you need switch
 *            to big endian please see "Endianness definitions" in the
 *            header file of the driver (_reg.h).
 */

#define SENSOR_BUS hi2c2

/* Includes ------------------------------------------------------------------*/
#include "app_hts221.h"
#include "stm32u5xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "b_u585i_iot02a_bus.h"

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static int16_t data_raw_humidity;
static int16_t data_raw_temperature;
static float_t humidity_perc;
static float_t temperature_degC;
static uint8_t hts221_whoamI;
static uint8_t tx_buffer[1000];
/*
 *  Function used to apply coefficient
 */
typedef struct {
  float_t x0;
  float_t y0;
  float_t x1;
  float_t y1;
} lin_t;
stmdev_ctx_t hts221_dev_ctx;
lin_t lin_hum;
lin_t lin_temp;

/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);


float_t linear_interpolation(lin_t *lin, int16_t x)
{
  return ((lin->y1 - lin->y0) * x + ((lin->x1 * lin->y0) -
                                     (lin->x0 * lin->y1)))
         / (lin->x1 - lin->x0);
}
/* Main Example --------------------------------------------------------------*/
void hts221_Init(void) {
	/* Initialize platform specific hardware */
	platform_init();
	/* Initialize mems driver interface */
	hts221_dev_ctx.write_reg = platform_write;
	hts221_dev_ctx.read_reg = platform_read;
	hts221_dev_ctx.mdelay = platform_delay;
	hts221_dev_ctx.handle = &SENSOR_BUS;
	/* Check device ID */
	hts221_whoamI = 0;
	hts221_device_id_get(&hts221_dev_ctx, &hts221_whoamI);

	if (hts221_whoamI != HTS221_ID){
		printf("HTS221_ID: %d\r\n", hts221_whoamI);
		while (1); /*manage here device not found */
	}

	/* Read humidity calibration coefficient */
	hts221_hum_adc_point_0_get(&hts221_dev_ctx, &lin_hum.x0);
	hts221_hum_rh_point_0_get(&hts221_dev_ctx, &lin_hum.y0);
	hts221_hum_adc_point_1_get(&hts221_dev_ctx, &lin_hum.x1);
	hts221_hum_rh_point_1_get(&hts221_dev_ctx, &lin_hum.y1);
	/* Read temperature calibration coefficient */
	hts221_temp_adc_point_0_get(&hts221_dev_ctx, &lin_temp.x0);
	hts221_temp_deg_point_0_get(&hts221_dev_ctx, &lin_temp.y0);
	hts221_temp_adc_point_1_get(&hts221_dev_ctx, &lin_temp.x1);
	hts221_temp_deg_point_1_get(&hts221_dev_ctx, &lin_temp.y1);
	/* Enable Block Data Update */
	hts221_block_data_update_set(&hts221_dev_ctx, PROPERTY_ENABLE);
	/* Set Output Data Rate */
	hts221_data_rate_set(&hts221_dev_ctx, HTS221_ODR_12Hz5);
	/* Device power on */
	hts221_power_on_set(&hts221_dev_ctx, PROPERTY_ENABLE);
}

void hts221_read_data_polling(void) {
	/* Read samples in polling mode */
	/* Read output only if new value is available */
	hts221_status_reg_t status;
	hts221_status_get(&hts221_dev_ctx, &status);

	if (status.h_da) {
		/* Read humidity data */
		memset(&data_raw_humidity, 0x00, sizeof(int16_t));
		hts221_humidity_raw_get(&hts221_dev_ctx, &data_raw_humidity);
		humidity_perc = linear_interpolation(&lin_hum, data_raw_humidity);

		if (humidity_perc < 0) {
			humidity_perc = 0;
		}

		if (humidity_perc > 100) {
			humidity_perc = 100;
		}

		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"Humidity [%%]:%3.2f\r\n", humidity_perc);
		tx_com(tx_buffer, strlen((char const*) tx_buffer));
	}

	if (status.t_da) {
		/* Read temperature data */
		memset(&data_raw_temperature, 0x00, sizeof(int16_t));
		hts221_temperature_raw_get(&hts221_dev_ctx, &data_raw_temperature);
		temperature_degC = linear_interpolation(&lin_temp,
				data_raw_temperature);
		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"Temperature [degC]:%6.2f\r\n", temperature_degC);
		tx_com(tx_buffer, strlen((char const*) tx_buffer));
	}
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 * @retval BSP Status
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len) {
	(void) handle; /* handle is unused when using BSP_ function */

	return BSP_I2C2_WriteReg(HTS221_I2C_ADDRESS, reg, bufp, len);
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 * @retval BSP Status
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len) {
	(void) handle; /* handle is unused when using BSP_ function */

	return BSP_I2C2_ReadReg(HTS221_I2C_ADDRESS, reg, bufp, len);
}

/*
 * @brief  Send buffer to console (platform dependent)
 *
 * @param  tx_buffer     buffer to transmit
 * @param  len           number of byte to send
 * @retval none
 */
static void tx_com(uint8_t *tx_buffer, uint16_t len) {
	HAL_UART_Transmit(&huart1, tx_buffer, len, 1000);
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 * @retval none
 */
static void platform_delay(uint32_t ms) {
	HAL_Delay(ms);
}

/*
 * @brief  platform specific initialization (platform dependent)
 * @retval none
 */
static void platform_init(void) {
	/* Add board-specific init if needed (I2C/UART already init by CubeMX) */
	return;
}
