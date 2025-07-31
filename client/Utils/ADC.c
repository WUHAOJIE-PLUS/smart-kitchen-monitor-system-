//  ADC示例
#include "ADC.h"

//非连续模式（软件触发），非扫描模式（通过改变记录表的配置，做到扫描全部通道）
void AD_Init(void) {
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 设置ADC时钟: 选择时钟6分频(即: 72MHz/6 = 12MHz)
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // GPIO初始化
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Mode = GPIO_Mode_AIN;
    gpioInit.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_4; // 使用PA1和PA4采样模拟数据
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInit);

    // ADC初始化
    ADC_InitTypeDef ADC_InitStructure;
    // ADC模式选择: 暂时选择默认独立工作模式(即只使用ADC1)
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    // 数据对齐(和采样数据精度相关)
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    // 选择触发采集模式(不用外部硬件触发, 使用软件触发)
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    // 不进行连续转化, 不开启, 必须触发过来, 才进行采样
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // 不开启扫描模式, 因为我们准备每个组设置一个采样通道(通过设置, 采样, 修改设置, 采样的方式)
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    // 开启的通道数(给扫描模式指明扫描几个通过个数用的)
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    // 初始化
    ADC_Init(ADC1, &ADC_InitStructure);
    // 启动ADC
    ADC_Cmd(ADC1, ENABLE);

    // ADC校准: 提高ADC采样精度的固定硬件电路
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}


uint16_t AD_GetValue(uint8_t ADC_Channel) {
    // 在每次AD转换前配置规则组，这样可以灵活更改AD转换的通道
    // 第一个参数: 采用ADC1还是ADC2
    // 第二个参数: 使用那个通道, 比如ADC_Channel_1表示PA1通道; ADC_Channel_4表示PA4通道
    // 第三个参数: 规则组可以有最多16个通道，可以指定该通道在规则组中的采样顺序
    // 第四个参数: 对输入电压采样的时间长短; ADC_SampleTime_55Cycles5表示采样55.5个ADC时钟节拍
    ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);

    // 软件触发一次ADC1采样
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 等待采样结束: 采样是否结束, 高根据寄存器EOC标志位来判断
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    // 读取采样结果, 返回
    return ADC_GetConversionValue(ADC1);
}
