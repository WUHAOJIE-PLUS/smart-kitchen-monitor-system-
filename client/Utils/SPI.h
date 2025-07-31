#ifndef __SPI_H__
#define __SPI_H__

#include "header.h"


void SPI1_Init(void) ;

// SPI发送/接收一个字节: 既是发送又是接收
uint8_t SPI1_ReadWriteByte(uint8_t TxData) ;


// W25Q64片选(低电平有效)
void W25Q64_CS_Enable(void) ;


// W25Q64片选释放（高电平无效）
void W25Q64_CS_Disable(void) ;


// 检查W25Q64是否忙
uint8_t W25Q64_Wait_Busy(void) ;

// 写入一个字节到指定地址
void W25Q64_Write(uint32_t WriteAddr, uint8_t Data) ;

// 擦除扇区(最小也要按照扇区擦除)(为什么要擦除: 因为写入数据也要从1->写为->0)
// 在W25Q64中, Flash容量64Mbit即8M字节, 其中按照块划分(每块64K字节, 共128块)
// 一块又分16个扇区, 每个扇区4K字节
// 一个扇区分16页, 每页256字节
void W25Q64_Erase_Sector(uint32_t SectorAddr) ;


// 读数据
uint8_t W25Q64_Read_Byte(uint32_t ReadAddr) ;


// 写入字符串
void W25Q64_Write_String(uint32_t WriteAddr, char *str, int len) ;

#endif

