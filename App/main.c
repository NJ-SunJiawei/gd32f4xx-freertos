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

#define FLASH_WRITE_ADDRESS      0x000000
#define FLASH_READ_ADDRESS       FLASH_WRITE_ADDRESS


BYTE work[FF_MAX_SS];/**< 挂载工作内存,不可放入线程,占用内存太大*/
FATFS fs;/**< 磁盘挂载对象,不可放入线程,占用内存太大*/
FIL file, file_w, file_r;/**< 文件对象,不可放入线程,占用内存太大*/
DIR File1Dir;//目录信息结构体
char g_TestBuf1[64] = "12345,hello world,123456";
char g_TestBuf2[64] = {0};
char buffer[FF_MAX_SS];

void file_write_UT(char *filename);
void file_read_UT(char *filename);
void file_write_read_UT(char *filename);
void flash_FAT_avail_size(char *path);
void flash_FAT_check_file_status(char *filename);

os_sema_t  task_lock;

static void app_task1(void* pvParameters)
{
		for(;;)
		{
			BaseType_t err = os_sema_wait(task_lock, (unsigned int)-1);
			if(err != pdTRUE){
				printf("block err %u\r\n", os_time_get());
			}
			printf("task1 enter %u\r\n", os_time_get());
			gpio_bit_set(GPIOE, GPIO_PIN_2);	
			os_msleep(1000);
			gpio_bit_reset(GPIOE, GPIO_PIN_2);	
			os_msleep(1000);
		}
}
void EXTI10_15_IRQHandler(void)
{
		BaseType_t taskswitch;
    if(RESET != exti_interrupt_flag_get(EXTI_13)) {
				os_count_sema_post_ISR(task_lock, &taskswitch);
        exti_interrupt_flag_clear(EXTI_13);
    }
}

static void app_task2(void* pvParameters)
{
		for(;;)
		{
			printf("task2 enter %u\r\n", os_time_get());
			gpio_bit_set(GPIOE, GPIO_PIN_3);	
			os_msleep(200);
			gpio_bit_reset(GPIOE, GPIO_PIN_3);
			os_msleep(200);
		}
}

void Led_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOE);
		gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2|GPIO_PIN_3);
		gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2|GPIO_PIN_3);	
}

void spi_flash_UT1(void)
{
		printf("spi_flash_UT1>>>>\r\n");
		gd_eval_GD25Q40_BufferWrite((uint8_t *)g_TestBuf1, FLASH_WRITE_ADDRESS, strlen(g_TestBuf1));
		gd_eval_GD25Q40_BufferRead((uint8_t *)g_TestBuf2, FLASH_READ_ADDRESS, strlen(g_TestBuf1));
		printf("spi_flash_UT1 read :%s\r\n", g_TestBuf2);
}

void spi_flash_UT2(void)
{
		printf("spi_flash_UT2>>>>\r\n");
		spi_flash_buffer_write((uint8_t *)g_TestBuf1, FLASH_WRITE_ADDRESS, strlen(g_TestBuf1));
		spi_flash_buffer_read((uint8_t *)g_TestBuf2, FLASH_READ_ADDRESS, strlen(g_TestBuf1));
		printf("spi_flash_UT2 read :%s\r\n", g_TestBuf2);
}

void file_write_read_UT(char *filename)
{
		FRESULT res_flash;
		uint32_t bytesWrite = 0;
		uint32_t bytesRead = 0;

		res_flash = f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);//FA_OPEN_APPEND
		if(res_flash == FR_OK){
				res_flash = f_write(&file, g_TestBuf1, strlen(g_TestBuf1), &bytesWrite);
				//f_printf(&file, "%s\n", g_TestBuf1);
				if (res_flash != FR_OK){
						printf("write file error\r\n");
						while(1);
				}else{
					printf("write file[len = %u] success\r\n", bytesWrite);
					f_lseek(&file, 0);//必须添加，否则读不到

					res_flash = f_read(&file, buffer, sizeof(buffer), &bytesRead);
					if (res_flash == FR_OK){
						printf("read file[len = %u]>>>%s\r\n", bytesRead, buffer);
					}
				}
				f_sync(&file);
				res_flash = f_close(&file); 
				f_sync(&file);
				f_sync(&file);
				if(res_flash != FR_OK){
					printf("file_write_read_UT close file error[%d]\r\n", res_flash);
				}
		}else{
			printf("file_write_read_UT open file error\r\n");
		}
}

void file_read_UT(char *filename)
{
		FRESULT res_flash;
		uint32_t bytesRead = 0;

		res_flash = f_open(&file_r, filename, FA_OPEN_EXISTING | FA_READ);
		if(res_flash != FR_OK){
			printf("file_read_UT open file error[%d]\r\n", res_flash);
	  }else{
			res_flash = f_read(&file_r, buffer, sizeof(buffer), &bytesRead);
			if (res_flash == FR_OK){
				printf("read from file, len = %u>>>%s\r\n", bytesRead, buffer);
			}else{
				printf("read file error\r\n");
				while(1);
			}
			f_close(&file_r); 
		}
}

void flash_FAT_check_file_status(char *filename)
{
		FILINFO fno;
		FRESULT res_flash;
		res_flash = f_stat(filename, &fno);
		if(FR_OK == res_flash){
        printf("Size: %u\n", fno.fsize);
        printf("Timestamp: %u/%02u/%02u, %02u:%02u\r\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        printf("Attributes: %c%c%c%c%c\r\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
		}else{
				printf("%s is not exist[%d]\r\n", filename, res_flash);
		}
}

void flash_FAT_avail_size(char *path)
{
		FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    /* Get volume information and free clusters of drive 1 */
    uint8_t res = f_getfree(path, &fre_clust, &fs);
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    printf("%u KiB total drive space.%u KiB available.\r\n", tot_sect / 2, fre_sect / 2);
}

void flash_FAT_format(char *path)
{
		uint8_t res = f_mount(&fs,path,1);/**< 挂载文件系统, disk_name就是挂载的设备号为0的设备,1立即执行挂载*/	
		if(res==FR_NO_FILESYSTEM)/**< FR_NO_FILESYSTEM值为13,表示没有有效的设备*/
		{
			res = f_mkfs(path,0,work,sizeof(work));//格式化FLASH
			res = f_mount(NULL, path, 1);/**< 取消文件系统*/
      res = f_mount(&fs, path, 1);/**< 挂载文件系统*/
			if(res==0)
			{
				printf("format disk success\r\n");
			}else{
				printf("format disk error\r\n");   //格式化失败
			}
		}
		printf("mount disk success\r\n");
		flash_FAT_avail_size(path);
}

void test()
{
	UINT tempbw = 0;
	FRESULT res_flash;
	res_flash = f_opendir(&File1Dir, "1:");
	if(res_flash == FR_OK )//打开目录
	{
		res_flash = f_mkdir("1:/20200810");
		if(res_flash == FR_OK)//在当前目录下新建文件夹
		{   
			//f_chdir("1:/20200810");//改变当前工作目录 
			res_flash	 = f_open(&file,"1:/20200810/20200810.txt",FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			if(res_flash == FR_OK)
			{
				f_puts("WangChenchen 20200810 create succeed",&file);
				f_putc('Q',&file);
				f_write(&file,"FWRITEWCCWCC000123",18,&tempbw);
				f_close(&file);
			}else{
					printf("open \"1:/20200810/20200810.txt\" error[%d]", res_flash);
			}
		}else{
			printf("mkdir \"1:/20200810\" error[%d]", res_flash);
		}   		
	}else{
		printf("opendir \"1:\" error[%d]", res_flash);
	}
	flash_FAT_check_file_status("1:/20200810");
	flash_FAT_check_file_status("1:/20200810/20200810.txt");
}

void sdio_flash_UT(void)
{
		sd_erase(0 * 512, 1 * 512);
		printf("sdio_flash_UT>>>>\r\n");
		sd_block_write((uint32_t *)g_TestBuf1, FLASH_WRITE_ADDRESS, 512);
		sd_block_read((uint32_t *)g_TestBuf2, FLASH_READ_ADDRESS, 512);
		printf("sdio_flash_UT read :%s\r\n", g_TestBuf2);
}

void sdio_flash_init()
{
		sd_error_enum sd_error;
		uint16_t i = 5;
    do {
        /* initialize the card, get the card information and configurate the bus mode and transfer mode */
        sd_error = sd_config();
    } while((SD_OK != sd_error) && (--i));

    if(i) {
        printf("Card init success!\r\n");
    } else {
        printf("Card init failed!\r\n");
        while(1) {
        }
    }
}

void gpio_exit_ISR_init(void)
{
	  /* enable the Tamper key GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SYSCFG);

    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);

    /* enable and set key EXTI interrupt priority */
    nvic_irq_enable(EXTI10_15_IRQn, 7U, 0U);
    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN13);
    /* configure key EXTI line */
    exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_13);
	
		task_lock = os_sema_init();
}

#define bootloader_flag
int main(void)
{
		/* configure uart5 */
		gd_eval_com_init(EVAL_COM0);

#ifdef bootloader_flag
		//TT_ext_sdram_bootloader
		bootloader_run();
#else
	  {
			//TT_ext_sdram(code)+chip_ram(data)
			nvic_vector_table_set(0, 0);
		}

		Led_Init();
	
		gpio_exit_ISR_init();

		nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
		/* configure uart5 */
		gd_eval_com_init(EVAL_COM0);
	
		printf("################################\r\n");
#if DRV_SPI_SWITCH
		//spi flash 1
		//gd_eval_GD25Q40_Init();
		//gd_eval_GD25Q40_SectorErase(FLASH_WRITE_ADDRESS);
		//gd_eval_GD25Q40_BulkErase();//清空FLASH
		//spi_flash_UT1();
		//printf("FLASH ID = 0x%x\r\n", gd_eval_GD25Q40_ReadID());
		//printf("################################\r\n");
#else
		//spi flash 2
		spi_flash_init();
		//spi_flash_sector_erase(FLASH_WRITE_ADDRESS);
		spi_flash_bulk_erase();
		spi_flash_UT2();
		printf("FLASH ID = 0x%x\r\n", spi_flash_read_id());
		printf("################################\r\n");
#endif
		//sdio_flash_init();
		//sdio_flash_UT();
		//sd_erase(0, 65536 * 512);//清空SD
		printf("################################\r\n");

		//flash_FAT_format("0:");
		//file_write_read_UT("0:test1");
		//flash_FAT_avail_size("0:");
		//flash_FAT_check_file_status("0:test1");
		//f_mount(NULL, "0:", 1);
		//printf("*****************\n\r");
		//printf("*****************\n\r");
		//flash_FAT_format("1:");
		//file_write_read_UT("1:test2");
		//flash_FAT_avail_size("1:");
		//flash_FAT_check_file_status("1:test2");
		//printf("*****************\r\n");
		//test();
		//f_mount(NULL, "1:", 1);
		printf("################################\r\n");

		os_task_create(app_task1, "app_task1",2048,NULL,4,&task1_handle);// PSP
		os_task_create(app_task2, "app_task2",2048,NULL,4,&task2_handle);// PSP

		vTaskStartScheduler();
		while(1);
#endif
}
