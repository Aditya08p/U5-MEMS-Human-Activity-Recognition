/**
  ******************************************************************************
  * @file    app_lps22hh.h
  * @author  System Research and Applications Team
  * @brief   Header for app_lps22hh.c module.
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

#ifndef APP_LPS22HH_H
#define APP_LPS22HH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lps22hh_reg.h"
#include "string.h"
#include "stdio.h"
/**
  * @brief  Initializes LPS22HH Sensor
  * @retval none
  */
void lps22hh_Init(void);

/**
  * @brief  Example function reading data from LPS22HH in polling mode.
  * @retval none
  */
void lps22hh_read_data_polling(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_LPS22HH_H */
