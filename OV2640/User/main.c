/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   GPIO���--ʹ�ù̼������LED��
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32 F407 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include <stdlib.h>
#include "./led/bsp_led.h" 
#include "./lcd/bsp_ili9806g_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./i2c/bsp_i2c.h"
#include "./camera/bsp_ov2640.h"

/*���������*/
uint32_t Task_Delay[NumOfTask]={0};
uint8_t dispBuf[100];
uint8_t fps=0;
extern  DMA_HandleTypeDef DMA_Handle_dcmi;
extern uint16_t img_width, img_height;
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{  
	OV2640_IDTypeDef OV2640_Camera_ID;	
  /* ϵͳʱ�ӳ�ʼ����216 MHz */
  SystemClock_Config();
  /* LED �˿ڳ�ʼ�� */
  LED_GPIO_Config();	
  /*��ʼ��USART1*/
  DEBUG_USART_Config(); 
	
	//LCD ��ʼ��
	ILI9806G_Init ();         
  
  LCD_SetFont(&Font16x32);
	LCD_SetColors(RED,BLACK);

  ILI9806G_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */

 //����0��3��5��6 ģʽ�ʺϴ���������ʾ���֣�
 //���Ƽ�ʹ������ģʽ��ʾ����	����ģʽ��ʾ���ֻ��о���Ч��			
 //���� 6 ģʽΪ�󲿷�Һ�����̵�Ĭ����ʾ����  
	ILI9806G_GramScan ( 5 );
	
  CAMERA_DEBUG("STM32F407 DCMI ����OV2640����");
  	//��ʼ�� I2C
	I2CMaster_Init(); 
  
  OV2640_HW_Init();
 

	HAL_Delay(100);


	/* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
	OV2640_ReadID(&OV2640_Camera_ID);

	if(OV2640_Camera_ID.PIDH  == 0x26)
  {
   CAMERA_DEBUG("%x%x",OV2640_Camera_ID.PIDH ,OV2640_Camera_ID.PIDL); 
  }
  else
  {
    LCD_SetTextColor(RED);
    ILI9806G_DispString_EN(10,10,"Can not detect OV2640 module,please check the connection!");
    CAMERA_DEBUG("û�м�⵽OV2640����ͷ�������¼�����ӡ�");

    while(1);  
  }
   OV2640_UXGAConfig();
   
	OV2640_Init();  

  	//����
  fps =0;
	Task_Delay[0]=1000;

 //ɨ��ģʽ������
  ILI9806G_GramScan(5);
  LCD_SetFont(&Font16x32);
  LCD_SetColors(RED,BLACK);

  ILI9806G_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
  ILI9806G_DispStringLine_EN(LINE(0),"BH 4.8 inch LCD + OV2640");

    /*DMA������ݴ��䵽Һ���������������ݰ��������� */
  ILI9806G_OpenWindow(0,0,img_width,img_height);
  
  while(1)
	{
		//ʹ�ô������֡��
		if(Task_Delay[0]==0)
		{	
//      HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, FSMC_Addr_ILI9806G_DATA,1);
      
			/*���֡��*/
			CAMERA_DEBUG("\r\n֡��:%.1f/s \r\n", (double)fps/5.0);
			//����
			fps =0;						
			Task_Delay[0]=5000; //��ֵÿ1ms���1������0�ſ������½�������
		}		
	}	
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
 void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1) {};
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    while(1) {};
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/