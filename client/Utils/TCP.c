#include "TCP.h"

void TCP_Connect() {
  
    printf2("%s\r\n", "AT+CIPSTART=\"TCP\",\"47.111.20.8\",9052");
    Delay_ms(5000);
  
    printf2("%s\r\n", "AT+CIPMODE=1");
    Delay_ms(2000);

    printf2("%s\r\n", "AT+CIPSEND");
    Delay_ms(2000);
	
}

void TCP_Release() {
    Delay_ms(2000);
    printf2("%s", "+++");
    Delay_ms(2000);


    printf2("%s\r\n", "AT+CIPMODE=0");
    Delay_ms(2000);


    printf2("%s\r\n", "AT+CIPCLOSE");
	printf1("----------------AT+CIPCLOSE----------\r\n");
    Delay_ms(2000);
}

