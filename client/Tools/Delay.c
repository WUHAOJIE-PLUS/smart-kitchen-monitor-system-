#include "Delay.h"

void Delay_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // 使能TIM2时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;// 定义结构体
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;   // 设定自动重装值(最大 65535)
    TIM_TimeBaseStructure.TIM_Prescaler = 71;    // 72MHz / (71+1) = 1MHz (1µs 计数增加一次)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;// TIM_CKD_DIV1标示定时器使用CK_CNT作为时钟
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);// 使用上面配置的参数, 初始化TIM2

    TIM_Cmd(TIM2, ENABLE);  // 使能TIM2
}

void Delay_us(uint16_t us) {
    TIM_SetCounter(TIM2, 0);  // 计数器清零

    // TIM_GetCounter()函数用于获取指定TIM定时器的当前计数值
    while (TIM_GetCounter(TIM2) < us);  // 等待计数器达到 us 值
}

void Delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        // 延时1ms = 1000us
        Delay_us(1000);
    }
}



