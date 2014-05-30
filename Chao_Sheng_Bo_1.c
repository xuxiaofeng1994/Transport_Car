/*
	超声波测距程序
	12Mhz晶振  stc52芯片 H。。。超声波模块
*/
/**************************/

#include<reg52.h>
//#include<stdio.h> //等以后研究Keil中的printf()函数的时候再调用这个库

#define uchar unsigned char
#define uint unsigned int

sbit Trig = P1^5;
sbit Echo = P3^2;

uint distance;

/**************************/
//毫秒级的延时函数
void delayms(xms){
	uint i,j;
	for (i = xms; i > 0; i--){
		for (j = 110; j > 0; j--);
	}
}
/**************************/
//微秒级的延时函数
void delayus(uint xus){
	uint i;
	for(i = 0; i < xus; i++);	
}
/**************************/
//初始化定时器T0,T0用来记录Echo持续的高电平的时间
void init_TX(void){
	IT0 = 0;//采用低电平触发的方式

	TH0 = 0;
	TL0 = 0;
	TR0 = 0;//初值清零
}
/**************************/
//初始化串口端，这里没有用T1定时器，而是采用52及以上芯片中含有的T2定时器
//这是因为串口助手的默认波特率为9600Hz,而我们的板子采用的是12MHz的晶振，
//若采用T1,则在9600波特率下的误差将达到8.51%,这基本不能用。
void init_UART(void){

	RCAP2L = 0xD9;
	RCAP2H = 0xFF;//上两行设置在9600波特率下RACP2X的初值，FFD9H

	T2CON = 0x34;//RCLK = 1,TCLK = 1,TR2 = 1,当TCLK = 0时，将采用T1的溢出率来计算波特率

	SCON = 0x50;//设置串口的工作模式为1,接收使能
}
/**************************/
//初始化超声波测距程序
void init(void){

	EA = 1;//开启总中断

	//TMOD = 0x21;
	TMOD = 0x01;//设置T0为工作模式1

	init_UART();
	init_TX();

	Trig = 0;//拉低发送脉冲端
}
/**************************/
//发送持续时间大于10us的脉冲信号
void send_Trig(void){

	EA = 0;//发送脉冲期间关闭所有中断

	Trig = 1;
	delayus(100);//这里延时了20us				   	
	Trig = 0;

}
/**************************/
//获得Echo在收到反射信号后所产生的高电平
void get_Echo(void){

	while (Echo == 0);//等待Echo返回来的高电平

	EA = 1;
	TR0 = 1;

	EX0 = 1;//开启外部中断
	//一旦接收到了Echo返回的高电平，则开始T0开始计数

	while (Echo);//等待中断函数中的distance测量完毕

	EX0 = 0;//关闭外部中断（程序到这里，不出意外的话distance的值已经测量完毕了）

	TR0 = 0;
	TH0 = 0;
	TL0 = 0;//清零T0,为下一次测距做准备
}
/**************************/
//发送单个字符
void UART_send_byte(uint dat)
{
	SBUF=dat;
	while(TI==0);
	TI=0;
}
/**************************/
//发送测距结果
//这里我想用printf()来实现，但没成功，打印出来的distance的值始终为0
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
	UART_send_byte('\r');//回车
	UART_send_byte('\n');//换行

}
/**************************/
void main(void){
	init();
	while(1){
		send_Trig();
		get_Echo();
		send_date_UART(distance);
		delayms(1000);
	}	
}
/**************************/
void extra_interrupt() interrupt 0{
	distance = (256.0 * TH0 + TL0)* 0.184;//读出TH0和TL0的值，并计算出distance

	//这里的公式我没细推，用TH0和TL0算出来的应该是时间，然后0.184应该是声音在空气中的传播速度，厘米/秒
}
