#include "drv_gd25q40.h"

#if 0 /* 软件模拟 */
//spi0
#define SPI_SCK_HIGH  gpio_bit_set(GPIOB, GPIO_PIN_3);
#define SPI_SCK_LOW   gpio_bit_reset(GPIOB, GPIO_PIN_3);
#define SPI_MOSI_HIGH gpio_bit_set(GPIOB, GPIO_PIN_5);
#define SPI_MOSI_LOW  gpio_bit_reset(GPIOB, GPIO_PIN_5);
#define SPI_MISO_READ gpio_input_bit_get(GPIOB, GPIO_PIN_4);

static void SPI_Delay( uint32_t n)
{
    uint8_t i;

    while(n--)
    {
        /* 1us */
        for(i = 0; i < 50; i++)
        {
            __asm("NOP");
        }
    }
}

uint8_t gd_eval_SPI_SwapByte( uint8_t byte)
{
    uint8_t i = 0;
    uint8_t inDate = byte;
    uint8_t outBit = 0;
    uint8_t outDate = 0;
    
    /* SCKPL = 0; SCKPH = 0 */
    for (i = 0; i < 8; i++)
    {
        if (inDate & 0x80)
        {
            SPI_MOSI_HIGH;
        }
        else
        {
            SPI_MOSI_LOW;
        }
        
        SPI_Delay(5);
        SPI_SCK_HIGH;
        outBit = SPI_MISO_READ;
        if (outBit)
        {
            outDate |= 0x1;
        }
        SPI_Delay(5);
        SPI_SCK_LOW;
        SPI_Delay(5);
        inDate <<= 1;
        if (i <7)
        {
            outDate <<= 1;
        }
    }

    return outDate;
}

void gd_eval_SPI_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOB);
	
		gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3|GPIO_PIN_5);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3|GPIO_PIN_5);	

		gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);	
	
		SPI_SCK_LOW; /* SCKPL = 0; SCKPH = 0 */
}

#else /* 硬件方式 */
uint8_t gd_eval_SPI_SwapByte( uint8_t byte)
{
    while (spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);

    spi_i2s_data_transmit(SPI0, byte);

    while (spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) == RESET);

    return spi_i2s_data_receive(SPI0);
}

static void SPI_Configuration(void)
{
    spi_parameter_struct  SPI_InitStructure; 
    
    SPI_InitStructure.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    SPI_InitStructure.device_mode = SPI_MASTER;
    SPI_InitStructure.frame_size = SPI_FRAMESIZE_8BIT;
    SPI_InitStructure.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    SPI_InitStructure.nss = SPI_NSS_SOFT;
    SPI_InitStructure.prescale = SPI_PSC_2;
    SPI_InitStructure.endian = SPI_ENDIAN_MSB;

		spi_i2s_deinit(SPI0);
    spi_init(SPI0, &SPI_InitStructure);
    spi_enable(SPI0);      
}

static void SPI_GpioConfig(void)
{
		rcu_periph_clock_enable(RCU_GPIOB);
		rcu_periph_clock_enable(RCU_SPI0);
		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);	
}

void gd_eval_SPI_Init(void)
{
    SPI_GpioConfig();
    SPI_Configuration();
}
#endif

#define  GD25Q40_PageSize    0x100

#define WRITE      0x02  /* Write to Memory instruction */
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0x20  /* Sector Erase instruction */
#define BE         0xC7  /* Bulk Erase instruction */        

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */
#define Dummy_Byte 0xA5

#define  GD25Q40_CS_LOW()    gpio_bit_reset(GPIOE, GPIO_PIN_2)
#define  GD25Q40_CS_HIGH()   gpio_bit_set(GPIOE, GPIO_PIN_2)

static void GD25Q40_WriteEnable(void)
{
    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(WREN);
    GD25Q40_CS_HIGH();
}

static void GD25Q40_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;

    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(RDSR);

    do
    {
        FLASH_Status = gd_eval_SPI_SwapByte(Dummy_Byte);
    }
    while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

    GD25Q40_CS_HIGH();
}

void gd_eval_GD25Q40_SectorErase(uint32_t SectorAddr)
{
    GD25Q40_WriteEnable();

    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(SE);
    gd_eval_SPI_SwapByte((SectorAddr & 0xFF0000) >> 16);
    gd_eval_SPI_SwapByte((SectorAddr & 0xFF00) >> 8);
    gd_eval_SPI_SwapByte(SectorAddr & 0xFF);
    GD25Q40_CS_HIGH();

    GD25Q40_WaitForWriteEnd();
}

void gd_eval_GD25Q40_BulkErase(void)
{
    GD25Q40_WriteEnable();

    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(BE);
    GD25Q40_CS_HIGH();

    GD25Q40_WaitForWriteEnd();
}

void gd_eval_GD25Q40_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(READ);
    gd_eval_SPI_SwapByte((ReadAddr & 0xFF0000) >> 16);
    gd_eval_SPI_SwapByte((ReadAddr& 0xFF00) >> 8);
    gd_eval_SPI_SwapByte(ReadAddr & 0xFF);
    while (NumByteToRead--) 
    {
        *pBuffer = gd_eval_SPI_SwapByte(Dummy_Byte);
        pBuffer++;
    }
    GD25Q40_CS_HIGH();
}

void gd_eval_GD25Q40_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    GD25Q40_WriteEnable();

    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(WRITE);
    gd_eval_SPI_SwapByte((WriteAddr & 0xFF0000) >> 16);
    gd_eval_SPI_SwapByte((WriteAddr & 0xFF00) >> 8);
    gd_eval_SPI_SwapByte(WriteAddr & 0xFF);

    while (NumByteToWrite--)
    {
        gd_eval_SPI_SwapByte(*pBuffer);
        pBuffer++;
    }
    GD25Q40_CS_HIGH();

    GD25Q40_WaitForWriteEnd();
}

void gd_eval_GD25Q40_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = WriteAddr % GD25Q40_PageSize;
    count = GD25Q40_PageSize - Addr;
    NumOfPage =  NumByteToWrite / GD25Q40_PageSize;
    NumOfSingle = NumByteToWrite % GD25Q40_PageSize;
     /* WriteAddr is GD25Q40_PageSize aligned  */
    if (Addr == 0)
    {   
        /* NumByteToWrite < GD25Q40_PageSize */
        if (NumOfPage == 0) 
        {
            gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > GD25Q40_PageSize */
        {
            while (NumOfPage--)
            {
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, GD25Q40_PageSize);
                WriteAddr +=  GD25Q40_PageSize;
                pBuffer += GD25Q40_PageSize;
            }
            gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else /* WriteAddr is not GD25Q40_PageSize aligned  */
    {
        if (NumOfPage == 0)
        {
            /* (NumByteToWrite + WriteAddr) > GD25Q40_PageSize */
            if (NumOfSingle > count) 
            {
                temp = NumOfSingle - count;
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, temp);
            }
            else
            {
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > GD25Q40_PageSize */
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / GD25Q40_PageSize;
            NumOfSingle = NumByteToWrite % GD25Q40_PageSize;

            gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--)
            {
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, GD25Q40_PageSize);
                WriteAddr +=  GD25Q40_PageSize;
                pBuffer += GD25Q40_PageSize;
            }

            if (NumOfSingle != 0)
            {
                gd_eval_GD25Q40_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}

uint32_t gd_eval_GD25Q40_ReadID(void)
{
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

    GD25Q40_CS_LOW();
    gd_eval_SPI_SwapByte(0x9F);
    Temp0 = gd_eval_SPI_SwapByte(Dummy_Byte);
    Temp1 = gd_eval_SPI_SwapByte(Dummy_Byte);
    Temp2 = gd_eval_SPI_SwapByte(Dummy_Byte);
    GD25Q40_CS_HIGH();

    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}

void gd_eval_GD25Q40_Init(void)
{
    gd_eval_SPI_Init();
    rcu_periph_clock_enable(RCU_GPIOE);

    gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);	
}

