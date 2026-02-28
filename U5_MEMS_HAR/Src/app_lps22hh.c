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
#include "app_lps22hh.h"
#include "stm32u5xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "b_u585i_iot02a_bus.h"

/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME        5 //ms

#define TX_BUF_DIM          1000

/* Private variables ---------------------------------------------------------*/
static uint32_t data_raw_pressure;
static int16_t data_raw_temperature;
static float_t pressure_hPa;
static float_t temperature_degC;
static uint8_t lps22hh_whoamI, rst;
static uint8_t tx_buffer[TX_BUF_DIM];

stmdev_ctx_t lps22hh_dev_ctx;
lps22hh_status_t status;
/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_delay(uint32_t ms);
static void platform_init(void);

/* Main Example --------------------------------------------------------------*/

void lps22hh_Init(void)
{
  /* Initialize mems driver interface */
  lps22hh_dev_ctx.write_reg = platform_write;
  lps22hh_dev_ctx.read_reg = platform_read;
  lps22hh_dev_ctx.mdelay = platform_delay;
  lps22hh_dev_ctx.handle = &SENSOR_BUS;
  /* Initialize platform specific hardware */
  platform_init();
  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  /* Check device ID */
  lps22hh_whoamI = 0;
  lps22hh_device_id_get(&lps22hh_dev_ctx, &lps22hh_whoamI);

  if ( lps22hh_whoamI != LPS22HH_ID )
    while (1); /*manage here device not found */

  /* Restore default configuration */
  lps22hh_reset_set(&lps22hh_dev_ctx, PROPERTY_ENABLE);

  do {
    lps22hh_reset_get(&lps22hh_dev_ctx, &rst);
  } while (rst);

  /* Enable Block Data Update */
  lps22hh_block_data_update_set(&lps22hh_dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate */
  lps22hh_data_rate_set(&lps22hh_dev_ctx, LPS22HH_25_Hz_LOW_NOISE);

}

void lps22hh_read_data_polling(void)
/* Read samples in polling mode (no int) */
{
	/* Read output only if new value is available */
	lps22hh_read_reg(&lps22hh_dev_ctx, LPS22HH_STATUS, (uint8_t*) &status, 1);

	if (status.p_da) {
		memset(&data_raw_pressure, 0x00, sizeof(uint32_t));
		lps22hh_pressure_raw_get(&lps22hh_dev_ctx, &data_raw_pressure);
		pressure_hPa = lps22hh_from_lsb_to_hpa(data_raw_pressure);
		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"pressure [hPa]:%6.2f\r\n", pressure_hPa);
		tx_com(tx_buffer, strlen((char const*) tx_buffer));
	}

	if (status.t_da) {
		memset(&data_raw_temperature, 0x00, sizeof(int16_t));
		lps22hh_temperature_raw_get(&lps22hh_dev_ctx, &data_raw_temperature);
		temperature_degC = lps22hh_from_lsb_to_celsius(data_raw_temperature);
		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"temperature [degC]:%6.2f\r\n", temperature_degC);
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

	return BSP_I2C2_WriteReg(LPS22HH_I2C_ADD_H, reg, bufp, len);
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

	return BSP_I2C2_ReadReg(LPS22HH_I2C_ADD_H, reg, bufp, len);
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
