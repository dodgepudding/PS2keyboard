PS2keyboard
===========

PS2 keyboard for embed system like 51 and AVR.

描  述
-------
本驱动针对普通PS2的101键键盘, 适用于51和AVR单片机, 程序采用键盘中断模式，模仿实际键盘的操作，包括上挡键，大小写，普通功能键  

文件头宏定义
-------
KEY_XXX 	键盘状态或功能键码  
SetXXXX(x) 		设定键盘状态  
需在头文件定义的引脚：  
  Key_Data P36 //定义键盘信号线引脚  
  Key_CLK P33	 //此版本CLK脚只能连INT1脚，外部中断1被占用  

初始化方法 
-------
1）修改PS2.H头文件相应配置；  
2）打开总中断，在程序靠前位置写入while (!KeyInit());直至键盘校检成功，
本初始化默认按键重复速率2次/秒，使用键盘编码集2，NumLock状态开。  
3）此时可以运行SendKeyCmd()的相关宏，设置你需要的键盘状态。  
4）判断键盘状态KeyState()的返回值，若有键按下，调用Decode(GetCode())来获得ASCII码。  
但GetCode获取原始键码被执行后，会释放KeyV暂存，自动取下一键值，使用者可自行建立
键盘缓存，将GetCode()值保存起来，自行处理原始键码。  

函数方法
-------
ResetKey()  //键盘复位，过程等待较长  
SetDefault()    //设置默认模式  
SetKeyRate(r)  //设置按键速率0x00-0x1f，值越大，速率越低  
SetKeySet(t)  //设定键盘使用编码集, 取值1/2/3，此版本只支持默认模式  
SetAllKeyType(t) //设置按键响应模式，为上面四个参数之一，键盘编码集3模式下此功能才有效, 允许输入以下几种模式值  
> KEY_TYPE_T		0x07    //重复模式  
> KEY_TYPE_MB		0x08	//通/断模式  
> KEY_TYPE_M		0x09	//只通模式  
> KEY_TYPE_TMB	0x0A	//通/断/重复模式  

ResendKey()	 //重发上一键值  
KeyDisable()  //键盘禁止  
KeyEnable()	  //键盘允许  
SetKeyLED(l) //设置信号灯状态, 允许输入以下几种模式值  
> KEY_LED_SCROLL  0x01  
> KEY_LED_NUM		0x02  
> KEY_LED_CAPS	0x04  

SendEcho() //发送ECHO询问信号，键盘也会返回ECHO  
void SendKeyCmd(unsigned char cmd);	//发送键盘命令  
unsigned char Decode(unsigned char ScanCode);  //键盘解码，返回的是按键的ASCII码，特殊功能键除外  
unsigned char KeyInit();   //键盘初始化  
bit KeyState();	   //键盘状态查询，为1表示按键未处理，为0表示按键已处理  
unsigned char  GetCode(); //获得原始键码  
void SetState(bit b);	 //设置键盘按键状态  
unsigned char LEDState();//读取键盘灯状态  
void KeyReset();	 //键盘重启  


