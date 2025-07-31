#include "header.h"

#define AIR_LIMIT 350
#define LIGHT_LIMIT 3000
//没网的时候将原本传到服务器文件的数据传到本地，当网联通的时候，将本地数据传送到服务器的文件中
int is_net = 0;//为0时表示未连接WIFI，为1时表示连接到了WIFI，初始为0
int flash_count = 0;//表示往本地flash一共发送了多少次数据
int base_address = 0x000000;//本地flash的初始地址

QueueHandle_t queue_hc05, queue_esp01s, queue_tools;

//hc05
void task1(void  *p) {

    int address = 0x0800F000;
    int status = 0;
    char buffer[40] = {0};

    status = *((__IO uint32_t *)(address));

    if (status != 0xFFFFFFFF) {//假设有内容的话直接用存储的账户密码链接
        //有内容，代表连过网，读取数据内容，不需要蓝牙发送账户密码
        printf1("------------------------have content------------------------\n");
        readFlash(address + 8, buffer,40); //从address + 8开始读数据，所以读数据也是从这里读，读出来的是WIFI账户和密
        printf1("read: %d,  %s \r\n", status,  buffer);//status就是我输入的14



        //        if () {//字怎么看连没连成功，用AT指令地返回值看
        //            //本身连接成功，不做处理

        //        } else {
        //            //本身连接没成功，重新联网

        //        }
    } else {
        //无内容，代表没连过网，存入数据，蓝牙发送给账户密码
        printf1("------------------------no content------------------------\n");

        //1:收蓝牙信息
        char buf[100] = {0};
        int flag = 0;

        while (1) {

			//flash无内容，且手机不发送账户密码，则这个任务一直会卡在这里。看门狗不会启动
            char ch = 0;
            BaseType_t ret =  xQueueReceive(queue_hc05, &ch, 1000);//当阻塞在这1000ms的途中，其他任务会轮流执行，类似vTaskDekat函数

            if (ret == pdFAIL) {
                //没收到数据
            } else {
                //收到数据
                if (ch == '!') {
                    flag++;
                }

                buf[strlen(buf)] = ch;
            }

            if (flag == 2) {
                break;
            }
        }

        //2：联网
        printf1("buf:%s \r\n", buf);

        strncpy(buffer, buf, strlen(buf));

        erasePage(address);
        changeFlashFor32(address, 14);//测试用
        changeFlashFor32(address + 4, strlen(buffer));//写入buffer的长度

        for (int i = 0; i < sizeof(buffer); i += 4) {
            changeFlashFor32(address + 8 + i, *(int *)(buffer + i));
        }

        //如果flash中无数据就可以把buf存进flash
    }

    //切割账户和密码->拼接指令
    //AT->esp01s->OK
    char name[20] = {0};
    char pass[20] = {0};

    int index = 0;

    for (int i = 1; i < strlen(buffer); i++) {
        if (buffer[i] == '=') {
            index = i;
            break;
        }

        name[i - 1] = buffer[i];
    }

    for (int i = index + 1 ; i < strlen(buffer); i++) {
        if (buffer[i] == '!') {
            break;
        }

        pass[i - index - 1] = buffer[i];
    }

    printf1("name:--%s-- \r\n", name);
    printf1("pass:--%s--\r\n", pass);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", name, pass);
    printf1("buf:--%s--", buffer);
    printf2("%s", buffer); //esp01-->AT指令
	
    vTaskDelay(10000);//保证了其他任务的正常运行

    //加一个二值信号量

    //3：TCP连接
    printf1("----------------TCP_CONNECT_START----------------\r\n");
    taskENTER_CRITICAL();
    TCP_Connect();
    taskEXIT_CRITICAL();
    printf1("----------------TCP_CONNECT----------------\r\n");



//走到这里即表示有网络了
	is_net = 1;//表示连接到了WIFI
	//如果本地flash存了内容，则把本地flash里的内容输出到服务器
	char flash_buf[20] = {0};
	char flash_final[25] = {0};
	uint32_t flash_sub = 0x000000;
	if(flash_count > 0){
		printf1("---------------flash_SPI:%d---------------\r\n",flash_count);
		for(int q = 0;q< flash_count;q++)//要读内存多少次
		{
			for(int b = 0;b<20;b++)//每次读多少字节
			{
				char ch =  W25Q64_Read_Byte(flash_sub);
				strcat(flash_buf,&ch);
				flash_sub++;
				if(ch == '!')
				{
					break;//遇到！表示一个message的结束
				}
			}
			memcpy(flash_final,flash_buf,strlen(flash_buf));
			printf1("%s\r\n",flash_final);//测试
			printf2("%s\r\n",flash_final);//将从SPI读出的数据往服务器发，思考要不要加\0
			Delay_ms(1000);
			memset(flash_buf,0,sizeof(flash_buf));
			memset(flash_final,0,sizeof(flash_final));
			
		}
		flash_count = 0;
	}
	
	    IWDG_Init();//启动独立看门狗

    //5：关闭连接
    //按一个按钮就退出链接，用二值信号量
    while (1) {
        vTaskDelay(1000);//task1因该为最高优先级
    }
}

////esp01s
void task2(void  *p) {
    char buf[200] = {0};

    while (1) {
        while (1) {
            char ch = {0};
            BaseType_t ret =  xQueueReceive(queue_esp01s, &ch, 1000);

            //超时踢出
            if (ch == '$') {
				taskENTER_CRITICAL();
                TCP_Release();
				taskEXIT_CRITICAL();
				 //flag_time = 0;
            }

            if (ret == pdFAIL && ch != '$') {
                //没收到数据
            } else {
                //收到数据
                //printf1("%c",ch);
                buf[strlen(buf)] = ch;
            }

            //感觉没必要跳出，跳出后就再也打印不了了
            //            if (strlen(buf) >= 6 && strncmp(&buf[strlen(buf) - 6], "\r\nOK\r\n", 6) == 0) {
            //                break;
            //            }

            //            if (strlen(buf) >= 6 && strncmp(&buf[strlen(buf) - 6], "ERROR\r\n", 7) == 0) {
            //                break;
            //            }
            //printf1("----------------task2---------------\r\n");
            printf1("%c", ch);
        }

        //printf1("task2:buf:%s\r\n", buf);
    }
}

//加一个超时踢出
void task3(void  *p) {
    char buf[20] = {0};
    int error_light = 1; //当为0时表示光敏电阻传感器异常，此时空气质量检测器正常，蜂鸣器根据error_light的情状决定开不开
    int printf2_error_light = 1;//当为1时可以发送异常，为0不可以发动
    int printf2_normal_light = 1;
    int printf2_error_air = 1;//当为1时可以发送异常，为0不可以发动
    int printf2_normal_air = 1;


    while (1) {

        Delay_ms(1000);

        uint16_t  ret  =  AD_GetValue(ADC_Channel_1);


        if (ret > LIGHT_LIMIT) {
            LED_LIGHT_NONORMAL();
            Buzzer_ON();
            error_light = 0;
            OLED_ShowString(1, 1, "LIGHT ERROR!      ");

            if (printf2_error_light == 1) {
				if(is_net == 1){
					//连接到网络,输出到ESP01S	
					 printf2("%s\r\n", "LIGHT_ERROR!");
				}else{
					//未连接到网络，输出到本地flash（SPI）
					W25Q64_Write_String(base_address, "LIGHT_ERROR!",strlen("LIGHT_ERROR!"));
					base_address += strlen("LIGHT_ERROR!");//每次往里面写20个字节
					flash_count++;
				}  
                printf2_error_light = 0;
                printf2_normal_light = 1;
            }
        } else {
            error_light = 1;
            LED_LIGHT_NORMAL();
            Buzzer_OFF();
            sprintf(buf, "LIGHT VALUE:%d", ret);
            OLED_ShowString(1, 1, buf);

            if (printf2_normal_light == 1) {
				if(is_net == 1){
					//连接到网络,输出到ESP01S	
					 printf2("%s\r\n", "LIGHT_NORMAL!");
				}else{
					//未连接到网络，输出到本地flash（SPI）
					W25Q64_Write_String(base_address,  "LIGHT_NORMAL!",strlen("LIGHT_NORMAL!"));
					base_address += strlen("LIGHT_NORMAL!");//每次往里面写20个字节
					flash_count++;
					
				}       
                printf2_normal_light = 0;
                printf2_error_light = 1;
            }

            memset(buf, 0, sizeof(buf));
        }


        uint16_t  ret4  =  AD_GetValue(ADC_Channel_4);


        if (ret4 < AIR_LIMIT){
            LED_AIR_NONORMAL();
            Buzzer_ON();
            OLED_ShowString(2, 1, "AIR ERROR!         ");

            if (printf2_error_air == 1) {
				if(is_net == 1){
					//连接到网络,输出到ESP01S	
					 printf2("%s\r\n", "AIR_ERROR!");
				}else{
					//未连接到网络，输出到本地flash（SPI）
					
					W25Q64_Write_String(base_address, "AIR_ERROR!",strlen("AIR_ERROR!"));
					base_address += strlen("AIR_ERROR!");//每次往里面写20个字节
					flash_count++;
					
				}            
                printf2_error_air = 0;
                printf2_normal_air = 1;
            }

        } else {
            LED_AIR_NORMAL();

            if (error_light != 0) {
                Buzzer_OFF();
            }

            sprintf(buf, "AIR VALUE:%d", ret4);
            OLED_ShowString(2, 1, buf);

            if (printf2_normal_air == 1) {
				if(is_net == 1){
					//连接到网络,输出到ESP01S	
					 printf2("%s\r\n", "AIR_NORMAL!");
				}else{
					//未连接到网络，输出到本地flash（SPI）
					W25Q64_Write_String(base_address, "AIR_NORMAL!",strlen( "AIR_NORMAL!"));
					base_address += strlen( "AIR_NORMAL!");//每次往里面写20个字节
					flash_count++;
					
				}   
                printf2_normal_air = 0;
                printf2_error_air = 1;
            }

            memset(buf, 0, sizeof(buf));
        }

    }

}

//没连接网络，复位按键没有用
void task4(void * p) {
    int flag = 0;//0为未按下按键，1为按下了按键

    while (1) {
        if (!flag) {
            IWDG_ReloadCounter();  // 喂狗
        }

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET) {
            vTaskDelay(20);//延时消抖

			printf1("----------Button_in------------\n");
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET) {
                flag = 1;//不在继续喂狗,在这个while中10s则狗咬人，没有超过10s，则喂狗
            }

            IWDG_ReloadCounter();  // 喂狗
            flag = 0;
        }
        vTaskDelay(2000);
    }
}

int main() {

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    USART1_Init();
    USART2_Init();
    USART3_Init();

    Delay_Init();
    AD_Init();
    Buzzer_Init();
    LED_Init();
    Delay_Init();
    OLED_Init();
	SPI1_Init();


    Buzzer_OFF();
    LED_AIR_NORMAL();
    LED_LIGHT_NORMAL();
    OLED_Clear();

    Button_Init(RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_0); // 按键1


    // 检查是否是因为IWDG触发的复位
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {
        printf1("reset by IWDG \r\n");
        RCC_ClearFlag();
    } else {
        printf1("Normal boot \r\n");
    }

    queue_hc05 = xQueueCreate(100, 1);
    queue_esp01s = xQueueCreate(150, 1);
    queue_tools =  xQueueCreate(150, 1);
	

	
	is_net = 0;//每次复位时自动置为1，复位后一开始肯定没网
	flash_count = 0;
	base_address = 0x000000;
	
	
	// 擦除扇区( 擦除多少? 擦除哪一些扇区?这是个问题)
    W25Q64_Erase_Sector(0x000000); // 擦除扇区0
    W25Q64_Erase_Sector(0x001000);// 擦除扇区1
    W25Q64_Erase_Sector(0x002000);// 擦除扇区2

    Delay_ms(5000);

    xTaskCreate(task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(task3, "Task3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(task4, "Task4", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();

}








