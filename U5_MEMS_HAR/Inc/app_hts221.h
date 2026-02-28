/**
  ******************************************************************************
  * @file    app_hts221.h
  * @author  System Research and Applications Team
  * @brief   Header for app_hts221.c module.
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

#ifndef APP_HTS221_H
#define APP_HTS221_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hts221_reg.h"
#include "string.h"
#include "stdio.h"
/**
  * @brief  Initializes HTS221 Sensor
  * @retval none
  */
void hts221_Init(void);

/**
  * @brief  Example function reading data from HTS221 in polling mode.
  * @retval none
  */
void hts221_read_data_polling(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_HTS221_H */
