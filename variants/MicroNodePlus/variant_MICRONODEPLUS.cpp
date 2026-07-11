/*
 *******************************************************************************
 * Copyright (c) 2022, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#if defined(ARDUINO_NUCLEO_H723ZG)
#include "pins_arduino.h"
#include "Arduino.h"

// Digital PinName array — STM32H723VGHx, TFBGA100 (V package): ports A–E,
// PH0/PH1 and the PC2_C/PC3_C dual pads. Ports F/G are not bonded out.
const PinName digitalPin[] = {
  PA_0,   // D0/A0
  PA_1,   // D1/A1
  PA_2,   // D2/A2
  PA_3,   // D3/A3
  PA_4,   // D4/A4
  PA_5,   // D5/A5
  PA_6,   // D6/A6
  PA_7,   // D7/A7
  PA_8,   // D8
  PA_9,   // D9
  PA_10,  // D10
  PA_11,  // D11
  PA_12,  // D12
  PA_13,  // D13
  PA_14,  // D14
  PA_15,  // D15
  PB_0,   // D16/A8
  PB_1,   // D17/A9
  PB_2,   // D18
  PB_3,   // D19
  PB_4,   // D20
  PB_5,   // D21
  PB_6,   // D22
  PB_7,   // D23
  PB_8,   // D24
  PB_9,   // D25
  PB_10,  // D26
  PB_11,  // D27
  PB_12,  // D28
  PB_13,  // D29
  PB_14,  // D30
  PB_15,  // D31
  PC_0,   // D32/A10
  PC_1,   // D33/A11
  PC_4,   // D34/A12
  PC_5,   // D35/A13
  PC_6,   // D36
  PC_7,   // D37
  PC_8,   // D38
  PC_9,   // D39
  PC_10,  // D40
  PC_11,  // D41
  PC_12,  // D42
  PC_13,  // D43
  PC_14,  // D44
  PC_15,  // D45
  PD_0,   // D46
  PD_1,   // D47
  PD_2,   // D48
  PD_3,   // D49
  PD_4,   // D50
  PD_5,   // D51
  PD_6,   // D52
  PD_7,   // D53
  PD_8,   // D54
  PD_9,   // D55
  PD_10,  // D56
  PD_11,  // D57
  PD_12,  // D58
  PD_13,  // D59
  PD_14,  // D60
  PD_15,  // D61
  PE_0,   // D62
  PE_1,   // D63
  PE_2,   // D64
  PE_3,   // D65
  PE_4,   // D66
  PE_5,   // D67
  PE_6,   // D68
  PE_7,   // D69
  PE_8,   // D70
  PE_9,   // D71
  PE_10,  // D72
  PE_11,  // D73
  PE_12,  // D74
  PE_13,  // D75
  PE_14,  // D76
  PE_15,  // D77
  PH_0,   // D78
  PH_1,   // D79
  PC_2_C, // D80/A14
  PC_3_C  // D81/A15
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
  17, // A9,  PB1
  32, // A10, PC0
  33, // A11, PC1
  34, // A12, PC4
  35, // A13, PC5
  80, // A14, PC2_C
  81  // A15, PC3_C
};

// --- SERIAL PORT INSTANTIATION ---
// Board UART connectors, named by connector label (not USART instance):
//   Serial1 → USART2 on PD5(TX)/PD6(RX)
//   Serial2 → USART3 on PC10(TX)/PC11(RX) — ALT1 pins; plain PC10/PC11 map
//   to UART4, the debug console, and would hijack it.
// See the UART section of variant_MICRONODEPLUS.h for the wiring rationale.

#if defined(PIN_SERIAL1_RX) && defined(PIN_SERIAL1_TX)
HardwareSerial Serial1(PIN_SERIAL1_RX, PIN_SERIAL1_TX); // USART2
#endif

#if defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX)
HardwareSerial Serial2(PIN_SERIAL2_RX, PIN_SERIAL2_TX); // USART3
#endif

// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Debug console TX/RX swap. The J7 debug header wires the node's TX to PB8
// and RX to PB9 (schematic nets /DEBUG_TX and /DEBUG_RX, node-perspective),
// but the H723 pin mux only offers UART4_TX on PB9 / UART4_RX on PB8 (AF8).
// Request the USART SWAP advanced feature so HAL_UART_Init() crosses TX/RX
// inside the peripheral. This overrides the HAL's weak no-op; the Arduino
// core does not define its own. Caveat: the HAL only calls MspInit on a
// handle in RESET state — the first Serial.begin(), or any begin() after
// end(). A repeated begin() without end() skips it and would drop the swap.
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART4) {
    huart->AdvancedInit.AdvFeatureInit |= UART_ADVFEATURE_SWAP_INIT;
    huart->AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
  }
}

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
// NOT WEAK: generic_clock.c in this variant dir also compiles (the board JSON
// defines ARDUINO_GENERIC_H723VGHX alongside ARDUINO_NUCLEO_H723ZG) and its
// WEAK HSI-based SystemClock_Config wins archive link order over a WEAK one
// here. This config must be the one that runs: it clocks the PLLs from the
// external HSE (crystal-grade), which CAN-FD's data-phase oscillator-tolerance
// budget (~0.5% at 4 Mbps) requires — the HSI's ~±1% trim is not good enough.
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
  // Production MicroNodePlus has a real 8 MHz crystal on PH0/PH1, so the HSE
  // amplifier must be enabled (HSE_ON); bypass mode leaves the crystal undriven
  // and HSERDY never sets. Nucleo-H723ZG dev boards inject a square wave from
  // the ST-LINK MCO instead, which needs BYPASS — retried below as a fallback.
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    // No crystal came up — assume an externally driven clock (Nucleo dev board).
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
      Error_Handler();
    }
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_USB
                                             | RCC_PERIPHCLK_FDCAN;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 24;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 4;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  // FDCAN kernel clock must divide evenly into the CAN-FD bit rates. PLL1Q is
  // 137.5 MHz (550 MHz VCO / 4), which cannot produce an exact 4 Mbps data
  // phase (137.5/4 = 34.375 time quanta). PLL2Q = 192 MHz VCO / 2 = 96 MHz
  // divides exactly for both 1 Mbps arbitration and 4 Mbps data.
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /*Configure the clock recovery system (CRS)**********************************/

  /*Enable CRS Clock*/
  __HAL_RCC_CRS_CLK_ENABLE();

  /* Default Synchro Signal division factor (not divided) */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;

  /* Set the SYNCSRC[1:0] bits according to CRS_Source value */
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB1;

  /* HSI48 is synchronized with USB SOF at 1KHz rate */
  RCC_CRSInitStruct.ReloadValue =  RCC_CRS_RELOADVALUE_DEFAULT;
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;

  /* Set the TRIM[5:0] to the default value */
  RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;

  /* Start automatic synchronization */
  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

#ifdef __cplusplus
}
#endif
#endif /* ARDUINO_NUCLEO_H723ZG */
