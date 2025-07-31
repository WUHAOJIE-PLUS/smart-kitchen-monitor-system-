#ifndef __FLASH_H__
#define __FLASH_H__

#include "header.h"

// addr: 页首地址
void erasePage(uint32_t addr);

// addr: 页首地址
// data: 存放在首地址的32位数据
void changeFlashFor32(uint32_t addr, uint32_t data) ;


// addr: 页首地址
// data: 存放在首地址的16位数据
void changeFlashFor16(uint32_t addr, uint16_t data) ;


// 读取数据
void readFlash(uint32_t addr, char *buffer, int len) ;

#endif

