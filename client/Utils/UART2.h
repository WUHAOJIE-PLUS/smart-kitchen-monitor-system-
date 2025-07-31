// UART2.h
#ifndef __UART2_H__
#define __UART2_H__

#include "header.h"

//PA9 USART1_TX
//PA10 USART1_RX

//PA3 USART2_RX
//PA2 USART2_TX

//PB10 USART2_TX
//PB11 USART2_RX
extern QueueHandle_t queue_esp01s;

void USART2_Init(void);
void USART2_SendByte(uint8_t Byte);
void printf2(char *format, ...);
#endif

