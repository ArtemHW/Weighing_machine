/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE_BUFFER 50
#define ETALON 197
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
typedef struct
{
	QueueHandle_t queueh;
	QueueHandle_t queueh_clbrt;
	char tx[2];
	char rx;
	uint16_t adc_buf[SIZE_BUFFER];
	uint16_t adc_buf_result;
	uint16_t adc_calibration[SIZE_BUFFER];
	uint16_t adc_calibration_result;
	uint16_t adc_calibration_result1;
	uint16_t offset;
	uint16_t k;
}buffer_uart;
buffer_uart buffer;

SemaphoreHandle_t xSemaphore1;
//SemaphoreHandle_t xSemaphore2;
EventGroupHandle_t xCreatedEventGroup1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void calibration(void);
void weighing(void);
void sendUSART1weighing(void);
void receiveUSART1(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void sendUSART1int(void);
void sendUSART1char(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	buffer.offset = 0;
	buffer.k = 1;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&buffer.adc_buf, sizeof(buffer.adc_buf)/2);
  //HAL_DMA_Start(&hdma_adc1, &ADC1->DR, (uint32_t)&buffer.adc_buf, sizeof(buffer.adc_buf)/2);

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  xSemaphore1 = xSemaphoreCreateBinary();
  //xSemaphore2 = xSemaphoreCreateCounting(4, 0);
  xCreatedEventGroup1 = xEventGroupCreate();
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  QueueHandle_t myQueue1;
  myQueue1 = xQueueCreate(40, sizeof(char));
  buffer.queueh = myQueue1;

  QueueHandle_t myQueue2;
  myQueue2 = xQueueCreate(40, sizeof(char));
  buffer.queueh_clbrt = myQueue2;
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 80);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  xTaskCreate(calibration, "calibration", 180, NULL, 5, NULL);
  xTaskCreate(weighing, "weighing", 64, NULL, 2, NULL);
  xTaskCreate(sendUSART1weighing, "send data W", 128, NULL, 3, NULL);
  xTaskCreate(receiveUSART1, "receive data", 64, NULL, 2, NULL);
  xTaskCreate(sendUSART1int, "send data i", 160, NULL, 4, NULL);
  xTaskCreate(sendUSART1char, "send data c", 128, NULL, 4, NULL);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Adc1ClockSelection = RCC_ADC1PLLCLK_DIV1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_TSC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_TSC_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void calibration(void)
{
	for( ;; )
	{
		xSemaphoreTake(xSemaphore1, portMAX_DELAY);
		for(int i = 0; i < SIZE_BUFFER; i++)
		{
			arm_mean_q15((int16_t*)&buffer.adc_buf, sizeof(buffer.adc_buf)/2, (int16_t*)&buffer.adc_calibration[i]);
			vTaskDelay(50);
		}
		arm_mean_q15((int16_t*)&buffer.adc_calibration, sizeof(buffer.adc_calibration)/2, (int16_t*)&buffer.adc_calibration_result);
		char string_buff[] = "Calibration 0 value: = ";
		for(uint8_t i = 0; i<(sizeof(string_buff)); i++)
		{
			xQueueSend(buffer.queueh_clbrt, (void*)(&string_buff[i]), 5);
		}
		xEventGroupSetBits(xCreatedEventGroup1, 0x1);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x2, pdTRUE, pdTRUE, portMAX_DELAY);
		xQueueSend(buffer.queueh_clbrt, (void*)(((char*) &buffer.adc_calibration_result)), 1);
		xQueueSend(buffer.queueh_clbrt, (void*)(((char*) &buffer.adc_calibration_result)+1), 1);
		xEventGroupSetBits(xCreatedEventGroup1, 0x4);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x8, pdTRUE, pdTRUE, portMAX_DELAY);
		char string_buff2[] = "Place 197 grams etalon\r\n";
		xQueueReset(buffer.queueh_clbrt);
		for(uint8_t i = 0; i<(sizeof(string_buff2)); i++)
		{
			xQueueSend(buffer.queueh_clbrt, (void*)(&string_buff2[i]), 5);
		}
		xEventGroupSetBits(xCreatedEventGroup1, 0x1);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x2, pdTRUE, pdTRUE, portMAX_DELAY);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x10, pdTRUE, pdTRUE, portMAX_DELAY);
		for(int i = 0; i < SIZE_BUFFER; i++)
		{
			arm_mean_q15((int16_t*)&buffer.adc_buf, sizeof(buffer.adc_buf)/2, (int16_t*)&buffer.adc_calibration[i]);
			vTaskDelay(50);
		}
		arm_mean_q15((int16_t*)&buffer.adc_calibration, sizeof(buffer.adc_calibration)/2, (int16_t*)&buffer.adc_calibration_result1);
		char string_buff3[] = "Calibration 1 value: = ";
				for(uint8_t i = 0; i<(sizeof(string_buff3)); i++)
		{
			xQueueSend(buffer.queueh_clbrt, (void*)(&string_buff3[i]), 5);
		}
		xEventGroupSetBits(xCreatedEventGroup1, 0x1);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x2, pdTRUE, pdTRUE, portMAX_DELAY);
		xQueueSend(buffer.queueh_clbrt, (void*)(((char*) &buffer.adc_calibration_result1)), 1);
		xQueueSend(buffer.queueh_clbrt, (void*)(((char*) &buffer.adc_calibration_result1)+1), 1);
		xEventGroupSetBits(xCreatedEventGroup1, 0x4);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x8, pdTRUE, pdTRUE, portMAX_DELAY);
		buffer.offset = buffer.adc_calibration_result1 - buffer.adc_calibration_result;
		buffer.k = ETALON/(buffer.adc_calibration_result1 - buffer.adc_calibration_result);

		vTaskDelay(400);
		xSemaphoreTake(xSemaphore1, 1);
		xEventGroupWaitBits(xCreatedEventGroup1, 0x10, pdTRUE, pdTRUE, 1);
	}
	vTaskDelete(xTaskGetHandle("calibration"));
}

void weighing(void)
{
	for( ;; )
	{
		vTaskDelay(500);
		arm_mean_q15((int16_t*)&buffer.adc_buf, sizeof(buffer.adc_buf)/2, (int16_t*)&buffer.adc_buf_result);
		buffer.adc_buf_result = (buffer.adc_buf_result - buffer.offset)*buffer.k;
		xQueueSend(buffer.queueh, (void*)(((char*) &buffer.adc_buf_result)), 1);
		xQueueSend(buffer.queueh, (void*)(((char*) &buffer.adc_buf_result)+1), 1);
		xTaskNotify(xTaskGetHandle("send data W"), 0, eNoAction);
		taskYIELD();
	}
}

void sendUSART1weighing(void)
{
	uint16_t res_to_uart;
	char string_buff[8];
	for( ;; )
	{
		xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);
		xQueueReceive(buffer.queueh, &buffer.tx[0], portMAX_DELAY);
		xQueueReceive(buffer.queueh, &buffer.tx[1], portMAX_DELAY);
		res_to_uart = buffer.tx[0];
		res_to_uart = res_to_uart + (buffer.tx[1] << 8);
		sprintf(string_buff, "%d\r\n", res_to_uart);
		HAL_UART_Transmit(&huart1, (uint8_t*) string_buff, sizeof(string_buff), 100);
	}
	 vTaskDelete(xTaskGetHandle("send data W"));
}

void sendUSART1int(void)
{
	for( ;; )
	{
		uint16_t res_to_uart;
		char string_buff[30] = {0};
		xEventGroupWaitBits(xCreatedEventGroup1, 0x4, pdTRUE, pdTRUE, portMAX_DELAY);
		xQueueReceive(buffer.queueh_clbrt, &buffer.tx[0], portMAX_DELAY);
		xQueueReceive(buffer.queueh_clbrt, &buffer.tx[1], portMAX_DELAY);
		res_to_uart = buffer.tx[0];
		res_to_uart = res_to_uart + (buffer.tx[1] << 8);
		sprintf(string_buff, "%d\r\n", res_to_uart);
		HAL_UART_Transmit(&huart1, (uint8_t*) string_buff, sizeof(string_buff), 100);
		xEventGroupSetBits(xCreatedEventGroup1, 0x8);
	}
	 vTaskDelete(xTaskGetHandle("send data i"));
}
void sendUSART1char(void)
{
	for( ;; )
	{
		char string_buff[30] = {0};
		BaseType_t res;
		uint8_t i = 0;
		xEventGroupWaitBits(xCreatedEventGroup1, 0x1, pdTRUE, pdTRUE, portMAX_DELAY);
		do{
			res = xQueueReceive(buffer.queueh_clbrt, &string_buff[i], portMAX_DELAY);
			i++;
		}while(res != errQUEUE_EMPTY && string_buff[i-1] != '\0');

		HAL_UART_Transmit(&huart1, (uint8_t*) string_buff, i+1, 100);
		xEventGroupSetBits(xCreatedEventGroup1, 0x2);
	}
	 vTaskDelete(xTaskGetHandle("send data c"));
}

void receiveUSART1(void)
{
	for( ;; )
	{
		vTaskDelete(xTaskGetHandle("receive data"));
	}
	vTaskDelete(xTaskGetHandle("receive data"));
}

HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_2)
	{
		xSemaphoreGiveFromISR(xSemaphore1, NULL);
	}
	if(GPIO_Pin == GPIO_PIN_13)
	{
		xEventGroupSetBitsFromISR(xCreatedEventGroup1, 0x10, pdFALSE);
	}
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {

  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
