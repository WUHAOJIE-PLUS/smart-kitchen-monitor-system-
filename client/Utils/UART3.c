#include "UART3.h"


//HC05
void USART3_Init() {
    // 开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  // USART3 用 APB1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // GPIOB 仍用 APB2

    // 配置 GPIOB10 为 TX
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 GPIOB11 为 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 USART2sssss
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStructure);

    // 启动 USART2
    USART_Cmd(USART3, ENABLE);

    // 开启中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // NVIC 配置
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 12;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);


}

/*
USART_TypeDef* USARTx:USART1 or USART2
*/
void USART3_ClearRDR() {
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
        USART_ReceiveData(USART3); // 读取但不存储
    }		//类似C语言的while(getchar());
}

/*
USART_TypeDef* USARTx:USART1 or USART2
*/
// 发送字节
void USART3_SendByte(uint8_t Byte) {
    USART_SendData(USART3, Byte);

    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void USART3_IRQHandler(void) {
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        uint8_t temp = (uint8_t)USART_ReceiveData(USART3);
		
		BaseType_t stat = pdTRUE;
		xQueueSendFromISR(queue_hc05,&temp, &stat);

		USART_ClearITPendingBit(USART3, USART_IT_RXNE);

   }
}

/*
SemaphoreHandle_t semaphoreX:semaphore1 or semaphore2
USART_TypeDef* USARTx:USART1 or USART2
*/
//pr中SemaphoreHandle_t semaphoreX不需要指针，不需要从函数中取内容
void printf3(char *format, ...) {
   
    char strs[100];
    va_list list;
    va_start(list, format);
    vsprintf(strs, format, list);
    va_end(list);
     
    for (uint8_t i = 0; strs[i] != '\0'; i++) {
        USART3_SendByte(strs[i]);
    }
}



