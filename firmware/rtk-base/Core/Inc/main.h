/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern UART_HandleTypeDef huart3;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ESP32_GP1_Pin GPIO_PIN_2
#define ESP32_GP1_GPIO_Port GPIOE
#define ESP32_GP2_Pin GPIO_PIN_3
#define ESP32_GP2_GPIO_Port GPIOE
#define ESP32_GP3_Pin GPIO_PIN_4
#define ESP32_GP3_GPIO_Port GPIOE
#define ESP32_GP4_Pin GPIO_PIN_5
#define ESP32_GP4_GPIO_Port GPIOE
#define ESP32_GP5_Pin GPIO_PIN_6
#define ESP32_GP5_GPIO_Port GPIOE
#define SDMMC_CD_Pin GPIO_PIN_13
#define SDMMC_CD_GPIO_Port GPIOC
#define USB3300_RST_Pin GPIO_PIN_1
#define USB3300_RST_GPIO_Port GPIOC
#define VBATT_IN_Pin GPIO_PIN_1
#define VBATT_IN_GPIO_Port GPIOA
#define F9P_GEOFENCH_STAT_Pin GPIO_PIN_6
#define F9P_GEOFENCH_STAT_GPIO_Port GPIOA
#define F9P_RTK_STAT_Pin GPIO_PIN_7
#define F9P_RTK_STAT_GPIO_Port GPIOA
#define F9P_ANT_DETECT_Pin GPIO_PIN_4
#define F9P_ANT_DETECT_GPIO_Port GPIOC
#define F9P_ANT_OFF_Pin GPIO_PIN_5
#define F9P_ANT_OFF_GPIO_Port GPIOC
#define F9P_ANT_SHORT_N_Pin GPIO_PIN_2
#define F9P_ANT_SHORT_N_GPIO_Port GPIOB
#define MAGNETO_SPARE1_Pin GPIO_PIN_7
#define MAGNETO_SPARE1_GPIO_Port GPIOE
#define MAGNETO_SPARE2_Pin GPIO_PIN_8
#define MAGNETO_SPARE2_GPIO_Port GPIOE
#define F9P_TIMEP_Pin GPIO_PIN_10
#define F9P_TIMEP_GPIO_Port GPIOE
#define F9P_EXTINT_Pin GPIO_PIN_11
#define F9P_EXTINT_GPIO_Port GPIOE
#define F9P_SAFEBOOT_N_Pin GPIO_PIN_12
#define F9P_SAFEBOOT_N_GPIO_Port GPIOE
#define F9P_RESET_N_Pin GPIO_PIN_13
#define F9P_RESET_N_GPIO_Port GPIOE
#define F9P_D_SEL_Pin GPIO_PIN_14
#define F9P_D_SEL_GPIO_Port GPIOE
#define F9P_TX_READY_Pin GPIO_PIN_15
#define F9P_TX_READY_GPIO_Port GPIOE
#define SX1262_SPI_MISO_Pin GPIO_PIN_14
#define SX1262_SPI_MISO_GPIO_Port GPIOB
#define SX1262_SPI_MOSI_Pin GPIO_PIN_15
#define SX1262_SPI_MOSI_GPIO_Port GPIOB
#define DEBUG_TX_Pin GPIO_PIN_8
#define DEBUG_TX_GPIO_Port GPIOD
#define DEBUG_RX_Pin GPIO_PIN_9
#define DEBUG_RX_GPIO_Port GPIOD
#define nPGOOD_Pin GPIO_PIN_10
#define nPGOOD_GPIO_Port GPIOD
#define nCHG_Pin GPIO_PIN_11
#define nCHG_GPIO_Port GPIOD
#define LSM6DSOX_INT1_Pin GPIO_PIN_14
#define LSM6DSOX_INT1_GPIO_Port GPIOD
#define LSM6DSOX_INT2_Pin GPIO_PIN_15
#define LSM6DSOX_INT2_GPIO_Port GPIOD
#define SX1262_RXEN_Pin GPIO_PIN_6
#define SX1262_RXEN_GPIO_Port GPIOC
#define SX1262T_XEN_Pin GPIO_PIN_7
#define SX1262T_XEN_GPIO_Port GPIOC
#define UART7_RX_F9P_TXD2_Pin GPIO_PIN_8
#define UART7_RX_F9P_TXD2_GPIO_Port GPIOA
#define SX1262_SPI_CLK_Pin GPIO_PIN_9
#define SX1262_SPI_CLK_GPIO_Port GPIOA
#define SX1262_SPI2_CS_Pin GPIO_PIN_10
#define SX1262_SPI2_CS_GPIO_Port GPIOA
#define SX1262_SPI2_RST_Pin GPIO_PIN_11
#define SX1262_SPI2_RST_GPIO_Port GPIOA
#define SX1262_SPI2_BUSY_Pin GPIO_PIN_12
#define SX1262_SPI2_BUSY_GPIO_Port GPIOA
#define UART7T_X_F9P_RXD2_Pin GPIO_PIN_15
#define UART7T_X_F9P_RXD2_GPIO_Port GPIOA
#define BQ24975_nCE_Pin GPIO_PIN_0
#define BQ24975_nCE_GPIO_Port GPIOD
#define BQ24975_EN1_Pin GPIO_PIN_3
#define BQ24975_EN1_GPIO_Port GPIOD
#define BQ24975_EN2_Pin GPIO_PIN_4
#define BQ24975_EN2_GPIO_Port GPIOD
#define PD5_LD3_Pin GPIO_PIN_5
#define PD5_LD3_GPIO_Port GPIOD
#define PD6_LD4_Pin GPIO_PIN_6
#define PD6_LD4_GPIO_Port GPIOD
#define SYSOFF_Pin GPIO_PIN_7
#define SYSOFF_GPIO_Port GPIOD
#define SX1262_SPI2_DIO2_Pin GPIO_PIN_6
#define SX1262_SPI2_DIO2_GPIO_Port GPIOB
#define SX1262_SPI2_DIO1_Pin GPIO_PIN_7
#define SX1262_SPI2_DIO1_GPIO_Port GPIOB
#define F9P_RX_Pin GPIO_PIN_8
#define F9P_RX_GPIO_Port GPIOB
#define F9P_TX_Pin GPIO_PIN_9
#define F9P_TX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
