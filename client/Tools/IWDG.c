#include "IWDG.h"


void IWDG_Init() {
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  // 允许修改
    IWDG_SetPrescaler(IWDG_Prescaler_256);        // 设置预分频
    IWDG_SetReload(3124);  // 设置重装值
    IWDG_Enable();  // 启动 IWDG
}






