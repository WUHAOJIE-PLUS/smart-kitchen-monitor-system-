// UART1.h
#ifndef __UART1_H__
#define __UART1_H__

#include "header.h"


//PA9 USART1_TX
//PA10 USART1_RX

//PA3 USART2_RX
//PA2 USART2_TX

//PB10 USART2_TX
//PB11 USART2_RX

void USART1_Init(void);
void USART1_SendByte(uint8_t Byte);
void printf1(char *format, ...);
#endif

