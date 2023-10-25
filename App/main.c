/*!
    \file  main.c
    \brief ADC0_ADC1_ADC2_inserted_parallel
    
    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
    \version 2022-03-09, V3.0.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2022, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f450i_eval.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"

static void app_task1(void* pvParameters)
{
		for(;;)
		{
				printf("task1 enter \r\n");
				gpio_bit_set(GPIOC, GPIO_PIN_6);	
				vTaskDelay(1000);
				gpio_bit_reset(GPIOC, GPIO_PIN_6);	
				vTaskDelay(1000);
		}
}

static void app_task2(void* pvParameters)
{
		for(;;)
		{
				printf("task2 enter \r\n");
				gpio_bit_set(GPIOC, GPIO_PIN_13);	
				vTaskDelay(200);
				gpio_bit_reset(GPIOC, GPIO_PIN_13);
				vTaskDelay(200);
		}
}

void Led_Init(void)
{
		//��ʼ��ʱ��
		rcu_periph_clock_enable(RCU_GPIOC);
		//�����������ģʽ
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_13);
		//������������
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6|GPIO_PIN_13);	
}
/*!
\brief      main function
\param[in]  none
\param[out] none
\retval     none
*/
int main(void)
{
		/*�����ж�ʹ��Ϊ����4,�ж�Ƕ������Ӧ���ȼ�*/
		/*�����ж�ʹ��Ϊ����0,���ж�Ƕ��*/
		nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
		//NVIC_SetPriorityGrouping(NVIC_PRIGROUP_PRE0_SUB4);

		/* ʹ��uart5 debug log*/
		gd_eval_com_init(EVAL_COM1);

		Led_Init();

		/* ����app_task1���� */
		xTaskCreate((TaskFunction_t )app_task1,  	/* ������ں��� */
				(const char*    )"app_task1",					/* �������� */
				(uint16_t       )128,  								/* ����ջ��С */
				(void*          )NULL,								/* ������ں������� */
				(UBaseType_t    )4, 									/* ��������ȼ� */
				(TaskHandle_t*  )NULL);								/* ������ƿ�ָ�� */ 

				
		/* ����app_task2���� */
		xTaskCreate((TaskFunction_t )app_task2,  	/* ������ں��� */
				(const char*    )"app_task2",					/* �������� */
				(uint16_t       )128,  								/* ����ջ��С */
				(void*          )NULL,								/* ������ں������� */
				(UBaseType_t    )4, 									/* ��������ȼ� */
				(TaskHandle_t*  )NULL);								/* ������ƿ�ָ�� */ 

		/* ����������� */
		vTaskStartScheduler(); 
}
