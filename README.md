# RAK811_LoRaNode
**update RAK811_LoRaNode firmware version to 2.x.3.2**<br>
1.Fix a debug about "The Hard Fault exception occurs,Reset!".<br>
2.The end number in version indicate the version with DEBUG or RELEASE :odd:DEBUG ,even:RELEASE.<br><br>
**RAK811_LoRaNode firmware version 2.x.3.1**<br>

1.RAK811 is a LoRaNode module  based on STM32L151 and SX1276. It has two kinds of boards : RAK811-HF board and RAK811-LF board.

2.This Firmware is based on LoRaWAN 1.0.2 protocol ,support Class A and Class C mode.User could switch the mode by such as 'at+set_config=class:2' command,0:class A,1:class B(unsupported),2:class C.<br><br>
Tips:It supports almost all frequency bands:(HF)->EU868, US915, AU915, KR920, AS923，IN865.<br>
　　 　　　　　　　　　　　　　　　　　        (LF)->EU433，CN470.<br>

3.[The AT Command folder](https://github.com/RAKWireless/RAK811_LoRaNode/tree/master/doc/AT%20Command "AT Firmware") contains two firmware: "RAK811_HF.bin"and"RAK811_LF.bin". <br> 
　　 　"RAK811_HF.bin" surpport region:EU868, US915, AU915, KR920, AS923，IN865.<br> 　　 　
"RAK811_LF.bin" surpport region:EU433，CN470.<br>

Tips：  Region switch by such as"at+band=EU868"command，
  details about AT command refer to [RAK811 Lora AT Command V1.4.pdf](https://github.com/RAKWireless/RAK811_LoRaNode/blob/master/doc/Software/RAK811%C2%A0Lora%C2%A0AT%C2%A0Command%C2%A0V1.4.pdf).   <br>
  
4.Method of The Demo project generates different firmware refer to [ReleaseNotes.txt](https://github.com/RAKWireless/RAK811_LoRaNode/blob/master/src/board/RAK811/ReleaseNotes.txt).
