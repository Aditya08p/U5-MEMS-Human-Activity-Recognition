/**
  ******************************************************************************
  * @file    app_ism330dhcx.h
  * @author  System Research and Applications Team
  * @brief   Header for app_ism330dhcx.c module.
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

#ifndef APP_ISM330DHCX_H
#define APP_ISM330DHCX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ism330dhcx_reg.h"
#include "string.h"
#include "stdio.h"
/**
  * @brief  Initializes ISM330DHCX Sensor
  * @retval none
  */
void ism330dhcx_Init(void);

/**
  * @brief  Example function reading data from ISM330DHCX in polling mode.
  * @retval none
  */
void ism330dhcx_read_data_polling(void);

/**
  * @brief  Example function reading data from ISM330DHCX in data ready interrupt mode.
  * @retval none
  */
void ism330dhcx_read_data_drdy(void);


#ifdef __cplusplus
}
#endif

#endif /* APP_ISM330DHCX_H */
