// UART3.h
#ifndef __UART3_H__
#define __UART3_H__

#include "header.h"

//PA9 USART1_TX
//PA10 USART1_RX

//PA3 USART2_RX
//PA2 USART2_TX

//PB10 USART2_TX
//PB11 USART2_RX


extern QueueHandle_t queue_hc05;

void USART3_Init(void);
void USART3_SendByte(uint8_t Byte);
void printf3(char *format, ...);
#endif

