#ifndef __gd_eval_GD25Q40_H__
#define __gd_eval_GD25Q40_H__

#include "gd32f4xx.h"
#include <stdio.h>

extern void gd_eval_GD25Q40_Init(void);
extern void gd_eval_GD25Q40_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
extern void gd_eval_GD25Q40_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
extern void gd_eval_GD25Q40_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
extern void gd_eval_GD25Q40_BulkErase(void);
extern void gd_eval_GD25Q40_SectorErase(uint32_t SectorAddr);
extern uint32_t gd_eval_GD25Q40_ReadID(void);

#endif

