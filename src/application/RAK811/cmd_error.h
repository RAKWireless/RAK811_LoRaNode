#ifndef _CMD_ERROR_
#define _CMD_ERROR_

enum cmd_error {
    RAK_UNKNOWN_ERR = -20,
    RAK_TX_LEN_LIMITE_ERR = -13,
    RAK_RD_CFG_ERR = -12,    
    RAK_WR_CFG_ERR = -11,
    RAK_INTER_ERR = -8,
    RAK_TX_ERR = -7,
    RAK_MAC_BUSY_ERR = -6,
    RAK_NOT_JOIN = -5,    
    RAK_JOIN_OTAA_ERR = -4, 
    RAK_JOIN_ABP_ERR = -3,   
    RAK_ARG_NOT_FIND = -2,    
    RAK_ARG_ERR = -1,
    RAK_OK = 0,
};

#endif