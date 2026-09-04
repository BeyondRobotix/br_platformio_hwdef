/*
 *******************************************************************************
 * Copyright (c) 2020-2026, STMicroelectronics / Beyond Robotix
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 * opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 *
 * NOTE (Beyond Robotix): SystemClock_Config() below targets STM32C5xx's new
 * "HAL2" RCC API (hal_status_t / HAL_RCC_* handle-less calls), which is a
 * different generation from the classic RCC_OscInitTypeDef/HAL_RCC_OscConfig
 * API used on this platform's other boards (MicroNode/CoreNode/MicroNodePlus).
 * It has not been build- or hardware-verified. HSIS is STM32C5's internal
 * 144 MHz factory-trimmed oscillator and is the reset-default SYSCLK source,
 * so this just confirms it and sets flash wait states -- no external
 * crystal (HSE) is assumed since no schematic exists for this board yet.
 */
#include "pins_arduino.h"
#include "Arduino.h"

// Digital PinName array
const PinName digitalPin[] = {
    PA_0,  // D0/A0
    PA_1,  // D1/A1
    PA_2,  // D2/A2
    PA_3,  // D3/A3
    PA_4,  // D4/A4
    PA_5,  // D5/A5
    PA_6,  // D6/A6
    PA_7,  // D7/A7
    PA_8,  // D8
    PA_9,  // D9
    PA_10, // D10
    PA_11, // D11 (FDCAN1_RX)
    PA_12, // D12 (FDCAN1_TX)
    PA_13, // D13 (SWDIO)
    PA_14, // D14 (SWCLK)
    PA_15, // D15
    PB_0,  // D16/A8
    PB_1,  // D17/A9
    PB_2,  // D18
    PB_3,  // D19
    PB_4,  // D20
    PB_5,  // D21
    PB_6,  // D22
    PB_7,  // D23
    PB_8,  // D24
    PB_9,  // D25
    PB_10, // D26
    PB_12, // D27
    PB_13, // D28
    PB_14, // D29
    PB_15, // D30
    PC_13, // D31
    PC_14, // D32
    PC_15, // D33
    PE_2,  // D34
    PH_0,  // D35
    PH_1,  // D36
    PH_2   // D37
};

// Analog (Ax) pin number array
const uint32_t analogInputPin[] = {
    0,  // A0,  PA0
    1,  // A1,  PA1
    2,  // A2,  PA2
    3,  // A3,  PA3
    4,  // A4,  PA4
    5,  // A5,  PA5
    6,  // A6,  PA6
    7,  // A7,  PA7
    16, // A8,  PB0
    17  // A9,  PB1
};

#ifdef __cplusplus
extern "C"
{
#endif
    WEAK void SystemClock_Config(void)
    {
        // Make sure HSIS (144 MHz internal RC) is enabled and ready -- it is the
        // factory reset-default SYSCLK source, so this is normally a no-op.
        if (HAL_RCC_HSIS_IsReady() != HAL_RCC_OSC_READY)
        {
            if (HAL_RCC_HSIS_Enable() != HAL_OK)
            {
                Error_Handler();
            }
        }

        // 144 MHz needs 4 wait states on this flash controller (matches the
        // STM32CubeC5 reference example for the max-frequency HSIS case) --
        // verify against the STM32C5 reference manual's latency table.
        if (HAL_FLASH_ITF_SetLatency(HAL_FLASH, HAL_FLASH_ITF_LATENCY_4) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_RCC_SetSYSCLKSource(HAL_RCC_SYSCLK_SRC_HSIS) != HAL_OK)
        {
            Error_Handler();
        }

        hal_rcc_bus_clk_config_t bus_config = {};
        bus_config.hclk_prescaler = HAL_RCC_HCLK_PRESCALER1;
        bus_config.pclk1_prescaler = HAL_RCC_PCLK_PRESCALER1;
        bus_config.pclk2_prescaler = HAL_RCC_PCLK_PRESCALER1;
        bus_config.pclk3_prescaler = HAL_RCC_PCLK_PRESCALER1;
        if (HAL_RCC_SetBusClockConfig(&bus_config) != HAL_OK)
        {
            Error_Handler();
        }

        SystemCoreClock = HAL_RCC_GetSYSCLKFreq();

        // Reinitialize SysTick to enable the delay() function
        if (SysTick_Config(SystemCoreClock / 1000))
        {
            Error_Handler();
        }
        NVIC_EnableIRQ(SysTick_IRQn);
    }

#ifdef __cplusplus
}
#endif
