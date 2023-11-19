/*!
    \file    gd32f450i_eval.c
    \brief   firmware functions to manage leds, keys, COM ports
    
    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

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
#include "gd32f450i_eval.h"

/* private variables */
static uint32_t GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT,
                                   LED3_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN};

static rcu_periph_enum COM_CLK[COMn] = {EVAL_COM0_CLK, EVAL_COM1_CLK};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {EVAL_COM0_GPIO_CLK, EVAL_COM1_GPIO_CLK};
static uint32_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN, EVAL_COM1_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN, EVAL_COM1_TX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {EVAL_COM0_GPIO_PORT, EVAL_COM1_GPIO_PORT};
static uint32_t COM_AF[COMn] = {EVAL_COM0_AF, EVAL_COM1_AF};

static rcu_periph_enum GPIO_CLK[LEDn] = {LED1_GPIO_CLK, LED2_GPIO_CLK, 
                                         LED3_GPIO_CLK};

static uint32_t KEY_PORT[KEYn] = {WAKEUP_KEY_GPIO_PORT, 
                                  TAMPER_KEY_GPIO_PORT,
                                  USER_KEY_GPIO_PORT};
static uint32_t KEY_PIN[KEYn] = {WAKEUP_KEY_PIN, TAMPER_KEY_PIN,USER_KEY_PIN};
static rcu_periph_enum KEY_CLK[KEYn] = {WAKEUP_KEY_GPIO_CLK, 
                                        TAMPER_KEY_GPIO_CLK,
                                        USER_KEY_GPIO_CLK};
static exti_line_enum KEY_EXTI_LINE[KEYn] = {WAKEUP_KEY_EXTI_LINE,
                                             TAMPER_KEY_EXTI_LINE,
                                             USER_KEY_EXTI_LINE};
static uint8_t KEY_PORT_SOURCE[KEYn] = {WAKEUP_KEY_EXTI_PORT_SOURCE,
                                        TAMPER_KEY_EXTI_PORT_SOURCE,
                                        USER_KEY_EXTI_PORT_SOURCE};
static uint8_t KEY_PIN_SOURCE[KEYn] = {WAKEUP_KEY_EXTI_PIN_SOURCE,
                                       TAMPER_KEY_EXTI_PIN_SOURCE,
                                       USER_KEY_EXTI_PIN_SOURCE};
static uint8_t KEY_IRQn[KEYn] = {WAKEUP_KEY_EXTI_IRQn, 
                                 TAMPER_KEY_EXTI_IRQn,
                                 USER_KEY_EXTI_IRQn};

/*!
    \brief    configure led GPIO
    \param[in]  lednum: specify the Led to be configured
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void  gd_eval_led_init (led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    /* configure led GPIO port */ 
    gpio_mode_set(GPIO_PORT[lednum], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN[lednum]);
    gpio_output_options_set(GPIO_PORT[lednum], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);

    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    turn on selected led
    \param[in]  lednum: specify the Led to be turned on
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void gd_eval_led_on(led_typedef_enum lednum)
{
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    turn off selected led
    \param[in]  lednum: specify the Led to be turned off
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void gd_eval_led_off(led_typedef_enum lednum)
{
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    toggle selected led
    \param[in]  lednum: specify the Led to be toggled
      \arg        LED1
      \arg        LED2
      \arg        LED3
    \param[out] none
    \retval     none
*/
void gd_eval_led_toggle(led_typedef_enum lednum)
{
    GPIO_TG(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief    configure key
    \param[in]  key_num: specify the key to be configured
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER: user key
    \param[in]  key_mode: specify button mode
      \arg        KEY_MODE_GPIO: key will be used as simple IO
      \arg        KEY_MODE_EXTI: key will be connected to EXTI line with interrupt
    \param[out] none
    \retval     none
*/
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
    gpio_mode_set(KEY_PORT[key_num], GPIO_MODE_INPUT, GPIO_PUPD_NONE,KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 2U, 0U);

        /* connect key EXTI line to key GPIO pin */
        syscfg_exti_line_config(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }
}

/*!
    \brief    return the selected button state
    \param[in]  button: specify the button to be checked
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER: user key
    \param[out] none
    \retval     the button GPIO pin value
*/
uint8_t gd_eval_key_state_get(key_typedef_enum button)
{
    return gpio_input_bit_get(KEY_PORT[button], KEY_PIN[button]);
}

/*!
    \brief    configure COM port
    \param[in]  COM: COM on the board
      \arg        EVAL_COM0: COM on the board
    \param[out] none
    \retval     none
*/
void gd_eval_com_init(uint32_t com)
{
    /* enable GPIO clock */
    uint32_t COM_ID = 0;
    if(EVAL_COM0 == com){
        COM_ID = 0U;
    }else if(EVAL_COM1 == com){
				COM_ID = 1U;
		}

    rcu_periph_clock_enable(COM_GPIO_CLK[COM_ID]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[COM_ID]);

    /* connect port to USARTx_Tx */
    gpio_af_set(COM_GPIO_PORT[COM_ID], COM_AF[COM_ID], COM_TX_PIN[COM_ID]);

    /* connect port to USARTx_Rx */
    gpio_af_set(COM_GPIO_PORT[COM_ID], COM_AF[COM_ID], COM_RX_PIN[COM_ID]);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(COM_GPIO_PORT[COM_ID], GPIO_MODE_AF, GPIO_PUPD_PULLUP,COM_TX_PIN[COM_ID]);
    gpio_output_options_set(COM_GPIO_PORT[COM_ID], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,COM_TX_PIN[COM_ID]);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(COM_GPIO_PORT[COM_ID], GPIO_MODE_AF, GPIO_PUPD_PULLUP,COM_RX_PIN[COM_ID]);
    gpio_output_options_set(COM_GPIO_PORT[COM_ID], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,COM_RX_PIN[COM_ID]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com,115200U);//115200U
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}

//#define NOT_USE_MICROLIB 
#ifdef NOT_USE_MICROLIB
#pragma import(__use_no_semihosting)
		struct __FILE { 
						int handle; 
		};

FILE __stdout;

void _sys_exit(int x) 
{ 
    x = x; 
}
#endif

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
	usart_data_transmit(EVAL_COM0, (uint8_t)ch);
	while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
	return ch;
}

//scanf
int fgetc(FILE *f)
{
	return usart_data_receive(EVAL_COM1);
}

sd_card_info_struct sd_cardinfo;/* information of SD card */

sd_error_enum sd_config(void)
{
    sd_error_enum status = SD_OK;
    uint32_t cardstate = 0;
    /* initialize the card */
    status = sd_init();
    if(SD_OK == status) {
        status = sd_card_information_get(&sd_cardinfo);
    }
    if(SD_OK == status) {
        status = sd_card_select_deselect(sd_cardinfo.card_rca);
    }
    status = sd_cardstatus_get(&cardstate);
    if(cardstate & 0x02000000) {
        printf("\r\n The card is locked!");
#if 0
        /* unlock the card if necessary */
        status = sd_lock_unlock(SD_UNLOCK);
        if(SD_OK != status) {
            printf("\r\n Unlock failed!");
            while(1) {
            }
        } else {
            printf("\r\n The card is unlocked! Please reset MCU!");
        }
#endif
        while(1) {
        }
    }
    if((SD_OK == status) && (!(cardstate & 0x02000000))) {
        /* set bus mode */
        status = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
//        status = sd_bus_mode_config( SDIO_BUSMODE_1BIT );
    }
    if(SD_OK == status) {
        /* set data transfer mode */
//        status = sd_transfer_mode_config( SD_DMA_MODE );
        status = sd_transfer_mode_config(SD_POLLING_MODE);
    }
    return status;
}

typedef void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress = 0;
#define ApplicationAddress    0xC0000000 //sdram start

#define FLASH_READ_ADDRESS    0x08004000//app start
#define SDRAM_WRITE_READ_ADDR 0x00000000//sdram start
#define BUFFER_SIZE           0x01
#define SRAM_SIZE             (BUFFER_SIZE*4)
uint32_t addr = 0x00000000;
int32_t rxbuffer[BUFFER_SIZE];

void bootloader_run(void)
{
		exmc_synchronous_dynamic_ram_init(EXMC_SDRAM_DEVICE0);
    printf("SDRAM initialized!\r\n");
    delay_nop(10);
		//fmc_erase_sector_by_address(FLASH_READ_ADDRESS);
	  // 0 16K(0x4000)
		//fmc_erase_sector(1);//16K
		//fmc_erase_sector(2);//16K
		//fmc_erase_sector(3);//16K
		//fmc_erase_sector(4);//64K
		//fmc_erase_sector(5);//128K
		
		//只需要搬向量表
		while(addr < 0x000001ac){
				memset(rxbuffer, 0x00, BUFFER_SIZE);
				// read a block of data from the flash to rx_buffer
				fmc_read_32bit_data(FLASH_READ_ADDRESS + addr, BUFFER_SIZE, rxbuffer);
				sdram_writebuffer_8(EXMC_SDRAM_DEVICE0, (uint8_t *)rxbuffer, SDRAM_WRITE_READ_ADDR + addr, SRAM_SIZE);
				addr += SRAM_SIZE;
				//printf("next start addr:0x%08x\r\n", (FLASH_READ_ADDRESS + addr));
		}
		printf("move success\r\n");
		
		rcu_periph_clock_enable(RCU_SYSCFG);
		//SDRAM bank0 of EXMC (0xC0000000~0xC7FFFFFF) is mapped at address 0x00000000
		syscfg_bootmode_config(SYSCFG_BOOTMODE_EXMC_SDRAM);
	
		//暂时屏蔽
		//if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FF00000 ) == 0x00000000) //ApplicationAddress为新程序的起始地址，检查栈顶地址是否合法
		{
				JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);               //用户代码区第二个字存储为新程序起始地址（新程序复位向量指针）
				Jump_To_Application = (pFunction) JumpAddress;                          
				__set_MSP(*(__IO uint32_t*) ApplicationAddress);                        //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
				Jump_To_Application();                                                  // 设置PC指针为新程序复位中断函数的地址
		}
		printf("boot success\r\n");
}
