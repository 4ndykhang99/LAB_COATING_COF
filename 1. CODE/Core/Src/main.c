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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "ssd1306.h"
#include "fonts.h"
#include "mylan.h"

#define FLASH_ADDR_PAGE_122 ((uint32_t)0x0800E810)
#define FLASH_ADDR_PAGE_123 ((uint32_t)0x0800EC10)
#define FLASH_ADDR_PAGE_124 ((uint32_t)0x0801F010)
#define FLASH_ADDR_PAGE_125 ((uint32_t)0x0801F410)
#define FLASH_ADDR_PAGE_126 ((uint32_t)0x0801F810)
#define FLASH_ADDR_PAGE_127 ((uint32_t)0x0801FC00)


#define FLASH_currentspeed_start_addr  FLASH_ADDR_PAGE_126
#define FLASH_currentspeed_end_addr  FLASH_ADDR_PAGE_127 + 0x400 - 0x01


#define FLASH_CW_start_addr FLASH_ADDR_PAGE_124
#define FLASH_CW_end_addr FLASH_ADDR_PAGE_125 + 0x400 - 0x01

#define FLASH_CCW_start_addr FLASH_ADDR_PAGE_122
#define FLASH_CCW_end_addr FLASH_ADDR_PAGE_123 + 0x400 - 0x01 



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
//uint32_t startpage = FLASH_currentspeed__start_addr;
//uint32_t dataread;



int defaultmenu_flag = 0;
int Main_menu_flag = 0;
int Freq_menu_flag = 0;
int COF_menu_flag = 0;

//Stepper flags
int CW_Stepper = 0;
int CCW_Stepper = 0;

int Freq_menu_chosen = 0; // 1: Frequency menu are chosing by the pointer
int COF_menu_chosen = 0;  // 1: COF measurament menu are chosing by the pointer

// button flags
int button_A;
int button_B;
int button_C;
int button_D;
int button_E;
int button_F;
int CW_limit = 0;
int CCW_limit = 0;

//Freq variable

int T_High = 300;
int T_Low = 300;


// speed variables
float current_speed;
int speed = 1000;
int N_currentspeed;
int R_currentspeed;

char V[10];
char R[10];

//Button debounce

uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
uint32_t counterOutside = 0; //For testing only
uint32_t counterInside = 0; //For testing only

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

// a function to get microsecond
void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
} 


//Flag declaration for each type of menu 
void resetbutton()
{
	button_A = 0;
	button_B = 0;
	button_C = 0;
	button_D = 0;
}
void Mainmenu()
{
	defaultmenu_flag = 0;
	Main_menu_flag = 1;
	Freq_menu_flag = 0;
	COF_menu_flag = 0;
}
void Freqmenu()
{
	defaultmenu_flag = 0;
	Main_menu_flag = 0;
	Freq_menu_flag = 1;
	COF_menu_flag = 0;
}
void COFmenu()
{
	defaultmenu_flag = 0;
	Main_menu_flag = 0;
	Freq_menu_flag = 0;
	COF_menu_flag = 1;
}
void defaultmenu()
{
	defaultmenu_flag = 1;
	Main_menu_flag = 0;
	Freq_menu_flag = 0;
	COF_menu_flag = 0;
}

//void default_F()
//{
//	
//}
void speedmonitor(float T)
{
		N_currentspeed = T/10;
    R_currentspeed = fmod(T,10);
    
}
void calculatime(float speed)
{
	///////////////////////////////////////////////////////////////
	// each turn of vitme equal to 5mm on journey
	// 500 pulse, we have 1 round rotating
	// duty circle of each pulse = T_High + T_Low (T_ high = 300)
	///////////////////////////////////////////////////////////////
	
	double t = (5/((speed)/(60*pow(10,6)))); //microsecond
	T_Low = (t/4000)/2 ;
	
	
}

////////////////////////////////////////////////////////////////////////
///
///				EEPROM - FLASH save voletive variable
///
///
///
///////////////////////////////////////////////////////////////////////
void FLASH_WritePage(uint32_t startPage,uint32_t endPage, uint32_t data32)
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInit;
	EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInit.PageAddress = startPage;
//	EraseInit.NbPages = 1;
	EraseInit.NbPages = (endPage - startPage + 0x01)/(FLASH_PAGE_SIZE);
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&EraseInit,&PageError);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startPage, data32);
	HAL_FLASH_Lock();
}

// Read addr and return addr's value to data variable 
uint32_t FLASH_ReadData32(uint32_t addr)
{
	uint32_t data = *(__IO uint32_t *)(addr);
	return data;
}



void test_1round()
{
	for (int i = 0; i<4000 ;i++)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
		delay_us(T_Low);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
		delay_us(T_Low);
	}
	
}
//void VtoT(int speed)
//{
//	
//}


void menuHMI_selection(uint16_t speed_adjustment, uint16_t COF)
{
	SSD1306_Clear();
	SSD1306_GotoXY(14,0);
	SSD1306_Puts("COATING - COF",&Font_7x10,0x01);
	SSD1306_GotoXY(0,9);
	SSD1306_Puts("--------------------",&Font_7x10,0x01);
	SSD1306_GotoXY(0,20);
	SSD1306_Puts("Speed adjustment",&Font_7x10,speed_adjustment);
	SSD1306_GotoXY(0,32);
	SSD1306_Puts("COF Measuarament",&Font_7x10,COF);

//instruction	text
	SSD1306_GotoXY(0,52);
	SSD1306_Puts("a",&Font_7x10,0x00);
	SSD1306_GotoXY(7,52);
	SSD1306_Puts(":Up",&Font_7x10,0x01);
	
	SSD1306_GotoXY(32,52);
	SSD1306_Puts("b",&Font_7x10,0x00);
	SSD1306_GotoXY(39,52);
	SSD1306_Puts(":DWN",&Font_7x10,0x01);
	
	SSD1306_GotoXY(69,52);
	SSD1306_Puts("c",&Font_7x10,0x00);
	SSD1306_GotoXY(76,52);
	SSD1306_Puts(":<",&Font_7x10,0x01);
	
	SSD1306_GotoXY(92,52);
	SSD1306_Puts("d",&Font_7x10,0x00);
	SSD1306_GotoXY(99,52);
	SSD1306_Puts(":Ent",&Font_7x10,0x01);
	
	
	
	SSD1306_UpdateScreen();
}
void COFHMI()
{
	SSD1306_Clear();
	SSD1306_GotoXY(14,0);
	SSD1306_Puts("COF Measurament",&Font_7x10,0x01);
	SSD1306_GotoXY(0,11);
	SSD1306_Puts("--------------------",&Font_7x10,0x01);
	SSD1306_GotoXY(0,24);
	SSD1306_Puts("AVG COF =",&Font_11x18,0x01);
	SSD1306_GotoXY(80,24);
//	sprintf(F, "%d", Freq); //convert int to String type 
//	SSD1306_Puts(F ,&Font_11x18,0x01);
	
	
	SSD1306_GotoXY(0,52);
	SSD1306_Puts("a",&Font_7x10,0x00);
	SSD1306_GotoXY(7,52);
	SSD1306_Puts(":Up",&Font_7x10,0x01);
	
	SSD1306_GotoXY(32,52);
	SSD1306_Puts("b",&Font_7x10,0x00);
	SSD1306_GotoXY(39,52);
	SSD1306_Puts(":DWN",&Font_7x10,0x01);
	
	SSD1306_GotoXY(69,52);
	SSD1306_Puts("c",&Font_7x10,0x00);
	SSD1306_GotoXY(76,52);
	SSD1306_Puts(":<",&Font_7x10,0x01);
	
	SSD1306_GotoXY(92,52);
	SSD1306_Puts("d",&Font_7x10,0x00);
	SSD1306_GotoXY(99,52);
	SSD1306_Puts(":Ent",&Font_7x10,0x01);
	
	SSD1306_UpdateScreen();
}
void SpeedHMI()
{
	SSD1306_Clear();
	SSD1306_GotoXY(14,0);
	SSD1306_Puts("SPEED ADJUSTMENT",&Font_7x10,0x01);
	SSD1306_GotoXY(0,11);
	SSD1306_Puts("--------------------",&Font_7x10,0x01);
	SSD1306_GotoXY(0,24);
	SSD1306_Puts("V=",&Font_11x18,0x01);
	
	
	speedmonitor(current_speed);
	SSD1306_GotoXY(23,24);
	sprintf(V, "%d", N_currentspeed ); //convert int to String type 
	SSD1306_Puts(V ,&Font_11x18,0x01);
	SSD1306_GotoXY(46,24);
	SSD1306_Puts(".",&Font_11x18,0x01);
	SSD1306_GotoXY(57,24);
	sprintf(R, "%d", R_currentspeed ); //convert int to String type 
	SSD1306_Puts(R ,&Font_11x18,0x01);
	
//	SSD1306_GotoXY(69,24);
//	SSD1306_Puts("m/min", &Font_11x18,0x01);
	
//	sprintf(F, "%d", speed[current_speed]); //convert int to String type 
//	SSD1306_Puts(F ,&Font_11x18,0x01);
	
	
	//instruction	text
	SSD1306_GotoXY(0,52);	
	SSD1306_Puts("a",&Font_7x10,0x00);
	SSD1306_GotoXY(7,52);
	SSD1306_Puts(":Up",&Font_7x10,0x01);
	
	SSD1306_GotoXY(32,52);
	SSD1306_Puts("b",&Font_7x10,0x00);
	SSD1306_GotoXY(39,52);
	SSD1306_Puts(":DWN",&Font_7x10,0x01);
	
	SSD1306_GotoXY(69,52);
	SSD1306_Puts("c",&Font_7x10,0x00);
	SSD1306_GotoXY(76,52);
	SSD1306_Puts(":<",&Font_7x10,0x01);
	
	SSD1306_GotoXY(92,52);
	SSD1306_Puts("d",&Font_7x10,0x00);
	SSD1306_GotoXY(99,52);
	SSD1306_Puts(":Ent",&Font_7x10,0x01);
	
	SSD1306_UpdateScreen();
}
void menuHMI()
{
	SSD1306_Clear();
	SSD1306_GotoXY(14,0);
	SSD1306_Puts("COATING - COF",&Font_7x10,0x01);
	SSD1306_GotoXY(0,9);
	SSD1306_Puts("--------------------",&Font_7x10,0x01);
	SSD1306_GotoXY(0,20);
	SSD1306_Puts("Speed adjustment",&Font_7x10,0x00);
	SSD1306_GotoXY(0,32);
	SSD1306_Puts("COF Measuarament",&Font_7x10,0x01);

//instruction	text
	SSD1306_GotoXY(0,52);
	SSD1306_Puts("a",&Font_7x10,0x00);
	SSD1306_GotoXY(7,52);
	SSD1306_Puts(":Up",&Font_7x10,0x01);
	
	SSD1306_GotoXY(32,52);
	SSD1306_Puts("b",&Font_7x10,0x00);
	SSD1306_GotoXY(39,52);
	SSD1306_Puts(":DWN",&Font_7x10,0x01);
	
	SSD1306_GotoXY(69,52);
	SSD1306_Puts("c",&Font_7x10,0x00);
	SSD1306_GotoXY(76,52);
	SSD1306_Puts(":<",&Font_7x10,0x01);
	
	SSD1306_GotoXY(92,52);
	SSD1306_Puts("d",&Font_7x10,0x00);
	SSD1306_GotoXY(99,52);
	SSD1306_Puts(":Ent",&Font_7x10,0x01);
	
	
	
	SSD1306_UpdateScreen();
}
	
void defaultHMI()
{
	SSD1306_Clear();
	SSD1306_GotoXY(14,0);
	SSD1306_Puts("COATING - COF",&Font_7x10,0x01);
	SSD1306_GotoXY(0,9);
	SSD1306_Puts("--------------------",&Font_7x10,0x01);
	SSD1306_GotoXY(0,21);
	SSD1306_Puts("Click A: Run FWD",&Font_7x10,0x01);
	SSD1306_GotoXY(0,31);
	SSD1306_Puts("Click B: Run BWD",&Font_7x10,0x01);
	SSD1306_GotoXY(0,41);
	SSD1306_Puts("Click C: Stop",&Font_7x10,0x01);
	SSD1306_GotoXY(0,51);
	SSD1306_Puts("Click D: Open Menu",&Font_7x10,0x01);
	SSD1306_UpdateScreen();
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  UNUSED(GPIO_Pin);
	currentMillis = HAL_GetTick();
  if (GPIO_Pin == GPIO_PIN_0 && (currentMillis - previousMillis > 100)) // FWD
	{
		button_A = 1;
		button_B = 0;
		button_C = 0;
		button_D = 0;
		button_F = 0;
//		CCW_limit = 0;
		CW_Stepper = 1;
		CCW_Stepper = 0;
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7, GPIO_PIN_SET);
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		
		previousMillis = currentMillis;
	} 
	else if (GPIO_Pin == GPIO_PIN_1 && (currentMillis - previousMillis > 100)) // BWD
	{
		button_A = 0;
		button_B = 1;
		button_C = 0;
		button_D = 0;
		button_E = 0;
//		CW_limit = 0;
		CW_Stepper = 0;
		CCW_Stepper = 1;
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7, GPIO_PIN_RESET);
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		
		
		previousMillis = currentMillis;
		
	}
	else if (GPIO_Pin == GPIO_PIN_2 && (currentMillis - previousMillis > 100)) //Stop button
	{
		button_A = 0;
		button_B = 0;
		button_C = 1;
		button_D = 0;
		button_E = 0;
		button_F = 0;
//		CW_limit = 0;
		CW_Stepper = 0;
		CCW_Stepper = 0;
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		previousMillis = currentMillis;
	}
	else if (GPIO_Pin == GPIO_PIN_3 && (currentMillis - previousMillis > 100)) // Menu
	{
		button_A = 0;
		button_B = 0;
		button_C = 0;
		button_D = 1;
		button_E = 0;
		button_F = 0;
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		previousMillis = currentMillis;

	}
	else if (GPIO_Pin == GPIO_PIN_4 && (currentMillis - previousMillis > 100)) // Journey switch
	{
//		button_A = 0;
		button_B = 0;
		button_C = 0;
		button_D = 0;
		button_E = 1;
		button_F = 0;
		CW_Stepper = 0;
		CW_limit = 1;
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		previousMillis = currentMillis;
	}
	else if (GPIO_Pin == GPIO_PIN_5 &&  (currentMillis - previousMillis > 100))// Journey switch 2
	{
		button_A = 0;
//		button_B = 0;
		button_C = 0;
		button_D = 0;
		button_E = 0;
		button_F = 1;
		CCW_Stepper = 0;
		CCW_limit = 1;
		FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
		FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
		previousMillis = currentMillis;
	}
//	else if (GPIO_Pin == GPIO_PIN_5 && GPIO_Pin == GPIO_PIN_4)
//	{
////		button_A = 0;
////		button_B = 0;
//		button_C = 0;
//		button_D = 0;
//		button_E = 1;
//		button_F = 1;
//		CW_Stepper = 0;
//		CCW_limit = 1;
//		CW_limit = 1;
//		previousMillis = currentMillis;
//	}
}

void Run_FWD()
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
	delay_us(T_Low);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
	delay_us(T_Low);
}
//void Run_BWD()
//{
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
//	delay_us(T_Low);
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
//	delay_us(T_Low);
//}

/*--------------------------------------------------------------------------------------------------------
Des: This void is to determine what exacly the pointer and Interrupt service routine have done on each strigger

author: Khang tran
Date: 8/2/23

----------------------------------------------------------------------------------------------------------
*/
void Menu_handler()
{
	if(Main_menu_flag)
	{
		menuHMI();
		Freq_menu_chosen = 1;
		COF_menu_chosen = 0;
		HAL_Delay(200);
		
		while(Main_menu_flag)
			if(button_A && COF_menu_chosen)
			{
				menuHMI_selection(0x00,0x01);				// COF ==> Freq
				Freq_menu_chosen = 1; 
				COF_menu_chosen = 0;
				resetbutton();
				
			}
			else if(button_A && Freq_menu_chosen) // Keep Freq menu pointer
			{
				menuHMI_selection(0x00,0x01);
				Freq_menu_chosen = 1;
				COF_menu_chosen = 0;
				resetbutton();
			}
			else if(button_B && COF_menu_chosen)  // Keep COF menu pointer
			{
				menuHMI_selection(0x01,0x00);
				Freq_menu_chosen = 0;
				COF_menu_chosen = 1;
				resetbutton();
			}
			else if(button_B && Freq_menu_chosen)	// Freq ==> COF
			{
				menuHMI_selection(0x01,0x00);
				Freq_menu_chosen = 0;
				COF_menu_chosen = 1;
				resetbutton();
			}
			else if(button_D && Freq_menu_chosen)
			{
				Freqmenu();
				resetbutton();
			}
			else if(button_D && COF_menu_chosen)
			{
				COFmenu();
				resetbutton();
			}
			else if(button_C)
			{
				defaultmenu();
				resetbutton();
				if(CW_limit) 	// thoat ve man hinh default nhung van giu trang thai cong tac hanh trinh
				{
					button_E = 1;
					CW_limit = 0;
				}
				else if(CCW_limit)
				{
					button_F = 1;
					CCW_limit =0;
				}
				else
				{
					button_E = 0;
					button_F = 0;
					CW_limit = 0;
					CCW_limit = 0;
				}
				
			}
				
			
	}
	
// in this default menu, user are able to directly control the step motor VIA button A (CW) & button B (CCW) & button C to Stop
// Button D (menu) only be accessed in case of the stepper stopped ( CW flag and CCW flag == 0)
	
	
	if (defaultmenu_flag) // default menu access
	{
		defaultHMI();
		HAL_Delay(100);
		while(defaultmenu_flag)
		{
			if(button_D && CW_Stepper == 0  && CCW_Stepper == 0) 
			{
				Mainmenu();
				resetbutton();
			}
			else if (button_A)
			{	
//				CW_Stepper = 1;
//				CCW_Stepper = 0;
				CCW_limit = 0;
				button_A = 0;
				button_B = 0;
				button_C = 0;
				button_D = 0;
				button_F = 0;
				calculatime(speed);
				while(CW_Stepper)
				{	
					Run_FWD();
				}
				
			}
			else if (button_B)
			{
//				CW_Stepper = 0;
//				CCW_Stepper = 1;
				CW_limit = 0;
				button_A = 0;
				button_B = 0;
				button_C = 0; 
				button_D = 0;
				button_E = 0;
				calculatime(speed);
				while(CCW_Stepper)
				{
					Run_FWD();
				}
				
				
			}
			else if (button_C)
			{
				CW_Stepper = 0;
				CCW_Stepper = 0;
				resetbutton();
				button_E = 0;
				button_F = 0;
				if(CW_limit) 	// thoat ve man hinh default nhung van giu trang thai cong tac hanh trinh
				{
					button_E = 1;
					CW_limit = 0;
					FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
				}
				else if(CCW_limit)
				{
					button_F = 1;
					CCW_limit = 0;
					FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
				}
				else
				{
					button_E = 0;
					button_F = 0;
					CW_limit = 0;
					CCW_limit = 0;
					FLASH_WritePage(FLASH_CW_start_addr,FLASH_CW_end_addr,button_E);
					FLASH_WritePage(FLASH_CCW_start_addr,FLASH_CCW_end_addr,button_F);
				}
			}
			else if (button_E)
			{
				CW_limit = 1;
//				button_A = 0;
				button_B = 0; 
				button_C = 0;
				button_D = 0;
				button_F = 0;
				while (button_E)
				{
					CW_Stepper = 0;
					delay_us(50);
				}					
			}
			else if (button_F)
			{
				CCW_limit = 1;
				button_A = 0;
//				button_B = 0; 
				button_C = 0;
				button_D = 0;
				button_E = 0;
				while (button_F)
				{
					CCW_Stepper = 0;
					delay_us(50);
				}					
			}
		}
	}
	
	
	if (Freq_menu_flag) // Speed adjustment menu access
	{
		SpeedHMI();
		HAL_Delay(100);
		while(Freq_menu_flag)
		{
			if(button_C)
			{
				Mainmenu();
				resetbutton();
			}
			if(button_A)
			{
				if(speed < 4000)
				{
					speed += 100;
					current_speed = speed / 100;
					speedmonitor(current_speed);
					FLASH_WritePage(FLASH_currentspeed_start_addr, FLASH_currentspeed_end_addr, speed);
					SpeedHMI();				
					resetbutton();
				} 
				else 
				{
					speed = speed;
					current_speed = speed / 100;
					speedmonitor(current_speed);
					FLASH_WritePage(FLASH_currentspeed_start_addr, FLASH_currentspeed_end_addr, speed);
					SpeedHMI();
					resetbutton();
				}
			}
			else if(button_B)
			{
				if(speed > 200)
				{
					speed -= 100;			
					current_speed = speed / 100;
					speedmonitor(current_speed);
					FLASH_WritePage(FLASH_currentspeed_start_addr, FLASH_currentspeed_end_addr, speed);
					SpeedHMI();
					resetbutton();
				}
				else
				{
					speed = speed;
					current_speed = speed / 100;
					speedmonitor(current_speed);
					FLASH_WritePage(FLASH_currentspeed_start_addr, FLASH_currentspeed_end_addr, speed);
					SpeedHMI();
					resetbutton();
				}
			}
			else if(button_D)
			{
				Mainmenu();
				resetbutton();
			}
		}
	}
	if(COF_menu_flag) // COF monitor menu access
	{
		COFHMI();
		HAL_Delay(100);
		while(COF_menu_flag)
		{
				if(button_C)
			{
				Mainmenu();
				resetbutton();
			}
		}
	}
}


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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_TIM_Base_Start(&htim1);	
	
//	FLASH_WritePage(FLASH_currentspeed__start_addr, FLASH_currentspeed_end_addr, 2000);
//	FLASH_WritePage(FLASH_currentspeed__start_addr,FLASH_currentspeed_end_addr,speed);
//	
//		FLASH_WritePage(FLASH_CW_start_addr, FLASH_CW_end_addr, 0x01);
//	FLASH_WritePage(FLASH_currentspeed__start_addr,FLASH_CW_end_addr,speed);
//	
//		FLASH_WritePage(FLASH_CCW_start_addr, FLASH_CCW_end_addr, 0x01);
//	FLASH_WritePage(FLASH_currentspeed__start_addr,FLASH_CCW_end_addr,speed);
	
	
//	test_1round();
	SSD1306_Init();
	SSD1306_Clear();
//	SSD1306_Fill(0x00);
//	SSD1306_UpdateScreen();
//	HAL_Delay(1000);
	SSD1306_DrawBitmap(34, 4, mylanlogo, 60, 32, 0x01);
	SSD1306_GotoXY(25,38);
	SSD1306_Puts("Formulating",&Font_7x10,0x01);
	SSD1306_GotoXY(8,52);
	SSD1306_Puts("A Greener World",&Font_7x10,0x01);
	SSD1306_UpdateScreen();
	
	
	
	button_E = FLASH_ReadData32(FLASH_CW_start_addr);
	HAL_Delay(1000);
	button_F = FLASH_ReadData32(FLASH_CCW_start_addr);
	HAL_Delay(1000);
	speed = FLASH_ReadData32(FLASH_currentspeed_start_addr);

	
	current_speed = speed / 100;
	
	
	speedmonitor(current_speed);
//	HAL_Delay(5000);
	
	
	
	menuHMI_selection(0x01, 0x00);	
	defaultmenu();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		Menu_handler();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  hi2c1.Init.ClockSpeed = 400000;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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
