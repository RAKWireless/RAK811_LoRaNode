/*
/ _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
\____ \| ___ |    (_   _) ___ |/ ___)  _ \
_____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
(C)2013 Semtech

Description: LoRaMac classA device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

//#define  LORA_HF_BOARD

/*! \file classA/LoRaMote/main.c */
#include <time.h>
#include <string.h>
#include "board.h"

#include "app.h"
#include "rw_lora.h"
#include "rw_sys.h"

lora_config_t g_lora_config;

extern void lora_cli_loop(void);

int main( void )
{
    uart_config_t uart_config;
	
    BoardInitMcu( );
    
    if (read_partition(PARTITION_1, (char *)&uart_config, sizeof(uart_config)) < 0) {
        SET_UART_CONFIG_DEFAULT(uart_config);
    } 
    
    UartMcuInit(&Uart1, 1, UART_TX, UART_RX);
    UartMcuConfig(&Uart1, RX_TX, uart_config.baudrate, 
                                      uart_config.wordLength,
                                      uart_config.stopBits,
                                      uart_config.parity,
                                      uart_config.flowCtrl);									

    e_printf("Welcome to RAK811.\r\n");
		
    rw_ReadUsrConfig();
		
    rw_InitLoRaWAN();
		
    rw_LoadUsrConfig();

	GPIOIRQ_Enable();	
		

#if 0
    DelayMs(5000);
    enter_sleep();
#endif
    e_printf("Initialization OK!\r\n");
    while(1) {
        lora_cli_loop();
        TimerLowPowerHandler( );
    }
}

