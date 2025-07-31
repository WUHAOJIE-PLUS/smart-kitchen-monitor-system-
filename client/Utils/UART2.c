#include "UART2.h"

//esp01s
void USART2_Init() {
    // 开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // USART2 用 APB1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // GPIOA 仍用 APB2

    // 配置 GPIOA2 为 TX
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 GPIOA3 为 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 USART2
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    // 启动 USART2
    USART_Cmd(USART2, ENABLE);

    // 开启中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // NVIC 配置
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 12;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

   
}


/*
USART_TypeDef* USARTx:USART1 or USART2
*/
void USART2_ClearRDR() {
    while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET) {
        USART_ReceiveData(USART2); // 读取但不存储
    }		//类似C语言的while(getchar());
}

/*
USART_TypeDef* USARTx:USART1 or USART2
*/
// 发送字节
void USART2_SendByte(uint8_t Byte) {
    USART_SendData(USART2, Byte);

    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}



//void USART2_IRQHandler(void) {
//    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
//        uint8_t temp = (uint8_t)USART_ReceiveData(USART2);
//		
//		//超时踢出
//		if(temp == '$')
//		{
//			TCP_Release();
//			flag = 0;
//		}
//        if (Subscript2 < MAXSIZE_USART2 - 1) {
//            str2[Subscript2++] = temp;
//        }

//        // 判断是否接收到完整结尾：以 \r\nOK\r\n 结尾（完整应答）
//        if (Subscript2 >= 6 && strncmp(&str2[Subscript2 - 6], "\r\nOK\r\n", 6) == 0) {

//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            USART2_ClearRDR();

//            // 清空缓冲
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        }

//        // 也可以添加 ERROR 响应处理（选做）
//        else if (Subscript2 >= 7 &&
//                 strncmp(&str2[Subscript2 - 7], "ERROR\r\n", 7) == 0) {
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            USART2_ClearRDR();
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        } else if (Subscript2 == MAXSIZE_USART2 - 1) {
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        } else { //当打印AT+CWLAP时，数组空间不够，所以需要发几次，当执行玩上面的if后，有概率让OK or ERROR拦腰截断在两个数组，导致不能进入上三个if语句
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        }
//    }
//}


void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
        uint8_t temp = (uint8_t)USART_ReceiveData(USART2);
		
		BaseType_t stat = pdTRUE;
		xQueueSendFromISR(queue_esp01s,&temp, &stat);
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
//		//超时踢出
//		if(temp == '$')
//		{ 
//			TCP_Release();
//			flag_time = 0;
//		}
//        if (Subscript2 < MAXSIZE_USART2 - 1) {
//            str2[Subscript2++] = temp;
//        }

//        // 判断是否接收到完整结尾：以 \r\nOK\r\n 结尾（完整应答）
//        if (Subscript2 >= 6 && strncmp(&str2[Subscript2 - 6], "\r\nOK\r\n", 6) == 0) {

//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            USART2_ClearRDR();

//            // 清空缓冲
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        }

//        // 也可以添加 ERROR 响应处理（选做）
//        else if (Subscript2 >= 7 &&
//                 strncmp(&str2[Subscript2 - 7], "ERROR\r\n", 7) == 0) {
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            USART2_ClearRDR();
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        } else if (Subscript2 == MAXSIZE_USART2 - 1) {
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        } else { //当打印AT+CWLAP时，数组空间不够，所以需要发几次，当执行玩上面的if后，有概率让OK or ERROR拦腰截断在两个数组，导致不能进入上三个if语句
//            str2[Subscript2] = '\0';
//            printf1("%s", str2);
//            Subscript2 = 0;
//            memset(str2, 0, sizeof(str2));
//        }
    }
}
/*
SemaphoreHandle_t semaphoreX:semaphore1 or semaphore2
USART_TypeDef* USARTx:USART1 or USART2
*/
//pr中SemaphoreHandle_t semaphoreX不需要指针，不需要从函数中取内容
void printf2(char *format, ...) {

    char strs[200];
    va_list list;
    va_start(list, format);
    vsprintf(strs, format, list);
    va_end(list);
     
    for (uint8_t i = 0; strs[i] != '\0'; i++) {
        USART2_SendByte(strs[i]);
    }


}



