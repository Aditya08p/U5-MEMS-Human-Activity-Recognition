/*
 ******************************************************************************
 * @file    app_iis2mdc.c
 * @author  System Research and Applications Team
 * @brief   This file shows how to get data from IIS2MDC sensor.
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
#include "app_iis2mdc.h"
#include "stm32u5xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "b_u585i_iot02a_bus.h"

/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME        20 // ms

/* Private variables ---------------------------------------------------------*/
static int16_t data_raw_temperature;
static int16_t data_raw_magnetic[3];
static float_t temperature_degC;
static float_t magnetic_mG[3];
static uint8_t iis2mdc_whoamI, rst, drdy;
static uint8_t tx_buffer[1000];
stmdev_ctx_t iis2mdc_dev_ctx;

/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_delay(uint32_t ms);
static void platform_init(void);

/* Main Example --------------------------------------------------------------*/
void iis2mdc_Init(void)
{
  /* Initialize mems driver interface */
  iis2mdc_dev_ctx.write_reg = platform_write;
  iis2mdc_dev_ctx.read_reg = platform_read;
  iis2mdc_dev_ctx.mdelay = platform_delay;
  iis2mdc_dev_ctx.handle = &SENSOR_BUS;
  /* Initialize platform specific hardware */
  platform_init();
  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  /* Check device ID */
  iis2mdc_whoamI = 0;
  iis2mdc_device_id_get(&iis2mdc_dev_ctx, &iis2mdc_whoamI);

  if ( iis2mdc_whoamI != IIS2MDC_ID ){
	  printf("IIS2MDC_ID: %d\r\n", iis2mdc_whoamI);
      while (1);
  }
  /* Restore default configuration */
  iis2mdc_reset_set(&iis2mdc_dev_ctx, PROPERTY_ENABLE);

  do {
    iis2mdc_reset_get(&iis2mdc_dev_ctx, &rst);
  } while (rst);

  /* Enable Block Data Update */
  iis2mdc_block_data_update_set(&iis2mdc_dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate */
  iis2mdc_data_rate_set(&iis2mdc_dev_ctx, IIS2MDC_ODR_20Hz);
  /* Set / Reset sensor mode */
  iis2mdc_set_rst_mode_set(&iis2mdc_dev_ctx, IIS2MDC_SENS_OFF_CANC_EVERY_ODR);
  /* Enable temperature compensation */
  iis2mdc_offset_temp_comp_set(&iis2mdc_dev_ctx, PROPERTY_ENABLE);
  /* Set device in continuous mode */
  iis2mdc_operating_mode_set(&iis2mdc_dev_ctx, IIS2MDC_CONTINUOUS_MODE);
}

/* Read samples in polling mode (no int) */
void iis2mdc_read_data_polling(void){
  /* Read output only if new value is available */
  iis2mdc_mag_data_ready_get(&iis2mdc_dev_ctx, &drdy);

  if (drdy) {
    /* Read magnetic field data */
    memset(data_raw_magnetic, 0x00, 3 * sizeof(int16_t));
    iis2mdc_magnetic_raw_get(&iis2mdc_dev_ctx, data_raw_magnetic);
    magnetic_mG[0] = iis2mdc_from_lsb_to_mgauss( data_raw_magnetic[0]);
    magnetic_mG[1] = iis2mdc_from_lsb_to_mgauss( data_raw_magnetic[1]);
    magnetic_mG[2] = iis2mdc_from_lsb_to_mgauss( data_raw_magnetic[2]);
    snprintf((char *)tx_buffer, sizeof(tx_buffer),
            "Magnetic field [mG]:%4.2f\t%4.2f\t%4.2f\r\n",
            magnetic_mG[0], magnetic_mG[1], magnetic_mG[2]);
    tx_com( tx_buffer, strlen( (char const *)tx_buffer ) );
    /* Read temperature data */
    memset( &data_raw_temperature, 0x00, sizeof(int16_t) );
    iis2mdc_temperature_raw_get( &iis2mdc_dev_ctx, &data_raw_temperature );
    temperature_degC = iis2mdc_from_lsb_to_celsius (
                         data_raw_temperature );
    snprintf((char *)tx_buffer, sizeof(tx_buffer), "Temperature [degC]:%6.2f\r\n",
            temperature_degC );
    tx_com( tx_buffer, strlen( (char const *)tx_buffer ) );
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

	return BSP_I2C2_WriteReg(IIS2MDC_I2C_ADD, reg, bufp, len);
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

	return BSP_I2C2_ReadReg(IIS2MDC_I2C_ADD, reg, bufp, len);
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
