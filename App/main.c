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

#include "os_api.h"

os_task_t  task1_handle;
os_task_t  task2_handle;

static void app_task1(void* pvParameters)
{
		for(;;)
		{
			printf("task1 enter %u\r\n", os_time_get());
			gpio_bit_set(GPIOC, GPIO_PIN_6);	
			os_msleep(1000);
			gpio_bit_reset(GPIOC, GPIO_PIN_6);	
			os_msleep(1000);
		}
}

static void app_task2(void* pvParameters)
{
		for(;;)
		{
			printf("task2 enter %u\r\n", os_time_get());
			gpio_bit_set(GPIOC, GPIO_PIN_13);	
			os_msleep(200);
			gpio_bit_reset(GPIOC, GPIO_PIN_13);
			os_msleep(200);
		}
}

void Led_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOC);
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6|GPIO_PIN_13);	
}

int main(void)
{
		nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
		/* configure uart5 */
		gd_eval_com_init(EVAL_COM1);
		/* configure spi flash */
		gd_eval_GD25Q40_Init();

		//DRV_GD25Q40_BulkErase();
		char *test = "12345678";
		gd_eval_GD25Q40_BufferWrite((uint8_t *)test, 0x000000000, 8);

		uint8_t tmp[20] = {0};
		gd_eval_GD25Q40_BufferRead(tmp, 0x000000000, 8);
		printf("flash1 %s\r\n", tmp);

		Led_Init();

		os_task_create(app_task1, "app_task1",128,NULL,4,&task1_handle);
		os_task_create(app_task2, "app_task2",128,NULL,4,&task2_handle);

		vTaskStartScheduler();
		for(;;){}
}
