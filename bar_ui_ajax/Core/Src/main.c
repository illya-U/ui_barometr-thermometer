/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "baro.h"
#include <malloc.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define countof(_a) (sizeof(_a)/sizeof(_a[0]))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* Definitions for task_take_raw_s */
osThreadId_t task_take_raw_sHandle;
const osThreadAttr_t task_take_raw_s_attributes = {
  .name = "task_take_raw_s",
  .stack_size = 1000 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for task_showing */
osThreadId_t task_showingHandle;
const osThreadAttr_t task_showing_attributes = {
  .name = "task_showing",
  .stack_size = 1000 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for mutex */
osMutexId_t mutexHandle;
const osMutexAttr_t mutex_attributes = {
  .name = "mutex"
};
/* Definitions for first_iter */
osSemaphoreId_t first_iterHandle;
const osSemaphoreAttr_t first_iter_attributes = {
  .name = "first_iter"
};
/* Definitions for MEANING_SEM */
osSemaphoreId_t MEANING_SEMHandle;
const osSemaphoreAttr_t MEANING_SEM_attributes = {
  .name = "MEANING_SEM"
};
/* USER CODE BEGIN PV */

struct list
{
	struct list* prev;
	struct list* next;
	int32_t temp;
	int32_t pres;
};

struct list* begin = NULL;
struct list* end = NULL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void start_task_take_raw_s(void *argument);
void start_task_showing(void *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char text[100];

struct list* create_new_elem()
{
	struct list* new_elem = (struct list*)malloc(sizeof(struct list));
	if(begin == NULL)
	{
		begin = new_elem;
		end = new_elem;
		new_elem->prev = NULL;
		new_elem->next = NULL;
		return new_elem;
	}
	begin->next = new_elem;
	new_elem->prev = begin;
	new_elem->next = NULL;
	begin = new_elem;
	return begin;
}

struct list* delete_elem()
{
	if(begin == end)
	{
		free(end);
		begin = NULL;
		end = NULL;
		return NULL;
	}
	struct list* new_end = end->next;
	new_end->prev = NULL;
	free(end);
	end = new_end;
	return end;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of mutex */
  mutexHandle = osMutexNew(&mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of first_iter */
  first_iterHandle = osSemaphoreNew(1, 1, &first_iter_attributes);

  /* creation of MEANING_SEM */
  MEANING_SEMHandle = osSemaphoreNew(5, 5, &MEANING_SEM_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of task_take_raw_s */
  task_take_raw_sHandle = osThreadNew(start_task_take_raw_s, NULL, &task_take_raw_s_attributes);

  /* creation of task_showing */
  task_showingHandle = osThreadNew(start_task_showing, NULL, &task_showing_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//  //osSemaphoreRelease(semButtonPressedHandle);
//}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_start_task_take_raw_s */
/**
  * @brief  Function implementing the task_take_raw_s thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_start_task_take_raw_s */
void start_task_take_raw_s(void *argument)
{
  /* USER CODE BEGIN 5 */
	if(baro_init() != BARO_OK)
	{
		snprintf(text,countof(text),"Error initialization barometr\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)text, strnlen(text,countof(text)),1000);
		while(1){}
	}
	osSemaphoreAcquire(first_iterHandle,osWaitForever);
   /* Infinite loop */
   for(;;)
   {
	   osMutexAcquire(mutexHandle,osWaitForever);
	   osStatus_t val = osSemaphoreAcquire(MEANING_SEMHandle,10U);
	   if(val != osOK)
	   {
		   osMutexRelease(mutexHandle);
		   osDelay(pdMS_TO_TICKS(150));
		   continue;
	   }
	   struct list* lst = create_new_elem();
	   lst->temp = baro_read_temp();
	   lst->pres = baro_read_press();
	   osMutexRelease(mutexHandle);
   }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_start_task_showing */
/**
* @brief Function implementing the task_showing thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_start_task_showing */
void start_task_showing(void *argument)
{
  /* USER CODE BEGIN start_task_showing */
  /* Infinite loop */
  for(;;)
  {
	 if(osSemaphoreRelease(first_iterHandle) == osErrorResource)
	 {
		osDelay(pdMS_TO_TICKS(150));
		continue;
	 }
	 osSemaphoreAcquire(first_iterHandle,osWaitForever);
	 osMutexAcquire(mutexHandle,osWaitForever);
	 if(osSemaphoreRelease(MEANING_SEMHandle) != osErrorResource)
	 	 {
		 	 snprintf(text,countof(text),"/*%ld.%02ld,%ld.%02ld*/\n",end->temp/100,end->temp%100,end->pres/100,end->pres%100);
		 	 HAL_UART_Transmit(&huart1, (uint8_t*)text, strnlen(text,countof(text)),1000);
		 	 delete_elem();
		 	 osMutexRelease(mutexHandle);
	 	 }
		 else
		 {
			 osMutexRelease(mutexHandle);
			 osDelay(pdMS_TO_TICKS(150));
		 }
  }
  /* USER CODE END start_task_showing */
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
