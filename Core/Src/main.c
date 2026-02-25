/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "GY88128_648_GREEN_SEGMENT_LCD_DRIVER.h"
#include "GY88128_80_UPS_SEGMENT_LCD_DRIVER.h"
#include <stdbool.h>
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STM_LED_Pin GPIO_PIN_13
#define STM_LED_GPIO_Port GPIOC
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  // ========================================================================
  // GY88128-648 Green LCD - Segment Mapping Discovery
  // Pins: PC7=DATA, PA10=WR, PA11=CS
  // ========================================================================
  GY88128_648_Init(); // Initialize the GY88128-648 Green LCD

  // NEW: Full Startup Test Sequence
  GY88128_648_FullTestSequence();

  // STEP 1: Turn on all segments to verify LCD is working
  //  GY88128_648_TurnOnAllSegments();
  //  HAL_Delay(2000);
  //  GY88128_648_Clear();
  //  HAL_Delay(500);

  // STEP 2: Scan by address - each address (0-31) lights up for 1 second
  // Watch and note which segments light up at each address
  // This helps identify where each digit and icon is mapped
  //  GY88128_648_ScanByAddress(1000);

  // STEP 3: After discovering address ranges, run detailed scan on specific
  // range Uncomment and adjust range after identifying digit addresses:
  // GY88128_648_TestAddressRange(0, 15, 500);  // Scan addresses 0-15 bit by
  // bit

  // STEP 4: Run the Standard Dashboard (Verified Mapping)
  SystemStatus_t test_stat = {
      .grid_volt = 228.5f,
      .batt_volt = 13.8f,
      .inv_out_volt = 230.1f,
      .inv_load_percent = 45,
      .is_grid_present = true,
      .is_solar_present = true,
      .is_charging = true,
      .output_active = true,
      .batt_type = 2 // AGM
  };

  uint8_t screen_mode = 0; // 0-3 Hybrid Scenarios
  uint8_t last_btn_state = GPIO_PIN_SET;
  uint32_t last_refresh_tick = 0;
  bool force_refresh = true;

  while (1) {
    // 1. Button Logic (PC6) - Cycle 4 Hybrid scenarios
    uint8_t btn_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
    if (last_btn_state == GPIO_PIN_SET && btn_state == GPIO_PIN_RESET) {
      HAL_Delay(20);
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET) {
        screen_mode = (screen_mode + 1) % 4; // 4 Scenarios
        force_refresh = true;
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET)
          ;
        HAL_Delay(20);
      }
    }
    last_btn_state = btn_state;

    // 2. Refresh Logic
    uint32_t current_tick = HAL_GetTick();
    if (force_refresh || (current_tick - last_refresh_tick >= 1000)) {
      last_refresh_tick = current_tick;
      force_refresh = false;

      // Toggle Physical LED (PC13)
      HAL_GPIO_TogglePin(STM_LED_GPIO_Port, STM_LED_Pin);

      // Simulate Hybrid Data
      test_stat.is_solar_present = true; // Most hybrid scenarios have solar
      test_stat.is_grid_present = true;  // Most hybrid scenarios have grid

      switch (screen_mode) {
      case 0: // S1: Dual Charging
        test_stat.is_charging = true;
        test_stat.output_active = false;
        test_stat.solar_volt = 280.0f;
        test_stat.grid_volt = 223.5f;
        test_stat.batt_volt = 45.8f;
        test_stat.chg_amp = 58.5f;
        break;
      case 1: // S2: Solar Priority (Inverter ON)
        test_stat.is_charging = true;
        test_stat.output_active = true;
        test_stat.is_grid_present = true; // Grid standby
        test_stat.solar_volt = 160.0f;
        test_stat.solar_amp = 11.25f;      // For 1.80 kVA
        test_stat.mppt_out_amp = 36.0f;    // As requested
        test_stat.mppt_out_watt = 1710.0f; // For 1.71 kVA
        test_stat.inv_out_volt = 230.0f;
        test_stat.inv_load_watt = 1500;
        test_stat.batt_volt = 28.1f;
        test_stat.chg_amp = 15.0f;
        test_stat.inv_load_percent = 75;
        break;
      case 2: // S3: Grid Priority (Bypass)
        test_stat.is_charging = true;
        test_stat.output_active = true;
        test_stat.solar_volt = 280.0f;
        test_stat.grid_volt = 228.0f;
        test_stat.inv_load_watt = 850;
        test_stat.chg_amp = 15.0f;
        break;
      case 3: // S4: PV Only Charge
        test_stat.is_grid_present = false;
        test_stat.output_active = false;
        test_stat.is_charging = true;
        test_stat.solar_volt = 280.0f;
        test_stat.batt_volt = 45.8f;
        test_stat.chg_amp = 58.5f;
        break;
      }

      Screen_Hybrid_Mode(&test_stat, screen_mode + 1);
    }
    HAL_Delay(20);
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA,
                    HT1621_CS_PIN_Pin | HT1621_WR_PIN_Pin | HT1621_DATA_PIN_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pins : HT1621_CS_PIN_Pin HT1621_WR_PIN_Pin
   * HT1621_DATA_PIN_Pin */
  GPIO_InitStruct.Pin =
      HT1621_CS_PIN_Pin | HT1621_WR_PIN_Pin | HT1621_DATA_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 (Manual Scan / Screen Cycle) */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
