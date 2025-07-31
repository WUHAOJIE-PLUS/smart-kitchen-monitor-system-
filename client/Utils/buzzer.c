#include "buzzer.h"

void Buzzer_Init(void){
	// 开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 初始化引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//PA9:USART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}


void Buzzer_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);

}
void Buzzer_OFF(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
}
