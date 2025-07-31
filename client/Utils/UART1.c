#include "UART1.h"

#define MAXSIZE_USART1 20
#define MAXSIZE_USART2 300 //空间一定要大，不然ESP01S发送给STM32的数据在USART2ISR中会产生地址越界，执行不到"\r\nOK\r\n" or "ERROR\r\n",导致USART2ISR卡死


static char str1[MAXSIZE_USART1];
static uint32_t Subscript1 = 0;



//PC端口
void USART1_Init() {
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置 GPIOA9 为 TX
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 GPIOA10 为 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 USART1
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    // 启动 USART1
    USART_Cmd(USART1, ENABLE);

    // 开启中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // NVIC 配置
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 12;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);


}



/*
USART_TypeDef* USARTx:USART1 or USART2
*/
void USART1_ClearRDR() {
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
        USART_ReceiveData(USART1); // 读取但不存储
    }		//类似C语言的while(getchar());
}

/*
USART_TypeDef* USARTx:USART1 or USART2
*/
// 发送字节
void USART1_SendByte(uint8_t Byte) {
    USART_SendData(USART1, Byte);

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

//PC
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
        uint8_t temp = (uint8_t)USART_ReceiveData(USART1);

        if (Subscript1 < MAXSIZE_USART1 - 1) {
            str1[Subscript1++] = temp;
        }

        // 判断是否遇到 \r\n，作为结束标志
        if (Subscript1 >= 2 &&
                str1[Subscript1 - 2] == '\r' &&
                str1[Subscript1 - 1] == '\n') {

            str1[Subscript1] = '\0';  // 加字符串终止符
            printf2("%s", str1);//在esp01s透传模式时，会传回来打印过的数据TODO：

            Subscript1 = 0;
            memset(str1, 0, sizeof(str1));
            USART1_ClearRDR();
        }
    }
}



/*
SemaphoreHandle_t semaphoreX:semaphore1 or semaphore2
USART_TypeDef* USARTx:USART1 or USART2
*/
//pr中SemaphoreHandle_t semaphoreX不需要指针，不需要从函数中取内容
void printf1(char *format, ...) {
    
    char strs[100];
    va_list list;
    va_start(list, format);
    vsprintf(strs, format, list);
    va_end(list);
     
    for (uint8_t i = 0; strs[i] != '\0'; i++) {
        USART1_SendByte(strs[i]);
    }

   
}



