#include "stm32f10x.h"
#include "OLED_Font.h"
#include "OLED.h"

/*
	引脚配置宏，若改变了SCL和SDA接入引脚需要修改
	当前SCL针脚接入PB10-PA11引脚
	当前SDA针脚接入PB11-PA12引脚
	建议按照文档中的接线图接线, 这样就无需修改这里！
*/
#define OLED_SCL_GPIO_PORT      GPIOA
#define OLED_SCL_GPIO_PIN       GPIO_Pin_11
#define OLED_SDA_GPIO_PORT      GPIOA
#define OLED_SDA_GPIO_PIN       GPIO_Pin_12

/* 引脚电平设置宏 */
#define OLED_W_SCL(x)    GPIO_WriteBit(OLED_SCL_GPIO_PORT, OLED_SCL_GPIO_PIN, (BitAction)(x))
#define OLED_W_SDA(x)    GPIO_WriteBit(OLED_SDA_GPIO_PORT, OLED_SDA_GPIO_PIN, (BitAction)(x))

/* 初始化I2C引脚 */
void OLED_I2C_Init(void) {
    // 使用不同的引脚，开启不同的GPIO时钟
    if (OLED_SCL_GPIO_PORT == GPIOA) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    } else if (OLED_SCL_GPIO_PORT == GPIOB) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    } else if (OLED_SCL_GPIO_PORT == GPIOC) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }

    // 根据宏定义配置SCL引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// 引脚设置为开漏输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = OLED_SCL_GPIO_PIN;
    GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStructure);

    // 根据宏定义配置SDA引脚
    GPIO_InitStructure.GPIO_Pin = OLED_SDA_GPIO_PIN;
    GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStructure);

    // 初始化引脚电平
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
static void OLED_I2C_Start(void) {
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
static void OLED_I2C_Stop(void) {
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
static void OLED_I2C_SendByte(uint8_t Byte) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
        OLED_W_SDA(!!(Byte & (0x80 >> i)));  // 发送每一位
        OLED_W_SCL(1);  // 拉高时钟
        OLED_W_SCL(0);  // 拉低时钟
    }

    OLED_W_SCL(1);  // 第9个时钟，用于接收ACK
    OLED_W_SCL(0);  // 拉低时钟
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
static void OLED_WriteCommand(uint8_t Command) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);		//从机地址
    OLED_I2C_SendByte(0x00);		//写命令
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
static void OLED_WriteData(uint8_t Data) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);		//从机地址
    OLED_I2C_SendByte(0x40);		//写数据
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X) {
    OLED_WriteCommand(0xB0 | Y);					//设置Y位置
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void) {
    uint8_t i, j;

    for (j = 0; j < 8; j++) {
        OLED_SetCursor(j, 0);

        for (i = 0; i < 128; i++) {
            OLED_WriteData(0x00);
        }
    }
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char) {
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分

    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
    }

    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分

    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
    }
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  * @node 字符串超出当前行后，支持自动换行
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String) {
    uint8_t i;
    uint8_t currentLine = Line;  // 当前行
    uint8_t currentColumn = Column;  // 当前列

    for (i = 0; String[i] != '\0'; i++) {
        // 如果当前列超出最大值（16），则换行
        if (currentColumn > 16) {
            currentLine++;  // 换到下一行
            currentColumn = 1;  // 重置列位置为起始列
        }

        // 如果当前行超出最大值（4），则停止显示
        if (currentLine > 4) {
            break;
        }

        // 显示字符
        OLED_ShowChar(currentLine, currentColumn, String[i]);
        currentColumn++;  // 列位置递增
    }
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y) {
    uint32_t Result = 1;

    while (Y--) {
        Result *= X;
    }

    return Result;
}

/**
  * @brief  OLED显示无符号整数（十进制，非负数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowUnsignedNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length) {
    uint8_t i;

    for (i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED显示有符号整数数（十进制）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  * @node 有符号数的长度不包括符号
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length) {
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0) {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    } else {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }

    for (i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED显示十六进制数字（正数，负数没表示需求）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  * @node 函数会自动在数字前加0x，但数字长度不包括这个0x
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length) {
    uint8_t i, SingleNumber;

    // 显示前缀 "0x"
    OLED_ShowChar(Line, Column, '0');
    OLED_ShowChar(Line, Column + 1, 'x');

    // 显示十六进制数字
    for (i = 0; i < Length; i++) {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;

        if (SingleNumber < 10) {
            OLED_ShowChar(Line, Column + 2 + i, SingleNumber + '0');
        } else {
            OLED_ShowChar(Line, Column + 2 + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
  * @brief  OLED显示二进制数字（正数，负数没表示需求）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length) {
    uint8_t i;

    for (i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
  * @brief  OLED显示浮点数（使用double类型，精度更大）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的浮点数
  * @param  IntLength 整数部分长度，范围：1~10
  * @param  FraLength 小数部分长度，范围：1~10
  * @node 小数部分长度小于实际长度时，会进行四舍五入
  */
void OLED_ShowDouble(uint8_t Line, uint8_t Column, double Number, uint8_t IntLength, uint8_t FraLength) {
    uint32_t IntPart = (uint32_t)Number;  // 提取整数部分
    double FraPart = Number - IntPart;    // 提取小数部分

    // 对小数部分进行四舍五入
    FraPart = FraPart * OLED_Pow(10, FraLength) + 0.5;  // 四舍五入
    uint32_t RoundedFraPart = (uint32_t)FraPart;        // 取整

    // 显示整数部分
    OLED_ShowUnsignedNum(Line, Column, IntPart, IntLength);

    // 显示小数点
    OLED_ShowChar(Line, Column + IntLength, '.');

    // 显示小数部分
    OLED_ShowUnsignedNum(Line, Column + IntLength + 1, RoundedFraPart, FraLength);
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void) {
    uint32_t i, j;

    for (i = 0; i < 1000; i++) {		//上电延时
        for (j = 0; j < 1000; j++);
    }

    OLED_I2C_Init();			//端口初始化

    OLED_WriteCommand(0xAE);	//关闭显示

    OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8);	//设置多路复用率
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3);	//设置显示偏移
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40);	//设置显示开始行

    OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置

    OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

    OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);	//设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);	//设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

    OLED_WriteCommand(0xA6);	//设置正常/倒转显示

    OLED_WriteCommand(0x8D);	//设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);	//开启显示

    OLED_Clear();				//OLED清屏
}
