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

static BYTE work[FF_MAX_SS];/**< 挂载工作内存,不可放入线程,占用内存太大*/
static FATFS fs;/**< 磁盘挂载对象,不可放入线程,占用内存太大*/
static FIL file, file_w, file_r;/**< 文件对象,不可放入线程,占用内存太大*/
void file_write_UT(void);
void file_read_UT(void);
void file_write_read_UT(void);

static void app_task1(void* pvParameters)
{
		for(;;)
		{
			printf("task1 enter %u\r\n", os_time_get());
			gpio_bit_set(GPIOC, GPIO_PIN_6);	
			os_msleep(1000);
			gpio_bit_reset(GPIOC, GPIO_PIN_6);	
			os_msleep(1000);
			//file_write_UT();
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
			//file_read_UT();
		}
}

void Led_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOC);
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6|GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6|GPIO_PIN_13);	
}

void spi_flash_UT(void)
{
		printf("spi_flash_UT>>>>\r\n");
		const char g_TestBuf1[100] = "12345,hello world,123456\r\n";
		char g_TestBuf2[100] = {0};
		gd_eval_GD25Q40_BufferWrite((uint8_t *)g_TestBuf1, 0, strlen(g_TestBuf1));
		gd_eval_GD25Q40_BufferRead((uint8_t *)g_TestBuf2, 0, strlen(g_TestBuf1));
		printf("spi flash read :%s\r\n", g_TestBuf2);
}

void file_write_read_UT(void)
{
		printf("file_write_read_UT>>>>\r\n");
		FRESULT res_flash;
		uint8_t g_TestBuf1[100] = "0:test1.txt:12345,hello world,123456\r\n";
		uint32_t bytesWrite = 0;
		uint32_t bytesRead = 0;
		uint8_t buffer[100] = {0};
	
		res_flash = f_open(&file, "0:test1.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
		if(res_flash == FR_OK){
				res_flash = f_write(&file, g_TestBuf1, sizeof(g_TestBuf1), &bytesWrite);
				if (res_flash != FR_OK){
						printf("write file fail\r\n");
						while(1);
				}else{
					printf("write file success\r\n");

					f_lseek(&file, 0);//必须添加，否则读不到

					res_flash = f_read(&file, buffer, sizeof(buffer), &bytesRead);
					if (res_flash == FR_OK){
						printf("read from file, len = %u:%s\r\n", bytesRead, buffer);
					}
				}
				f_close(&file); 
		}else{
			printf("file_write_UT open file fail\r\n");
		}
}

void file_read_UT(void)
{
		printf("read file UT>>>>\r\n");

		FRESULT res_flash;
		uint32_t bytesRead = 0;
		uint8_t buffer[50] = {0};
		res_flash = f_open(&file_r, "0:test1.txt", FA_OPEN_EXISTING | FA_READ);
		if(res_flash != FR_OK){
			printf("file_read_UT open file fail\r\n");
	  }else{
			res_flash = f_read(&file_r, buffer, sizeof(buffer), &bytesRead);
			if (res_flash == FR_OK){
				printf("read from file, len = %u:%s\r\n", bytesRead, buffer);
			}else{
				printf("read file fail\r\n");
				while(1);
			}
			f_close(&file_r); 
		}
}

void file_write_UT(void)
{
		printf("write file UT>>>>\r\n");

		FRESULT res_flash;
		uint8_t g_TestBuf1[50] = "0:test2.txt: 12345,hello world,123456\r\n";
		uint32_t bytesWrite = 0;

		res_flash = f_open(&file_w, "0:test2.txt", FA_CREATE_ALWAYS | FA_WRITE);
		if(res_flash == FR_OK){
				f_lseek(&file_w, f_size(&file_w));//指向末尾
				res_flash = f_write(&file_w, g_TestBuf1, sizeof(g_TestBuf1), &bytesWrite);
				if (res_flash != FR_OK){
						printf("write file fail\r\n");
						while(1);
				}else{
					printf("write file success\r\n");
				}
				f_close(&file_w); 
		}else{
			printf("file_write_UT open file fail\r\n");
		}
}

void flash_FAT_format(char *disk_name, char *alias)
{
		uint8_t res = f_mount(&fs,disk_name,1);/**< 挂载文件系统, disk_name就是挂载的设备号为0的设备,1立即执行挂载*/	
		if(res==FR_NO_FILESYSTEM)/**< FR_NO_FILESYSTEM值为13,表示没有有效的设备*/
		{
			res = f_mkfs(disk_name,0,work,sizeof(work));//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
			res = f_mount(NULL, disk_name, 1);/**< 取消文件系统*/
      res = f_mount(&fs, disk_name, 1);/**< 挂载文件系统*/
			if(res==0)
			{
				//f_setlabel((const TCHAR *)alias);	//设置Flash磁盘的名字为：TT
				printf("format disk success\r\n");
			}else{
				printf("format disk fail\r\n");   //格式化失败
			}
		}
		printf("mount disk success\r\n");
}

int main(void)
{
		Led_Init();

		nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
		/* configure uart5 */
		gd_eval_com_init(EVAL_COM1);
		gd_eval_GD25Q40_Init();
		gd_eval_GD25Q40_BulkErase();
		printf("FLASH ID = 0x%x\r\n", gd_eval_GD25Q40_ReadID());

		spi_flash_UT();
	
		flash_FAT_format("0:", "0:TT");

	  file_write_read_UT();
	  file_write_read_UT();
		file_read_UT();

		os_task_create(app_task1, "app_task1",1024,NULL,3,&task1_handle);//1k PSP
		os_task_create(app_task2, "app_task2",1024,NULL,4,&task2_handle);//1k PSP

		vTaskStartScheduler();
		while(1);
}
