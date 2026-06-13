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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart2;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SX1262_SPI_MOSI_Pin GPIO_PIN_1
#define SX1262_SPI_MOSI_GPIO_Port GPIOC
#define SX1262_SPI_MISO_Pin GPIO_PIN_2
#define SX1262_SPI_MISO_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define SX1262_SPI_SCLK_Pin GPIO_PIN_10
#define SX1262_SPI_SCLK_GPIO_Port GPIOB
#define SX1262_SPI_CS_Pin GPIO_PIN_12
#define SX1262_SPI_CS_GPIO_Port GPIOB
#define SX1262_SPI_RESET_Pin GPIO_PIN_13
#define SX1262_SPI_RESET_GPIO_Port GPIOB
#define SX1262_SPI_BUSY_Pin GPIO_PIN_14
#define SX1262_SPI_BUSY_GPIO_Port GPIOB
#define SX1262_SPI_TxENABLE_Pin GPIO_PIN_6
#define SX1262_SPI_TxENABLE_GPIO_Port GPIOC
#define SX1262_SPI_RxENABLE_Pin GPIO_PIN_7
#define SX1262_SPI_RxENABLE_GPIO_Port GPIOC
#define SX1262_SPI_DIO2_Pin GPIO_PIN_8
#define SX1262_SPI_DIO2_GPIO_Port GPIOC
#define SX1262_SPI_DIO1_Pin GPIO_PIN_9
#define SX1262_SPI_DIO1_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
