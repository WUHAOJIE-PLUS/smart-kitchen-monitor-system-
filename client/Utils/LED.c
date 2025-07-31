#include "LED.h"
//B5,B6,B7,B8
//B5,B6分别代表白灯和红灯
//B7,B8分别代表蓝灯和黄灯
//光敏传感器：正常运行白灯亮，异常运行，红灯亮
//空气质量传感器：正常运行蓝灯亮，异常运行黄灯亮
void LED_Init(void){
	// 开启时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 初始化引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

//正常：B5亮，B6灭
void LED_LIGHT_NORMAL(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}
//异常：B6亮，B5灭
void LED_LIGHT_NONORMAL(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
}
//正常：B7亮，B8灭
void LED_AIR_NORMAL(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_7);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
}
//异常：B8亮，B7灭
void LED_AIR_NONORMAL(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	GPIO_SetBits(GPIOB,GPIO_Pin_7);
}
