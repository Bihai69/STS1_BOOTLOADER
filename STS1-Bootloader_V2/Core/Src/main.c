/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main programm body for sts1 bootloader 
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
#define MAJOR 2
#define MINOR 0

const uint8_t BL_VERSION[2]={MAJOR,MINOR};

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi3;
TIM_HandleTypeDef htim3;
FLASH_EraseInitTypeDef FlashErase;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);  //system generated for clock_config
static void MX_GPIO_Init(void); //system generated for GPIO config
static void MX_SPI3_Init(void); //system generated for SPI config
static void MX_TIM3_Init(void); //system generated for timer3 config

static void jump_to_app(void); //jump to main app
static void flash_write_copy(void); //copy data from backup pool to live pool
static void flash_write_dummy(void); //testing
static uint32_t flash_erase_app_sector(void); //standard backup sector is 6

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{ 

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI3_Init();
  MX_TIM3_Init();

  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET); //LED ON
  HAL_Delay(2500); //DELAY
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET); //LED OFF

  flash_write_dummy(); //write test data to sector 6 for debugging
  
  HAL_TIM_Base_Start(&htim3);

  flash_erase_app_sector(); //erase the backup sector
  flash_write_copy(); //write a copy from backup sector to app sector
  
  uint16_t timer_value = __HAL_TIM_GET_COUNTER(&htim3); //get counter value in us
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET); //LED ON
  
  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,0x8010000,timer_value); //Write Word at address
  HAL_FLASH_Lock();
  HAL_TIM_Base_Stop(&htim3); //stop timer

  //jump_to_app(); //jumpt ot application sector

  while (1)
  {
   
    //if something goes wrong
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET); //LED ON
    HAL_Delay(500); //DELAY
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET); //LED OFF
    HAL_Delay(500); //DELAY
   
  }
}

/*------------User written functions---------------*/
static void flash_write_copy(void)
{
  volatile uint32_t app_flash_memory_address = 0x08020000; //start adress fpr programm sector of flash
  volatile uint32_t backup_flash_memory_address = 0x08040000; //start adress fpr backup programm sector of flash
  uint32_t current_read_word = 0xCAFEBABE; //current read word from backup sector


  HAL_FLASH_Unlock(); //unlock the flash
  for(uint32_t iterator = 0; iterator < 0x8000; iterator++) //4 bytes pro wort werden gelesen 0x4000 = 0x10000 im flash (128K/4=32K=0x8000)
  {
    current_read_word = *((volatile uint32_t*)backup_flash_memory_address);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,app_flash_memory_address,current_read_word); //Write Word at address
    app_flash_memory_address += 4;
    backup_flash_memory_address += 4;
  }

  HAL_FLASH_Lock(); //lock the flash

}

static uint32_t flash_erase_app_sector(void){

  uint32_t sector_error = 0;

  FlashErase.Sector = FLASH_SECTOR_5; //specifiy exact sector to earase
  FlashErase.NbSectors = 1; // specifiy number of following+initial sectors to be erased
  FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS; //specify operation type
  FlashErase.VoltageRange = VOLTAGE_RANGE_3; //specify parallelism of erase

  HAL_FLASH_Unlock();
  //status = (uint8_t) HAL_FLASHEx_Erase(&FlashErase, &sector_error);
  HAL_FLASHEx_Erase(&FlashErase, &sector_error);
  HAL_FLASH_Lock(); //lock the flash

  return sector_error;
}


static void flash_write_dummy(void){

  volatile uint32_t FlashMemAddress = 0x08040000;
  uint32_t test_word = 0xDEADBEEF;

  HAL_FLASH_Unlock(); //unlock the flash
  //Start writing from the 1st address
  for(int iterator = 0; iterator < 0x8000; iterator++) //32768 = 128k/4 (4 bytes pro wort)
  {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,FlashMemAddress, test_word);
      FlashMemAddress += 4; // word (4 bytes) increment
  }

  HAL_FLASH_Lock(); //lock the flash
}


static void jump_to_app(void){
  typedef void (*void_fn)(void);
  uint32_t* reset_vector_entry = (uint32_t *)(0x8020000U+4U); //get adress of reset handler of main app and store it as pointer
  uint32_t* reset_vector = (uint32_t*)(*reset_vector_entry);  //get content of pouinter (should be antoher pointer)
  void_fn jump_fn = (void_fn)reset_vector;                    //convert pointer to function pointer

  jump_fn(); //call stored function pointer as function
}

/*------------MxCube written functions---------------*/

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 200;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI3_Init(void)
{

  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  
}


static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = (50000-1);
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = (0xFFFF-1);
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
  // User can add his own implementation to report the file name and line number,
  // ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) 
}

#endif 
