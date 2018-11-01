# RAK811_LoRaNode

1.RAK811 is a LoRa module based on STM32L151 and SX1276.It has two kinds of boards : RAK811-HF board and RAK811-LF board.

2.[The AT Command folder](https://github.com/RAKWireless/RAK811/tree/master/AT%20Command) contains two firmware: "RAK811_HF.bin"and"RAK811_LF.bin".
  It based LoRaWAN 1.0.2 protocol ,support Class A and Class C mode.User could switch the mode by such as 'at+set_config=class:2' command,0:class A,1:class B(unsupported),2:class C.
       "RAK811_HF.bin" surpport region:EU868, US915, AU915, KR920, AS923£¬IN865,CN779.
       "RAK811_LF.bin" surpport region:EU433£¬CN470.
  Region switch by such as"at+band=EU868"command.
  Details about AT command refer to "RAK811 Lora AT Command V1.4.pdf" file.
  
3.Method of The Demo project generates different firmware refer to "..\src\board\RAK811\ReleaseNotes.txt".
