/*
 *******************************************************************************
 * Copyright (c) 2020-2026, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#if defined(ARDUINO_MICRONODEC5)
#include "pins_arduino.h"

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  *
  * WEAK fallback only -- variant_MICRONODEC5.cpp provides the real,
  * non-weak SystemClock_Config() for this board and takes priority.
  */
WEAK void SystemClock_Config(void)
{
#warning "SystemClock_Config() is empty. Default clock at reset is used."
}

#endif /* ARDUINO_MICRONODEC5 */
