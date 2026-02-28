/**
  ******************************************************************************
  * @file    app_iis2mdc.h
  * @author  System Research and Applications Team
  * @brief   Header for app_iis2mdc.c module.
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

#ifndef APP_IIS2MDC_H
#define APP_IIS2MDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iis2mdc_reg.h"
#include "string.h"
#include "stdio.h"
/**
  * @brief  Initializes IIS2MDC Sensor
  * @retval none
  */
void iis2mdc_Init(void);

/**
  * @brief  Example function reading data from IIS2MDC in polling mode.
  * @retval none
  */
void iis2mdc_read_data_polling(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_IIS2MDC_H */
