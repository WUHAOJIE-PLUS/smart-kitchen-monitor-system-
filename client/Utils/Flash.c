#include "FLASH.h"

// addr: 页首地址
void erasePage(uint32_t addr) {
    FLASH_Unlock();					//解锁
    FLASH_ErasePage(addr);			//页擦除
    FLASH_Lock();					//加锁
}

// addr: 页首地址
// data: 存放在首地址的32位数据
void changeFlashFor32(uint32_t addr, uint32_t data) {
    FLASH_Unlock();							// 解锁
    FLASH_ProgramWord(addr, data);			// 32bits
    FLASH_Lock();							// 加锁
}

// addr: 页首地址
// data: 存放在首地址的16位数据
void changeFlashFor16(uint32_t addr, uint16_t data) {
    FLASH_Unlock();							// 解锁
    FLASH_ProgramHalfWord(addr, data);		// 16bits
    FLASH_Lock();							// 加锁
}

// 读取数据
void readFlash(uint32_t addr, char *buffer, int len) {
    for (int i = 0; i < len; i++) {
        buffer[i] = *(__IO uint8_t *)(addr + i);
    }
}
