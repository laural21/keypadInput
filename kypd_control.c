#include "stm32f7xx_hal.h"              // Keil::Device:STM32Cube HAL:Common

#include <stm32f7xx.h>
#include <stdlib.h>
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.STM32F746G-Discovery::Board Support:Graphic LCD


static void SystemClock_Config(void);

extern GLCD_FONT GLCD_Font_16x24;

//Forward declare read_kypd function
signed char read_kypd(void);

int main(void)
{	
	signed char key = read_kypd();
	
	//Introduce state variable for security system. 0 by default, waiting for first key
	char state = 0;
	
	SystemClock_Config();
	
  //Activate GPIOB, GPIOF, GPIOI
  RCC->AHB1ENR |= (1 << 5) | (1 << 8);

  //Columns on the keyboard are outputs and rows are inputs
  //Define PI0 -> PI3 as outputs
  GPIOI->MODER |= (1 << 6) | (1 << 4) | (1 << 2) | (1 << 0);
    
  //PF6 -> PF9 are inputs by default

	//Initialize the screen, set background and foreground colors, font, etc.
	GLCD_Initialize();
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_DARK_GREEN);
	GLCD_ClearScreen();
	
	GLCD_SetFont(&GLCD_Font_16x24);
	
	/*
	//Read the keypad and display button pressed on the screen
	
	while(1){
		key=read_kypd();
		if(key == -1){
			char display_msg[] = "No key pressed";
			GLCD_DrawString(160, 204, display_msg);
		}
		else if(key == -2){
			char display_msg[] = "Multiple keys error";
			GLCD_DrawString(160, 204, display_msg);
		}
		else{
			GLCD_DrawChar(160, 240, key);
		}
	}
	return 0;
	*/
	char pwd[4]="";
	char unlocked[] = "Unlocked";
	char incorrect[] = "Incorrect password";
	char default_msg[] = "Enter password:";
	
	while(1){
		if(read_kypd() == -3){
			state = 0;
		}
		if(state == 0){
			GLCD_DrawString(120, 120, default_msg);
			if(read_kypd()=='8'){
				state++;
			}
		}
		else if(state == 1){
			pwd[0] = 'x';
			GLCD_DrawString(120, 120, default_msg);
			GLCD_DrawString(120, 140, pwd);
			if(read_kypd()=='6'){
				pwd[1] = 'x';
				GLCD_DrawString(120, 140, pwd);
				state++;
			} else if(!((read_kypd()=='8') || (read_kypd() == -1))){
				GLCD_ClearScreen();
				state = 0;
			}
		}
		else if(state == 2){
			GLCD_DrawString(120, 120, default_msg);
			GLCD_DrawString(120, 140, pwd);
			if(read_kypd()=='E'){
				pwd[2] = 'x';
				GLCD_DrawString(120, 140, pwd);	
				state++;
			} else if(!((read_kypd()=='6') || (read_kypd() == -1))){
					GLCD_DrawString(120, 160, incorrect);
					state = 0;
				}
			}
		else if(state == 3){
			GLCD_DrawString(120, 140, pwd);
			GLCD_DrawString(120, 160, unlocked);
		}
	}
}



signed char read_kypd(void){
	
	char keys_pressed = 0;
	signed char key;
	int INPUT = 0;
	char reset = 0;
	
	/*
	Set each column to be 0 and read the rows for that column, represented always by
	PF6-PF9. To write the corresponding bits of INPUT, make sure to shift from bits
	6-9 to the appropriate bits.
	Buttons will be read in the following order: 1470258F369EABCD.
	*/
	GPIOI->ODR |= 0x0F;
	GPIOI->ODR &= ~(1 << 0);
	INPUT = (GPIOF->IDR & 0x3C0) >> 6;
	
	GPIOI->ODR |= 0x0F;
	GPIOI->ODR &= ~(1 << 1);
	INPUT |= (GPIOF->IDR & 0x3C0) >> 2;
	
	GPIOI->ODR |= 0x0F;
	GPIOI->ODR &= ~(1 << 2);
	INPUT |= (GPIOF->IDR & 0x3C0) << 2;
	
	GPIOI->ODR |= 0x0F;
	GPIOI->ODR &= ~(1 << 3);
	INPUT |= (GPIOF->IDR & 0x3C0) << 6;
	
	reset |= (GPIOI->IDR & 0x800) >> 11;
	if(reset){
		return -3;
	}
	
	//For each 0 value in INPUT increment keys_pressed and change the value of key
	if(!(INPUT & 0x01)){
		key = '1';
		keys_pressed++;
	}
	if(!(INPUT & 0x02)){
		key = '4';
		keys_pressed++;
	}
	if(!(INPUT & 0x04)){
		key = '7';
		keys_pressed++;
	}
	if(!(INPUT & 0x08)){
		key = '0';
		keys_pressed++;
	}
	if(!(INPUT & 0x10)){
		key = '2';
		keys_pressed++;
	}
	if(!(INPUT & 0x20)){
		key = '5';
		keys_pressed++;
	}
	if(!(INPUT & 0x40)){
		key = '8';
		keys_pressed++;
	}
	if(!(INPUT & 0x80)){
		key = 'F';
		keys_pressed++;
	}
	if(!(INPUT & 0x100)){
		key = '3';
		keys_pressed++;
	}
	if(!(INPUT & 0x200)){
		key = '6';
		keys_pressed++;
	}
	if(!(INPUT & 0x400)){
		key = '9';
		keys_pressed++;
	}
	if(!(INPUT & 0x800)){
		key = 'E';
		keys_pressed++;
	}
	if(!(INPUT & 0x1000)){
		key = 'A';
		keys_pressed++;
	}
	if(!(INPUT & 0x2000)){
		key = 'B';
		keys_pressed++;
	}
	if(!(INPUT & 0x4000)){
		key = 'C';
		keys_pressed++;
	}
	if(!(INPUT & 0x8000)){
		key = 'D';
		keys_pressed++;
	}
	
	//If keys_pressed == 1, return key pressed, return error otherwise
	if(keys_pressed == 1){
		return key;
	}
	else if(keys_pressed > 1){
		return -2;
	}
	else if(keys_pressed == 0){
		return -1;
	}
}

void SysTick_Handler (void)
{
    HAL_IncTick();
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

