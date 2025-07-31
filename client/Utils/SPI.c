#include "SPI.h"


void SPI1_Init(void) {

    // 启用SPI1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置SPI1引脚(PA5=SCK时钟信号, PA6=MISO主机输入/从机输出, PA7=MOSI主机输出/从机输入)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置CS引脚(PA0)(表示是否选择使用当前PA0链接的设备)
    // CS拉低(0): 表示开始通信(从设备进入工作状态)
    // CS拉高(1): 表示结束通信(从设备进入空闲状态)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);// 先不进行片选

    // 配置SPI1
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // 双向全双工通信
    SPI_InitStructure.SPI_Mode =
        SPI_Mode_Master;  // 要求当前SPI片上外设主模式(表示当前设备是主机还是丛机)
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  // 8位数据(每次传输的数据位数)
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;  // 时钟空闲时候, 电平状态
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;  // 数据采样沿第几个边缘
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;  // 允许以:软件方式控制NSS
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  // 数据传输的波特率:72MHz/4=18MHz
    SPI_InitStructure.SPI_FirstBit =
        SPI_FirstBit_MSB;  // 在数据传输中,优先传输低bit位还是高bit位(类似大端小端)
    SPI_Init(SPI1, &SPI_InitStructure);

    // 配置stm32芯片自己的spi的NSS引脚
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);// 通过软件方式设置为非从机, 而非外接电压
    // 启用SPI1
    SPI_Cmd(SPI1, ENABLE);
}



// SPI发送/接收一个字节: 既是发送又是接收
uint8_t SPI1_ReadWriteByte(uint8_t TxData) {

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  // 等待发送缓冲区空

    SPI_I2S_SendData(SPI1, TxData);  // 发送数据

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  // 等待接收完成

    return SPI_I2S_ReceiveData(SPI1);  // 返回接收的数据

}

// W25Q64片选(低电平有效)
void W25Q64_CS_Enable(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}

// W25Q64片选释放（高电平无效）
void W25Q64_CS_Disable(void) {
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}


// 检查W25Q64是否忙
uint8_t W25Q64_Wait_Busy(void) {

    W25Q64_CS_Enable(); // 选中从设备
    SPI1_ReadWriteByte(0x05);  // 读状态寄存器

    uint8_t status;

    do {
        status = SPI1_ReadWriteByte(0xFF);  // 读取状态
    } while (status & 0x01);  // WIP=1表示忙

    W25Q64_CS_Disable();

    return status;
}

// 写入一个字节到指定地址
void W25Q64_Write(uint32_t WriteAddr, uint8_t Data) {
    W25Q64_Wait_Busy();  // 等待空闲
    W25Q64_CS_Enable();
    SPI1_ReadWriteByte(0x06);  // 写使能
    W25Q64_CS_Disable();

    W25Q64_CS_Enable();
    // W25Q64的存储空间被划分为多个页(Page), 每页大小为256字节(0x100)
    SPI1_ReadWriteByte(
        0x02);   // 发送页编程要求,开始页编程(其实就是允许主机向指定的地址写入数据)
    SPI1_ReadWriteByte((WriteAddr >> 16) & 0xFF);  // 开始发送地址(在该Flash中地址长度24bit): 地址高字节
    SPI1_ReadWriteByte((WriteAddr >> 8) & 0xFF);   // 地址中字节
    SPI1_ReadWriteByte(WriteAddr & 0xFF);          // 地址低字节
    SPI1_ReadWriteByte(Data);                      // 在发送的地址上:写入数据
    W25Q64_CS_Disable();
}


// 擦除扇区(最小也要按照扇区擦除)(为什么要擦除: 因为写入数据也要从1->写为->0)
// 在W25Q64中, Flash容量64Mbit即8M字节, 其中按照块划分(每块64K字节, 共128块)
// 一块又分16个扇区, 每个扇区4K字节
// 一个扇区分16页, 每页256字节
void W25Q64_Erase_Sector(uint32_t SectorAddr) {
    W25Q64_Wait_Busy(); // 等待从设备空闲
    W25Q64_CS_Enable(); // 选中从设备
    SPI1_ReadWriteByte(0x06);  // 发送写入数据要求
    W25Q64_CS_Disable();

    W25Q64_CS_Enable();
    SPI1_ReadWriteByte(0x20);  // 发送要求: 进行一个扇区的擦除
    SPI1_ReadWriteByte((SectorAddr >> 16) & 0xFF);  // 发送地址
    SPI1_ReadWriteByte((SectorAddr >> 8) & 0xFF);
    SPI1_ReadWriteByte(SectorAddr & 0xFF);
    W25Q64_CS_Disable();
}

// 读数据
uint8_t W25Q64_Read_Byte(uint32_t ReadAddr) {

    W25Q64_CS_Enable(); // 选中从设备
    SPI1_ReadWriteByte(0x03);  // 发送读数据要求
    SPI1_ReadWriteByte((ReadAddr >> 16) & 0xFF);// 发送地址信息
    SPI1_ReadWriteByte((ReadAddr >> 8) & 0xFF);
    SPI1_ReadWriteByte(ReadAddr & 0xFF);
    uint8_t Data = SPI1_ReadWriteByte(0xFF);  // 读前面发送的地址中的: 数据
    W25Q64_CS_Disable();
    return Data;
}


// 写入字符串
void W25Q64_Write_String(uint32_t WriteAddr, char *str, int len) {

    for (int i = 0; i < len; i++) {
        W25Q64_Write(WriteAddr++, str[i]);
        Delay_ms(1);
    }
}
