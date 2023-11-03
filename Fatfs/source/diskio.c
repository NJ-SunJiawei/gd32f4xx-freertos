/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "os_api.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

//add by sjw
#define EX_FLASH 0	                  //�ⲿspi flash,���Ϊ0
#define EX_SD    1	                  //�ⲿsd flash,���Ϊ1

#define FLASH_SECTOR_SIZE 	 FF_MAX_SS   ////4096
#define FLASH_SECTOR_COUNT   1024
#define FLASH_BLOCK_SIZE   	 1     	   //������������С����(������Ϊ��λ)

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	uint8_t res=0;	    
	switch(pdrv)
	{
		
		case EX_FLASH://�ⲿflash
#if DRV_SPI_SWITCH
			gd_eval_GD25Q40_Init();
#else
			spi_flash_init();
#endif
 			break;
		case EX_SD://�ⲿflash
			sd_config();
 			break;
		default:
			res=1; 
	}		 
	if(res) return STA_NOINIT;
	else return 0; //��ʼ���ɹ�
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint8_t res=0; 
  if (!count) return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(pdrv)
	{
		case EX_FLASH://�ⲿflash
			for(;count>0;count--)
			{
#if DRV_SPI_SWITCH
				gd_eval_GD25Q40_BufferRead((uint8_t *)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
#else
				spi_flash_buffer_read((uint8_t *)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
#endif
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
		case EX_SD://�ⲿflash
			if(count>1)
			{
					sd_multiblocks_read((uint32_t *)buff,(uint32_t)sector<<9,512,(uint32_t)count);//?��??ector��??��2������
			}else
			{
					sd_block_read((uint32_t *)buff,(uint32_t)sector<<9,512);
			}
			res=0;
			break;
		default:
			res=1; 
	}
   //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res==0)return RES_OK;	 
    else return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint8_t res=0;  
  if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(pdrv)
	{
		case EX_FLASH://�ⲿflash
			for(;count>0;count--)
			{
#if DRV_SPI_SWITCH				
				gd_eval_GD25Q40_BufferWrite((uint8_t *)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
#else
				spi_flash_buffer_write((uint8_t *)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
#endif
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
		case EX_SD://�ⲿflash
			if(count>1)
			{
					sd_multiblocks_write((uint32_t *)buff,(uint32_t)sector<<9,512,(uint32_t)count);//?��??ector��??��2������
			}else
			{
					sd_block_write((uint32_t *)buff,(uint32_t)sector<<9,512);
			}
			res=0;
			break;
		default:
			res=1; 
	}
	//������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
	if(res == 0)return RES_OK;	 
	else return RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  DRESULT res;						  			     
	if(pdrv==EX_FLASH)	//�ⲿFLASH  
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
						res = RES_OK;                       //ͬ������
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(DWORD*)buff = FLASH_SECTOR_SIZE;  //����������С
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(DWORD*)buff = FLASH_BLOCK_SIZE;    //���ز������С
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;  //������������
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else if(pdrv==EX_SD){
	    switch(cmd)
	    {
		    case CTRL_SYNC:
						res = RES_OK;           //ͬ������
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(DWORD*)buff = 512;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(DWORD*)buff = 8;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = 65536; //32G
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else{
		 res=RES_ERROR;//�����Ĳ�֧��
	}
  return res;
}

DWORD get_fattime (void)
{				 
	return RES_OK;
}
