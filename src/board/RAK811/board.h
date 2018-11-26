/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Andreas Pella (IMST GmbH), Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "stm32l1xx_hal.h"
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "radio.h"
#include "gps.h"
#include "gps-board.h"
#include "uart-board.h"
#include "sx1276/sx1276.h"
#include "adc-board.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "log.h"
#include "app.h"

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif


/*!
 * Board MCU pins definitions
 */
#ifdef LORA_HF_BOARD
	#define RADIO_RESET                                 PB_13
	#define RADIO_DIO_0                                 PA_11
	#define RADIO_DIO_2                                 PA_3
	#define RADIO_RF_CRX_RX                             PB_6  //CRF3
	#define RADIO_RF_CBT_HF                             PB_7  //CRF2 HF 
	#define OSC_LSE_IN                                  PC_14
	#define OSC_HSE_IN                                  PH_0
	#define OSC_HSE_OUT                                 PH_1
	#define JTAG_TDI                                    PA_15
	#define JTAG_TDO                                    PB_3
	#define I2C_SCL                                     PB_8
	#define I2C_SDA                                     PB_9	
#else
	#define RADIO_RESET                                 PA_8
	#define RADIO_DIO_0                                 PB_4
	#define RADIO_DIO_2                                 PB_9
	#define RADIO_RF_CRX_RX                             PB_3  //CRF3
	#define RADIO_RF_CBT_HF                             PB_8  //CRF2 HF 
	//#define OSC_LSE_IN                                   
	//#define OSC_HSE_IN                                  PH_0
	//#define OSC_HSE_OUT                                 PH_1
	//#define JTAG_TDI                                    PA_15
	//#define JTAG_TDO                                    PB_3
	#define I2C_SCL                                     PB_6
	#define I2C_SDA                                     PB_7
#endif


#define RADIO_XTAL_EN                               PH_1

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PB_0


#define RADIO_DIO_1                                 PB_1

#define RADIO_DIO_3                                 PH_0
#define RADIO_DIO_4                                 PC_13
//#define RADIO_DIO_5                                 PB_4



#define RADIO_RF_CTX_PA                             PA_4  //CRF1 PA 


#define OSC_LSE_OUT                                 PC_15

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12

#define JTAG_TMS                                    PA_13
#define JTAG_TCK                                    PA_14


//#define JTAG_NRST                                   PB_4

#define UART_TX                                     PA_9
#define UART_RX                                     PA_10
   
#define GPS_UART        		            UART_3 
#define GPS_UART_TX     		            PB_10
#define GPS_UART_RX     		            PB_11
#define GPS_POWER_ON_PIN		            PA_0
   
/*!
 * MCU objects
 */
extern Adc_t Adc;
extern I2c_t I2c;
extern Uart_t Uart1;
extern Uart_t GpsUart;

enum BoardPowerSource
{
    USB_POWER = 0,
    BATTERY_POWER
};

typedef struct{
    uint32_t baudrate;
    WordLength_t wordLength;
    StopBits_t stopBits;
    Parity_t parity; 
    FlowCtrl_t flowCtrl;
}uart_config_t;

#define DEFAULT_VALUE         {\
                                  115200,\
                                  UART_8_BIT,\
                                  UART_1_STOP_BIT,\
                                  NO_PARITY,\
                                  NO_FLOW_CTRL\
                              }

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Potentiometer level
 *
 * \retval value  Potentiometer level ( value in percent )
 */
uint8_t BoardMeasurePotiLevel( void );

/*!
 * \brief Measure the VDD voltage
 *
 * \retval value  VDD voltage in milivolts
 */
uint16_t BoardMeasureVdd( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source ( 0: USB_POWER,  1: BATTERY_POWER )
 */
uint8_t GetBoardPowerSource( void );


void InstallWakeUpPin(void);

void UninstallWakeUpPin(void);

void SysEnterUltraPowerStopMode( void );

#endif // __BOARD_H__
