/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2019/10/15
* Description        : Main program body.
*******************************************************************************/ 

/*
 *@Note
 GPIO例程：
 PA0推挽输出。
 
 本程序共有两个修改点
 1. main中修改GPIO的初始化
 2. debug.h中修改GPIO的宏定义（主要针对不同的端口）
 3. debug.c中修改了串口的初始化部分参数，全部有中文注释的
*/

#include "debug.h"

/* Global define */


/* Global Variable */ 


/*******************************************************************************
* Function Name  : GPIO_Toggle_INIT
* Description    : Initializes GPIOA.0
* Input          : None
* Return         : None
*******************************************************************************/
/* 初始化GPIO端口
如果需要修改GPIO，一定要在这里也修改
*/
void GPIO_Toggle_INIT(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;              
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);   

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);           
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/

//一些初始化的全局变量
	u16 USART_RX_STA;
	u8 USART_RX_BUF[USART_REC_LEN];

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  Delay_Init();
	USART_Printf_Init(115200);
	GPIO_Toggle_INIT();
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("GPIO Toggle TEST\r\n");
  //初始化一些变量
	u8 t; 
	u8 len; 

	//初始化DEP继电器（即关闭DEP继电器）
	PIN_DEP1=RELAY_OFF;
	PIN_DEP2=RELAY_OFF;
	PIN_DEP3=RELAY_OFF;
	PIN_DEP4=RELAY_OFF;
	//设置串口循环接收数据的间隔，单位毫秒
	int serial_interval=20;

	for(int i=1;i<7;i++){
		PIN_L(i)=RELAY_OFF;
		PIN_R(i)=RELAY_OFF;
	}

	printf("system initialized! \r\n");
	printf("serial interval is %d ms.\r\n",serial_interval);
	
	while(1)
  {	
	  if(USART_RX_STA&0x8000){
		len=USART_RX_STA&0x3f;
		printf("cmd received: \r\n");

		if((USART_RX_BUF[0]<0x37)&(USART_RX_BUF[0]>0x29)){
			int channel=USART_RX_BUF[0]-0x30;
			PIN_L(channel)=!PIN_L(channel);
			PIN_R(channel)=!PIN_R(channel);
			if(PIN_R(channel)^PIN_L(channel)) printf("WRONG STATUS,TRY AGAIN!");
			else if(PIN_R(channel)) {printf("channel %d is opened!\r\n",channel);PIN_R(0)=1;}
			else {printf("channel %d is closed!\r\n",channel);PIN_R(0)=0;}
		}
		else{
			printf("invalid command!\r\n");
		}
		/*
		if(USART_RX_BUF[0]==0x31){PIN_R(0)=1;}
		else if (USART_RX_BUF[0]==0x32){PIN_R(0)=0;}
		*/
			for(t=0;t<len;t++) {
				USART_SendData(USART1, USART_RX_BUF[t]);    //向串口1 发送数据 
            while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET); //等待发送结束
			}
		USART_RX_STA=0;
		//USART_ReceiveData(USART1);
	  }
	  else{
		//   假如没有接收到数据，则直接delay并退出循环
		  Delay_Ms(serial_interval);
		  //continue;
	  }
		//printf("%d",USART_RX_STA);
	  //uint16_t serial_cmd=USART_ReceiveData(USART1);
	  //printf("%d \r\n",serial_cmd);
		//Delay_Ms(500);

//		PBout(1) ^= (1<<0);
		//printf("PB\r\n");
	}
}

void USART1_IRQHandler(void)
{ 
 u8 Res; 
if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){ 
  Res =USART_ReceiveData(USART1);
	if((USART_RX_STA&0x8000)==0){ 
		if(USART_RX_STA&0x4000){ 
			if(Res!=0x0a)USART_RX_STA=0;
			else USART_RX_STA|=0x8000; //这里相当于必须识别到0x0a 
    } 
		else{  
			if(Res==0x0d)USART_RX_STA|=0x4000; 
			else{ 
				USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ; 
				USART_RX_STA++; 
				if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0; 
     }     
		} 
   }           
}   
}

