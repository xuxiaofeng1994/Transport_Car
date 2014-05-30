#include<reg52.h>
#include<stdio.h>

#define uchar unsigned char
#define uint unsigned int

sbit Trig = P1^5;
sbit Echo = P3^2;




uint distance;
uint th0,tl0;

void delayms(xms){
	uint i,j;
	for (i = xms; i > 0; i--){
		for (j = 110; j > 0; j--);
	}
}

void delayus(uint xus){
	uint i;
	for(i = 0; i < xus; i++);	
}

void init_TX(void){
	IT0 = 0;
	TH0 = 0;
	TL0 = 0;
	TR0 = 0;	
}

void init_UART(void){
	RCAP2L = 0xD9;
	RCAP2H = 0xFF;

	T2CON = 0x34;
	SCON = 0x50;
}

void init(void){
	EA = 1;
	TMOD = 0x21;
	init_UART();
	init_TX();
	Trig = 0;	
}
void send_Trig(void){
	EA = 0;
	Trig = 1;
	delayus(100);				   	
	Trig = 0;
}

void get_Echo(void){

	while (Echo == 0);	
	EA = 1;
	TR0 = 1;
	EX0 = 1;
	while (Echo);
	EX0 = 0;
	TR0 = 0;
	TH0 = 0;
	TL0 = 0;
}

void UART_send_byte(uint dat)
{
	SBUF=dat;
	while(TI==0);
	TI=0;
}

void send_date_UART(uint temp){
//	ES = 0;
//	TI = 1;
//	printf("The distance is %d cm\n",temp);	
//	while (!TI);
//	TI = 0;
//	ES = 1;
	UART_send_byte((temp/10000)%10 + '0');
	UART_send_byte((temp/1000)%10 + '0');
	UART_send_byte((temp/100)%10 + '0');
	UART_send_byte((temp/10)%10 + '0');
	UART_send_byte('.');
	UART_send_byte(temp%10 + '0');
	UART_send_byte('c');
	UART_send_byte('m');
	UART_send_byte('\r');
	UART_send_byte('\n');
}
void main(void){
	init();
	while(1){
		send_Trig();
		get_Echo();
		send_date_UART(distance);
		delayms(1000);
	}	
}

void extra_interrupt() interrupt 0{
	distance = (256.0 * TH0 + TL0)* 0.184;
}
