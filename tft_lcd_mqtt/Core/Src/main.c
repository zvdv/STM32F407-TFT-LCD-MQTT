/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
// PA1 UART RX
// PC10 UART TX
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define THERMRESNOM 100000 //datasheet says thermistor has resistance of 100k at 25C
#define BCOEFF 3950
#define TEMPNOM 25
#define ADCMAX 4095  //2^12-1 for a 12 bit ADC
#define AVG_OVER 100

#define USER_BUTTON 0
#define MQTT_BUTTON 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
bool buttonPressed = false;
bool lastbutton = false;

uint32_t adc_input = 3800; // initialized to avoid spike in first calculation of temperature

float thermRes;
float temp1;
float temperature = 20.0;
int avg_count = 0;
float avg_total = 0.0;

char temp_data_buffer[6] ="\0"; // buffer of temperature data
uint8_t button_buffer[1] = {0};
char user_button_buffer[2] = "\0";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void leds(char selection)
{
	if(selection == 'R'){
		  HAL_GPIO_TogglePin(RedLED_GPIO_Port, RedLED_Pin);
		 // HAL_Delay(50);
	}
	else if(selection == 'B') {
		HAL_GPIO_TogglePin(BlueLED_GPIO_Port, BlueLED_Pin);
		//HAL_Delay(50);
	}
	else if(selection == 'G') {
		HAL_GPIO_TogglePin(GreenLED_GPIO_Port, GreenLED_Pin);
		//HAL_Delay(50);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
     if(hadc->Instance==ADC1) {
	     adc_input = HAL_ADC_GetValue(&hadc1);
	 }
     HAL_GPIO_TogglePin(OrangeLED_GPIO_Port, OrangeLED_Pin);
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
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_CRC_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_UART4_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
  Displ_Init(Displ_Orientat_90);
  Displ_BackLight('I'); // initializes backlight
  HAL_TIM_Base_Start_IT(&TGFX_T);
  Displ_BackLight('1'); // turn on the backlight

  HAL_TIM_Base_Start(&htim3);
  HAL_ADC_Start_IT(&hadc1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // transmit initial temperature null string
  HAL_UART_Transmit(&huart4,(uint8_t *)temp_data_buffer,strlen(temp_data_buffer),50);

   while (1)
  {
//	  HAL_ADC_Start(&hadc1);
//	  HAL_ADC_PollForConversion(&hadc1, 200);
//	  adc_input = HAL_ADC_GetValue(&hadc1);
//	  HAL_ADC_Stop(&hadc1);
//	  HAL_Delay(200);

	  // this equation converts ADC value to thermistor resistance
	  // 10k is resistor between thermistor and 3.3 V
	  thermRes = 10000*adc_input/(ADCMAX-adc_input);
	  avg_total += thermRes;
	  avg_count++;
	  // solving equation 1/T = 1/To + 1/B ln(R/Ro)
	  // thermistor has resistance Ro at To, temperatures in Kelvin
	  if(avg_count == AVG_OVER) {
		  thermRes = avg_total/(float)AVG_OVER;
		  temperature = thermRes/THERMRESNOM;
	  	  temperature = log(temperature);
	  	  temperature /= BCOEFF;
	  	  temperature += 1.0/(TEMPNOM + 273.15);
	  	  temperature = 1.0/temperature;
	  	  temperature = temperature - 273.15;
	  	  avg_total = 0.0;
	  	  avg_count = 0;
	  	  if(temperature > -120.0 && temperature < 120.0) {
	  		  sprintf(temp_data_buffer,"%4.1f",temperature);
	  		  HAL_UART_Transmit(&huart4,(uint8_t *)temp_data_buffer,strlen(temp_data_buffer),50);
	  	  }
	  }

	  #if USER_BUTTON
	  // use the following code with User Button control
      if(HAL_GPIO_ReadPin(UserButton_GPIO_Port, UserButton_Pin) == GPIO_PIN_RESET && lastbutton != false)
      {
		 buttonPressed = false;
//		 sprintf(user_button_buffer, "%d", (int)buttonPressed);
//		 HAL_UART_Transmit(&huart4,(uint8_t *)user_button_buffer,1,1);
		 lastbutton = buttonPressed;
	  }
	  else if (HAL_GPIO_ReadPin(UserButton_GPIO_Port, UserButton_Pin) == GPIO_PIN_SET && lastbutton != true)
	  {
		 buttonPressed = true;
//		 sprintf(user_button_buffer, "%d", (int)buttonPressed);
//		 HAL_UART_Transmit(&huart4,(uint8_t *)user_button_buffer,1,1);
		 lastbutton = buttonPressed;
	  }
	  #elif MQTT_BUTTON
	  // use the following code with MQTT button subscription control
	  HAL_UART_Receive(&huart4,(uint8_t *) button_buffer, 1, 1); // with 1 msec timeout since messages are rare
	  if(button_buffer[0] == 0 || button_buffer[0] == 1)
		  buttonPressed = (bool)button_buffer[0];
	  #endif
    /* USER CODE END WHILE */

      MX_TouchGFX_Process();
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
