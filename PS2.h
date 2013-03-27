/************************************************************************************
**                            PS2键盘驱动模块
**                     (c) Copyright 2007-2013, Dodge
**                          All Rights Reserved
**                    https://github.com/dodgepudding/PS2keyboard
**                               V1.00.00
---------------------------------文件信息--------------------------------------------
**文  件: PS2.c  功能实现模块
**       PS2.h  引脚配置定义及头文件引用
**编写人: dodgepudding (dodgepudding#gmail.com)
**描  述：本驱动针对普通PS2的101键键盘
          宏定义说明：
          KEY_XXX 			键盘状态或功能键码
          SetXXXX(x) 		设定键盘状态
          需在头文件定义的引脚：
          	Key_Data P36 //定义键盘信号线引脚
			Key_CLK P33	 //此版本CLK脚只能连INT1脚，外部中断1被占用
		  使用方法：
		  1）修改PS2.H头文件相应配置 ；
		  2）打开总中断，在程序靠前位置写入while (!KeyInit());直至键盘校检成功，
		  本初始化默认按键重复速率2次/秒，使用键盘编码集2，NumLock状态开。
		  3）此时可以运行SendKeyCmd()的相关宏，设置你需要的键盘状态。
		  4）判断键盘状态KeyState()的返回值，若有键按下，调用Decode(GetCode())来获得ASCII码。
		  但GetCode获取原始键码被执行后，会释放KeyV暂存，自动取下一键值，使用者可自行建立
		  键盘缓存，将GetCode()值保存起来，自行处理原始键码。
**历史版本：
			V1.00.00 键盘中断模式，模仿实际键盘的操作，包括上挡键，大小写，普通功能键
*************************************************************************************/


#include <ADuC842.h>  //按照单片机类型设置头文件
#define Key_Data P34 //定义键盘信号线引脚
#define Key_CLK P33	 //此版本CLK脚只能连INT1脚
#define OSC_US 4	//定义核心频率1us运行的机器指令数目，以校准延时

/*************************************************************************/
//PS2键盘模式设置
#define ResetKey()		SendKeyCmd(0xFF) //键盘复位，过程等待较长
#define SetDefault()    SendKeyCmd(0xF6) //设置默认模式
#define SetKeyRate(r)	SendKeyCmd(0xF3);SendKeyCmd(0x1F&r)	 //设置按键速率0x00-0x1f，值越大，速率越低
#define SetKeySet(t)	SendKeyCmd(0xF0);SendKeyCmd(t)		 //设定键盘使用编码集, 取值1/2/3，此版本只支持默认模式2
#define KEY_TYPE_T		0x07    //重复模式
#define KEY_TYPE_MB		0x08	//通/断模式
#define KEY_TYPE_M		0x09	//只通模式
#define KEY_TYPE_TMB	0x0A	//通/断/重复模式
#define SetAllKeyType(t) SendKeyCmd(0xF0|t)	 //设置按键响应模式，为上面四个参数之一，键盘编码集3模式下此功能才有效
#define ResendKey()		SendKeyCmd(0xFE)	 //重发上一键值
#define	KeyDisable()	SendKeyCmd(0xF5)	 //键盘禁止
#define KeyEnable()		SendKeyCmd(0xF4)	 //键盘允许
#define KEY_LED_SCROLL  0x01
#define KEY_LED_NUM		0x02
#define KEY_LED_CAPS	0x04
#define SetKeyLED(l)	SendKeyCmd(0xED);SendKeyCmd(0x07&l)	 //设置信号灯状态
#define SendEcho()	    SendKeyCmd(0xEE)	 //发送ECHO询问信号，键盘也会返回ECHO
/*************************************************************************/
/*************************************************************************/
//特殊功能键ASCII码
#define KEY_F1		0x13
#define KEY_F2		0x14
#define KEY_F3		0x15
#define KEY_F4		0x16
#define KEY_F5		0x17
#define KEY_F6		0x18
#define KEY_F7		0x19
#define KEY_F8		0x1A
#define KEY_F9		0x1C
#define KEY_F10		0x1D
#define KEY_F11		0x1E
#define KEY_F12		0x1F
#define KEY_ESC		0x1B
#define KEY_ENTER	0x0D
#define KEY_BACK	0x08
#define KEY_TAB		0x09
#define KEY_ALT		0x12
#define KEY_SHIFT	0x10
#define KEY_CTRL	0x11
#define KEY_INSERT	0x0A
#define KEY_HOME    0x06
#define KEY_PGUP	0x0B
#define KEY_DEL		0x0E
#define KEY_END		0x01
#define KEY_PGDN	0x03
#define KEY_UP		0x07
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x05

/*************************************************************************/
void SendKeyCmd(unsigned char cmd);	//发送键盘命令
unsigned char Decode(unsigned char ScanCode);  //键盘解码，返回的是按键的ASCII码，特殊功能键除外
unsigned char KeyInit();   //键盘初始化
bit KeyState();	   //键盘状态查询，为1表示按键未处理，为0表示按键已处理
unsigned char  GetCode(); //获得原始键码
void SetState(bit b);	 //设置键盘按键状态
unsigned char LEDState();//读取键盘灯状态
void KeyReset();	 //键盘重启
