#include "PS2.h"
#include "scancode.h"
//5ms延时
static unsigned char IntNum = 0; //中断次数计数
static unsigned char KeyV; //键值
static unsigned char Key_UP=0, Shift = 0;//Key_UP是键松开标识，Shift是Shift键按下标识
static unsigned char BF = 0; //标识是否有字符被收到
static unsigned char PS2_ready = 0; //键盘初始化完毕标志
static unsigned char LEDS=0; //键盘上灯的状态

void wait_us(unsigned int delay)
{
delay=delay*OSC_US;
while (delay--);
}


void Keyboard_out(void) interrupt 2
{
if ((IntNum > 0) && (IntNum < 9))
{
KeyV = KeyV >> 1; //因键盘数据是低>>高，结合上一句所以右移一位
if (Key_Data) KeyV = KeyV | 0x80; //当键盘数据线为1时为1到最高位
}
IntNum++;
while (!Key_CLK); //等待PS/2CLK拉高
if (IntNum > 10)
{
IntNum = 0; //当中断11次后表示一帧数据收完，清变量准备下一次接收
BF = 1; //标识有字符输入完了
EX1 = 0; //关中断等显示完后再开中断 (注：如这里不用BF和关中断直接调Decode()则所Decode中所调用的所有函数要声明为再入函数)
//PutChar(2,0,KeyV);
}
}

void SendKeyCmd(unsigned char cmd){
bit D0,D1,D2,D3,D4,D5,D6,D7,parity,text1;
text1=EX1;
EX1=0;
	
if (Key_CLK)
 {
    wait_us(2000);
	Key_CLK=0;	
	D0=cmd&0x01;D1=cmd&0x02;D2=cmd&0x04;D3=cmd&0x08;
	D4=cmd&0x10;D5=cmd&0x20;D6=cmd&0x40;D7=cmd&0x80;
	parity=D0^D1^D2^D3^D4^D5^D6^D7;
	wait_us(20);
	Key_Data=0;
	wait_us(2);
	Key_CLK=1;
	while(Key_CLK);
	Key_Data=D0;	//写D0
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D1;	//写D1
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D2;	//写D2
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D3;	//写D3
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D4;	//写D4
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D5;	//写D5
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D6;	//写D6
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=D7;	//写D7
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=!parity;	//写奇偶校验位
	while(!Key_CLK);
	while(Key_CLK);
	Key_Data=1;	//写停止位
	wait_us(20);
 }
EX1=text1;
}

unsigned char Decode(unsigned char ScanCode) //注意:如SHIFT+G为12H 34H F0H 34H F0H 12H，也就是说shift的通码+G的通码+shift的断码+G的断码
{
unsigned char TempCyc,asc=0;

	if (ScanCode==0xAA)	PS2_ready=1;

if (!Key_UP) //当键盘松开时
{
switch (ScanCode)
 {
 case 0xF0 : // 当收到0xF0，Key_UP置1表示断码开始
	Key_UP = 1;
	break;

 case 0xFC :
	asc=0xFC;  //自检失败
	break;

 case 0xEE:
	asc=0xEE;  //键盘回复
	break;

 case 0x58: //若按下CapsLock
	LEDS^=KEY_LED_CAPS;
	SetKeyLED(LEDS);
	wait_us(2000);
	break;
 case 0x77: //若按下NumLock
	LEDS^=KEY_LED_NUM;
	SetKeyLED(LEDS);
	wait_us(2000);
	break;
 case 0x7E: //若按下ScrollLock
	LEDS^=KEY_LED_SCROLL;
	SetKeyLED(LEDS);
	wait_us(2000);
	break;
case 0x12 : // 左 SHIFT
	Shift = 1;
	asc=0x10;
	break;

case 0x59 : // 右 SHIFT
	Shift = 1;
	asc=0x10;
	break;

default:
  		   for (TempCyc = 0;(UnShifted[TempCyc][0]!=ScanCode)&&(TempCyc<26); TempCyc++);
			if (UnShifted[TempCyc][0] == ScanCode) 
				{
				if ((((LEDS&KEY_LED_CAPS)>0)&&(!Shift))||(((LEDS&KEY_LED_CAPS)==0)&&(Shift)))	//判断大写 
						asc=Shifted[TempCyc][1]; 
						else 
						asc=UnShifted[TempCyc][1];
				} 
		if (asc==0)
		{ 
			for (TempCyc = 26;(UnShifted[TempCyc][0]!=ScanCode)&&(TempCyc<47); TempCyc++);  	//判断shift
			if (UnShifted[TempCyc][0] == ScanCode) 
				{
					if (Shift) 
						asc=Shifted[TempCyc][1]; 
						else 	
						asc=UnShifted[TempCyc][1];
				}
		}
		if (asc==0) 
		{
			for (TempCyc = 47;(UnShifted[TempCyc][0]!=ScanCode)&&(TempCyc<59); TempCyc++);
			if (UnShifted[TempCyc][0] == ScanCode)
				{																		   //小键盘开关
				    if ((LEDS&KEY_LED_NUM)>0)
						asc=Shifted[TempCyc][1];
						else
						asc=UnShifted[TempCyc][1];
				}
		}
    	if (asc==0) 
		{ 
		 for(TempCyc = 0; (FuncKey[TempCyc][0]!=ScanCode)&&(TempCyc<34); TempCyc++);
		 if (FuncKey[TempCyc][0] == ScanCode) asc=FuncKey[TempCyc][1];
		}
 break;
 }
}
else
{
	Key_UP = 0;
	switch (ScanCode) //当键松开时不处理判码，如G 34H F0H 34H 那么第二个34H不会被处理
	{
		case 0x12 : // 左 SHIFT
		Shift = 0;
		break;

		case 0x59 : // 右 SHIFT
		Shift = 0;
		break;
	}
}
return asc;
}

unsigned char KeyInit()
{
EX1=1;
if (BF&&KeyV==0xAA) 
 { 
   	SetKeyRate(0x1f);
	//SetKeySet(2);
	//SetAllKeyType(KEY_TYPE_M);
	LEDS^=KEY_LED_NUM;
	SetKeyLED(LEDS);
	//SendEcho();
	BF=0;
	return 1;
 }
return 0; 
}

bit KeyState()
{
	return BF;
}
void SetState(bit b)
{
	BF=b;
}
unsigned char GetCode()
{
unsigned char ktmp=KeyV;
	BF = 0; //标识字符处理完了
	EX1 = 1;//开键盘中断接收下一数据
	return ktmp;
}
unsigned char LEDState()
{
	return LEDS;
}

void KeyReset()
{
	BF=0;
	ResetKey();
	EX1=1;
	LEDS=0;
}