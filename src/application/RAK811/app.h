#ifndef _APP_H_
#define _APP_H_
#include "board.h"
#include "LoRaMac.h"
#include "Region.h"
#include "lora_config.h"
#include "partition.h"
#include "log.h"
#include "cmd_error.h"
#include "rw_assert.h"


#define MAJOR_VER          2
#ifdef LORA_HF_BOARD
  #define CUSTOM_VER         0           //HF
#else
  #define CUSTOM_VER         1           //LF
#endif
#define FUN_VER            3
#define BUG_VER            4         //fit TTN and LoRaServer at region CN470
#define TEST_VER           1  // 1 test uplink downlink
                              // 2 actility cert test
                              // 3 HSI cabrit
                              // 4 actility cert TPIT 4.4
                              // 5 Fixed a situation where there was no response after waking up in sleep mode.
                              // 6 Integrate all bands and use the command to switch regions
                              // 7 Fix i2c access 0 address problem            


/*!
 * LoRaMac datarates definition
 */
#define SF_12                                       12  // SF12 - BW125
#define SF_11                                       11  // SF11 - BW125
#define SF_10                                       10  // SF10 - BW125
#define SF_9                                        9  // SF9  - BW125
#define SF_8                                        8  // SF8  - BW125
#define SF_7                                        7  // SF7  - BW125
#define SF_6                                        6  // SF7  - BW250

enum lora_event{
    LORA_EVENT_RECV_DATA = 0,
    LORA_EVENT_TX_COMFIRMED = 1,
    LORA_EVENT_TX_UNCOMFIRMED = 2,
    LORA_EVENT_JOINED_OTAA  = 3,
    LORA_EVENT_JOINED_FAILED = 4,
    LORA_EVENT_TX_TIMEOUT = 5,
    LORA_EVENT_RX2_TIMEOUT = 6,
    LORA_EVENT_DOWNLINK_REPEATED = 7,
    LORA_EVENT_WAKEUP = 8,
    /**Lora P2P define**/
    LORA_EVENT_P2PTX_COMPLETE = 9,
    LORA_EVENT_LINK_CHECK = 10,
    LORA_EVENT_UNKNOWN = 100,
};

                                        
#define SET_UART_CONFIG_DEFAULT(a) {\
                                       uart_config_t temp = DEFAULT_VALUE;\
                                       memcpy(&a, &temp, sizeof(temp));\
                                       write_partition(PARTITION_1, (char *)&a, sizeof(a));\
                                    }\


extern lora_config_t     g_lora_config;
extern lora_system_t     g_lora_system;

extern TimerTime_t SendRadioP2PFrame( S_LORAP2P_PARAM LoraP2PParams, void *fBuffer, uint16_t fBufferSize);
extern RadioState_t RxRadioP2PFrame( S_LORAP2P_PARAM LoraP2PParams );

#endif