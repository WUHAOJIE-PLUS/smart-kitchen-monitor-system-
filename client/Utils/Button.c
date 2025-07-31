#include "Button.h"                  


void Button_Init(uint32_t RCC_APB2Periph_GPIOx,GPIO_TypeDef* GPIOx,  uint16_t GPIO_Pin ){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOx, ENABLE);
	GPIO_InitTypeDef initType;
	initType.GPIO_Pin = GPIO_Pin;
	initType.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入: 默认高电平 (希望按键之后输入低电平)
	GPIO_Init(GPIOx, &initType);
	
}





