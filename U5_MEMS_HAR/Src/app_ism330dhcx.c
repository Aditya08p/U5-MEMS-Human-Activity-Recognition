/*
 ******************************************************************************
 * @file    app_ism330dhcx.c
 * @author  System Research and Applications Team
 * @brief   This file shows how to get data from ISM330DHCX sensor.
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
#include "app_ism330dhcx.h"
#include "stm32u5xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "b_u585i_iot02a_bus.h"

/* Private macro -------------------------------------------------------------*/
#define BOOT_TIME          10U // ms

/* Private variables ---------------------------------------------------------*/
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;
static uint8_t ism330dhcx_whoamI, rst;
static uint8_t tx_buffer[1000];
static stmdev_ctx_t ism330dhcx_dev_ctx;

/* Private function prototypes -----------------------------------------------*/
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len);
static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Sensor initialization function
 * @retval none
 */
void ism330dhcx_Init(void) {

	ism330dhcx_pin_int1_route_t int1_route;
	/* Initialize mems driver interface */
	ism330dhcx_dev_ctx.write_reg = platform_write;
	ism330dhcx_dev_ctx.read_reg = platform_read;
	ism330dhcx_dev_ctx.mdelay = platform_delay;
	ism330dhcx_dev_ctx.handle = &SENSOR_BUS;

	/* Init test platform */
	platform_init();

	/* Wait sensor boot time */
	platform_delay(BOOT_TIME);

	/* Check device ID */
	ism330dhcx_device_id_get(&ism330dhcx_dev_ctx, &ism330dhcx_whoamI);

	if (ism330dhcx_whoamI != ISM330DHCX_ID) {
		printf("ISM330DHCX_ID: %d\r\n", ism330dhcx_whoamI);
		while (1);
	}

	/* Restore default configuration */
	ism330dhcx_reset_set(&ism330dhcx_dev_ctx, PROPERTY_ENABLE);

	do {
		ism330dhcx_reset_get(&ism330dhcx_dev_ctx, &rst);
	} while (rst);

	/* Start device configuration. */
	ism330dhcx_device_conf_set(&ism330dhcx_dev_ctx, PROPERTY_ENABLE);
	/* Enable Block Data Update */
	ism330dhcx_block_data_update_set(&ism330dhcx_dev_ctx, PROPERTY_ENABLE);
	/* Set Output Data Rate */
	ism330dhcx_xl_data_rate_set(&ism330dhcx_dev_ctx, ISM330DHCX_XL_ODR_52Hz);
	ism330dhcx_gy_data_rate_set(&ism330dhcx_dev_ctx, ISM330DHCX_GY_ODR_52Hz);
	/* Set full scale */
	ism330dhcx_xl_full_scale_set(&ism330dhcx_dev_ctx, ISM330DHCX_4g);
	ism330dhcx_gy_full_scale_set(&ism330dhcx_dev_ctx, ISM330DHCX_500dps);
	/* Choose data‑ready notification mode (pulsed or latched) */
	ism330dhcx_data_ready_mode_set(&ism330dhcx_dev_ctx, ISM330DHCX_DRDY_PULSED);

    /* Configure INT1 pin electrical behaviour */
    ism330dhcx_pin_mode_set(&ism330dhcx_dev_ctx, ISM330DHCX_PUSH_PULL);        // push-pull
    ism330dhcx_pin_polarity_set(&ism330dhcx_dev_ctx, ISM330DHCX_ACTIVE_HIGH);  // active high

    /* Route DRDY to INT1: read current routing, modify DRDY bits and write back */
    ism330dhcx_pin_int1_route_get(&ism330dhcx_dev_ctx, &int1_route);
    /* enable accel DRDY on INT1 */
    int1_route.int1_ctrl.int1_drdy_xl = PROPERTY_ENABLE;
    int1_route.int1_ctrl.int1_drdy_g = PROPERTY_ENABLE;
    ism330dhcx_pin_int1_route_set(&ism330dhcx_dev_ctx, &int1_route);


	/* Configure filtering chain (No aux interface)
	 *
	 * Accelerometer - LPF1 + LPF2 path
	 */
	ism330dhcx_xl_hp_path_on_out_set(&ism330dhcx_dev_ctx, ISM330DHCX_LP_ODR_DIV_100);
	ism330dhcx_xl_filter_lp2_set(&ism330dhcx_dev_ctx, PROPERTY_ENABLE);

}

/* Main Example --------------------------------------------------------------*/
/**
 * @brief  Read data from sensor in polling mode
 * @retval none
 */
void ism330dhcx_read_data_polling(void) {
	/* Read samples in polling mode (no int) */
	uint8_t reg;

	/* Read output only if new xl value is available */
	ism330dhcx_xl_flag_data_ready_get(&ism330dhcx_dev_ctx, &reg);

	if (reg) {
		/* Read acceleration field data */
		memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
		ism330dhcx_acceleration_raw_get(&ism330dhcx_dev_ctx, data_raw_acceleration);
		acceleration_mg[0] = ism330dhcx_from_fs4g_to_mg(
				data_raw_acceleration[0]);
		acceleration_mg[1] = ism330dhcx_from_fs4g_to_mg(
				data_raw_acceleration[1]);
		acceleration_mg[2] = ism330dhcx_from_fs4g_to_mg(
				data_raw_acceleration[2]);
		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"%4.2f %4.2f %4.2f ", acceleration_mg[0],
				acceleration_mg[1], acceleration_mg[2]);
		tx_com(tx_buffer, (uint16_t) strlen((char const*) tx_buffer));
	}

	ism330dhcx_gy_flag_data_ready_get(&ism330dhcx_dev_ctx, &reg);

	if (reg) {
		/* Read angular rate field data */
		memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
		ism330dhcx_angular_rate_raw_get(&ism330dhcx_dev_ctx, data_raw_angular_rate);
		angular_rate_mdps[0] = ism330dhcx_from_fs500dps_to_mdps(
				data_raw_angular_rate[0]);
		angular_rate_mdps[1] = ism330dhcx_from_fs500dps_to_mdps(
				data_raw_angular_rate[1]);
		angular_rate_mdps[2] = ism330dhcx_from_fs500dps_to_mdps(
				data_raw_angular_rate[2]);
		snprintf((char*) tx_buffer, sizeof(tx_buffer),
				"%4.2f %4.2f %4.2f\r\n",
				angular_rate_mdps[0], angular_rate_mdps[1],
				angular_rate_mdps[2]);
		tx_com(tx_buffer, (uint16_t) strlen((char const*) tx_buffer));
	}

	ism330dhcx_temp_flag_data_ready_get(&ism330dhcx_dev_ctx, &reg);

//	if (reg) {
//		/* Read temperature data */
//		memset(&data_raw_temperature, 0x00, sizeof(int16_t));
//		ism330dhcx_temperature_raw_get(&ism330dhcx_dev_ctx, &data_raw_temperature);
//		temperature_degC = ism330dhcx_from_lsb_to_celsius(data_raw_temperature);
//		snprintf((char*) tx_buffer, sizeof(tx_buffer),
//				"Temperature [degC]:%6.2f\r\n", temperature_degC);
//		tx_com(tx_buffer, (uint16_t) strlen((char const*) tx_buffer));
//	}
}

void ism330dhcx_read_data_drdy(void){
	uint8_t drdy;

	/* accel */
	ism330dhcx_xl_flag_data_ready_get(&ism330dhcx_dev_ctx, &drdy);
	if (drdy)
	{
		ism330dhcx_acceleration_raw_get(&ism330dhcx_dev_ctx, data_raw_acceleration);
		acceleration_mg[0] = ism330dhcx_from_fs4g_to_mg(data_raw_acceleration[0]);
		acceleration_mg[1] = ism330dhcx_from_fs4g_to_mg(data_raw_acceleration[1]);
		acceleration_mg[2] = ism330dhcx_from_fs4g_to_mg(data_raw_acceleration[2]);
		snprintf((char *)tx_buffer, sizeof(tx_buffer), "%4.2f %4.2f %4.2f ", acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
		tx_com(tx_buffer, (uint16_t)strlen((char const *)tx_buffer));
	}

	/* gyro */
	ism330dhcx_gy_flag_data_ready_get(&ism330dhcx_dev_ctx, &drdy);
	if (drdy)
	{
		ism330dhcx_angular_rate_raw_get(&ism330dhcx_dev_ctx, data_raw_angular_rate);
		angular_rate_mdps[0] = ism330dhcx_from_fs500dps_to_mdps(data_raw_angular_rate[0]);
		angular_rate_mdps[1] = ism330dhcx_from_fs500dps_to_mdps(data_raw_angular_rate[1]);
		angular_rate_mdps[2] = ism330dhcx_from_fs500dps_to_mdps(data_raw_angular_rate[2]);
		snprintf((char *)tx_buffer, sizeof(tx_buffer), "%4.2f %4.2f %4.2f\r\n", angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
		tx_com(tx_buffer, (uint16_t)strlen((char const *)tx_buffer));
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

	return BSP_I2C2_WriteReg(ISM330DHCX_I2C_ADD_H, reg, bufp, len);
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

	return BSP_I2C2_ReadReg(ISM330DHCX_I2C_ADD_H, reg, bufp, len);
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
