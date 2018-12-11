#include "board.h"
#include "app.h"
#include "LoRaMac.h"

#include "rw_sys.h"
#include "rw_lora.h"


/** Structure for registering CLI commands */
struct cli_cmd {
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    //const char *help;
    /** The function that should be invoked for this command. */
    void (*function) (int argc, char *argv[]);
};

static void set_boot_config(uint32_t cmd);

static void out_error(int err);
static void lora_mode (int argc, char *argv[]);
static void lora_rf_config(int argc, char *argv[]);
static void lora_txc(int argc, char *argv[]);
static void lora_rxc(int argc, char *argv[]);
static void lora_tx_stop(int argc, char *argv[]);
static void lora_rx_stop(int argc, char *argv[]);
static void lora_status(int argc, char *argv[]);
static void lora_join (int argc, char *argv[]);
static void lora_reload(int argc, char *argv[]);
static void lora_signal(int argc, char *argv[]);
static void lora_version(int argc, char *argv[]);
static void lora_band(int argc, char *argv[]);
static void lora_reset(int argc, char *argv[]);
static void lora_write_config(int argc, char *argv[]);
static void lora_read_config(int argc, char *argv[]);
static void lora_send(int argc, char *argv[]);
static void lora_link_check(int argc, char *argv[]);
static void start_boot(int argc, char *argv[]);
static void lora_uart_config(int argc, char *argv[]);
static void lora_gpio(int argc, char *argv[]);
static void lora_rd_adc(int argc, char *argv[]);
static void lora_rd_reg(int argc, char *argv[]);
static void lora_wr_reg(int argc, char *argv[]);
static void lora_rd_iic(int argc, char *argv[]);
static void lora_wr_iic(int argc, char *argv[]);
static void lora_recv_set(int argc, char *argv[]);
static void lora_link_cnt(int argc, char *argv[]);
static void lora_abp_info(int argc, char *argv[]);
static void lora_sleep(int argc, char *argv[]);
static void lora_tx_dr(int argc, char *argv[]);

extern char config_buf[];

struct cli_cmd cli_cmds[] = {
                                 "join",           lora_join,
                                 "version",        lora_version,         
                                 "reset",          lora_reset,
                                 "boot",           start_boot,
                                 "signal",         lora_signal,
                                 "link_check",     lora_link_check,
                                 "get_config",     lora_read_config,
                                 "reload",         lora_reload,
                                 "set_config",     lora_write_config,
                                 "send",           lora_send,
                                 "uart",           lora_uart_config,
                                 "sleep",          lora_sleep,
                                 "dr",             lora_tx_dr,
                                 /**2016-11-24 junhua add**/
                                 "mode",           lora_mode,
                                 "rf_config",      lora_rf_config,
                                 "txc",            lora_txc,
                                 "rxc",            lora_rxc,
                                 "tx_stop",        lora_tx_stop,
                                 "rx_stop",        lora_rx_stop,
                                 "status",         lora_status,   
                                 /**2017-04-19 junhua add**/
																 "band",           lora_band,
																 /**2017-08-09 junhua add**/
                                 "rd_reg",          lora_rd_reg,
                                 "wr_reg",          lora_wr_reg,
                                 "gpio",            lora_gpio,
                                 "rd_adc",          lora_rd_adc,
                                 "rd_iic",          lora_rd_iic, //100KHz, Master, 7 bit address
                                 "wr_iic",          lora_wr_iic,
                                 "recv_ex",         lora_recv_set,
                                 "link_cnt",        lora_link_cnt,
                                 "abp_info",        lora_abp_info,
                                 };

lora_system_t     g_lora_system;
static char cli_buffer[CLI_LENGTH_MAX];
extern int e_getchar(void);

static int parse_args(char* str, char* argv[])
{
    int i = 0;
    char* ch = str;

    while(*ch != '\0') {
        i++;
        /*Check if length exceeds*/
        if (i > MAX_ARGV) {
            return 0;
        }

        argv[i-1] = ch;
        

        while(*ch != ',' && *ch != '\0' && *ch != '\r') {
            if (*ch == ':') {
                return i;  
            } 
            
            if(*ch == '=' && i == 1) {
                break;
            }
            else
                ch++;
        }
        if (*ch == '\r')
            break;
        if (*ch != '\0') {
            *ch = '\0';
            ch++;
            while(*ch == ',') {
                ch++;
            }
        }
    }
    return i;
}

static void do_work(char *str)
{
    int i;
    int argc;
    char* argv[MAX_ARGV]={NULL};
		
    if ((strncmp(str, "at+", 3) != 0) || str[3] == '\0') {
        out_error(RAK_ARG_ERR);
        return;
    }
    DPRINTF("[echo cmd:] %s\r\n", str);
    str += 3;
    argc = parse_args(str, argv);
    if (argc > 0) {
        for (i = 0; i < sizeof(cli_cmds)/sizeof(struct cli_cmd); i++) {
            if (strcmp(argv[0], cli_cmds[i].name) == 0) {
                cli_cmds[i].function(argc, argv);
                break;
            }
        
        }
        if (i == sizeof(cli_cmds)/sizeof(struct cli_cmd)) {
            out_error(RAK_ARG_ERR);
        }
    }
    else {
        out_error(RAK_ARG_ERR);
        return;
    }
}

int check_hex_invaild(uint8_t *data, uint16_t len) 
{
    uint8_t check1 = 0xff, check2 = 0;
    
    for (int i = 0; i < len; i++) {
        check1 &= data[i];
        check2 |= data[i];
    }
    if (check1 == 0xff || check2 == 0) {
        return 1;
    }
    return 0;
}

void lora_cli_loop(void)
{
    int c;
    int process_cli = 0;
    static int i;  
    c = e_getchar();
    if (c >= 0) {
        cli_buffer[i] = (char)c;
        if (c == '\r') {
            cli_buffer[i] = '\0';
        }
        i++;
        if (c == '\n' || i == CLI_LENGTH_MAX) {
            process_cli = 1; 
        } 
    } 
    
    if (process_cli) {
        process_cli = 0;
        do_work(cli_buffer);
        i = 0;
    }
}

static void out_error(int err)
{
    e_printf("ERROR%d\r\n", err);
}

static void out_hex_buf( char *buffer, uint16_t size)
{
     char hex_str[3] = {0};
  
    for (int i = 0; i < size; i++) {
        sprintf(hex_str, "%02x", buffer[i]);
        e_printf("%s", hex_str); 
    }
}

static void lora_mode(int argc, char *argv[])
{
    if (argc > 2) {
        out_error(RAK_ARG_ERR);
        return;
    }

    if (argc ==1) {
	   e_printf("OK%d\r\n",LoRaGetUserMode());
	   return;
    }

    if (read_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config)) < 0) {
        out_error(RAK_RD_CFG_ERR);
        return;
    }
		
   if (strcmp(argv[1], "0") == 0 || strcmp(argv[1], "1") == 0) {
	   g_lora_config.lora_mode = atoi(argv[1]);
	   LoRaSetUserMode(g_lora_config.lora_mode);
	   write_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config));
		 e_printf("Mode switch OK!Reset now...\r\n");
	   NVIC_SystemReset();//rw_ResetLoRaWAN();
    }
    else {
	   out_error(RAK_ARG_ERR);
	   return;
    }

//    e_printf("OK\r\n");
}

static void lora_rf_config(int argc, char *argv[])
{
    if (argc == 1) {      
        e_printf("OK%d,%d,%d,%d,%d,%d\r\n",  g_lora_config.lorap2p_param.Frequency, 
                                          	 g_lora_config.lorap2p_param.Spreadfact,
                                          	 g_lora_config.lorap2p_param.Bandwidth,
                                          	 g_lora_config.lorap2p_param.Codingrate,
                                          	 g_lora_config.lorap2p_param.Preamlen,
                                          	 g_lora_config.lorap2p_param.Powerdbm );
        return;
    } else {
        if (argc != 7) {
            out_error(RAK_ARG_ERR);
			return;
        }
	if (!(CHECK_P2P_FREQ(atoi(argv[1])) &&
	         CHECK_P2P_SF(atoi(argv[2])) &&
	         CHECK_P2P_BDW(atoi(argv[3])) &&
	         CHECK_P2P_CR(atoi(argv[4])) &&
	         CHECK_P2P_PREMLEN(atoi(argv[5])) &&
	         CHECK_P2P_PWR(atoi(argv[6])))) {
            out_error(RAK_ARG_ERR);
	    return;  
	  } 

	  if (read_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config)) < 0) {
		  out_error(RAK_RD_CFG_ERR);
		  return;
	  }

	  g_lora_config.lorap2p_param.Frequency = atoi(argv[1]);
	  g_lora_config.lorap2p_param.Spreadfact = atoi(argv[2]);
	  g_lora_config.lorap2p_param.Bandwidth = atoi(argv[3]);
	  g_lora_config.lorap2p_param.Codingrate = atoi(argv[4]);
	  g_lora_config.lorap2p_param.Preamlen = atoi(argv[5]);
	  g_lora_config.lorap2p_param.Powerdbm = atoi(argv[6]);
	  write_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config));
	  e_printf("OK\r\n");
    }
	
    return;
}

static void lora_txc(int argc, char *argv[])
{
    int      ret;
    uint16_t  counts = 0;
    uint32_t interval_ms = 0;
    int      tx_len = 0;
    uint8_t *tx_data = (uint8_t *)argv[3];
    char     hex_num[3] = {0};
    
    if (argc != 4) {
        out_error(RAK_ARG_ERR);
        return;
    }

    if ( !(CHECK_P2P_FREQ(g_lora_config.lorap2p_param.Frequency) &&
		   CHECK_P2P_SF(g_lora_config.lorap2p_param.Spreadfact) &&
		   CHECK_P2P_BDW(g_lora_config.lorap2p_param.Bandwidth) &&
		   CHECK_P2P_CR(g_lora_config.lorap2p_param.Codingrate) &&
		   CHECK_P2P_PREMLEN(g_lora_config.lorap2p_param.Preamlen) &&
		   CHECK_P2P_PWR(g_lora_config.lorap2p_param.Powerdbm))) {
		   
		out_error(RAK_ARG_ERR);
        return;
    }

    counts =  atoi(argv[1]);
	
    interval_ms = atoi(argv[2]);  
    if (counts >1) {
	if (interval_ms < 10 || interval_ms > 3600*1000) {
           out_error(RAK_ARG_ERR);
           return;
        }
    } 
  
    tx_len = strlen(tx_data);
    
    if (tx_len%2) {
        out_error(RAK_ARG_ERR);
        return;
    }
	
    if (tx_len > 64*2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    for (int i = 0; i < tx_len; i++) {
        if (!isxdigit(tx_data[i])) {
            out_error(RAK_ARG_ERR);
            return;   
        }
    }
    tx_len = tx_len/2;
    for (int i = 0; i < tx_len; i++) {
        memcpy(hex_num, &tx_data[i*2], 2);
        tx_data[i] = strtoul(hex_num, NULL, 16);
    }    
    ret = rw_LoRaP2PTxContinue(counts, interval_ms, tx_len, tx_data);
    if (ret != 0) {
        if (ret == LORAMAC_STATUS_BUSY) {
            ret = RAK_MAC_BUSY_ERR;
        } else {
            ret = RAK_ARG_ERR;
        }
        out_error(ret);
        return;
    }
    e_printf("OK\r\n");   
}

static void lora_rxc(int argc, char *argv[])
{
    int      ret;
	uint8_t  report_en = 0;
    
    if (argc != 2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    if ( !(CHECK_P2P_FREQ(g_lora_config.lorap2p_param.Frequency) &&
		   CHECK_P2P_SF(g_lora_config.lorap2p_param.Spreadfact) &&
		   CHECK_P2P_BDW(g_lora_config.lorap2p_param.Bandwidth) &&
		   CHECK_P2P_CR(g_lora_config.lorap2p_param.Codingrate) &&
		   CHECK_P2P_PREMLEN(g_lora_config.lorap2p_param.Preamlen) &&
		   CHECK_P2P_PWR(g_lora_config.lorap2p_param.Powerdbm))) {
		   
		out_error(RAK_ARG_ERR);
        return;
    }
    
    report_en =  atoi(argv[1]);
    if (report_en > 1) {
        out_error(RAK_ARG_ERR);
        return;
    }
	
    ret = rw_LoRaP2PRxContinue(report_en);
    if(ret != 0) {
        out_error(RAK_INTER_ERR);
        return;
    }
	
    e_printf("OK\r\n");   
}


static void lora_tx_stop(int argc, char *argv[])
{   
    if (argc != 1) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    rw_LoRaP2PTxStop();	
	
    e_printf("OK\r\n");   
}

static void lora_rx_stop(int argc, char *argv[])
{   
    if (argc != 1) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    rw_LoRaP2PRxStop();	
	
    e_printf("OK\r\n");   
}

static void lora_status(int argc, char *argv[])
{
    RadioStatus_t*   RStauts;   
    
    if (argc > 2) {
        out_error(RAK_ARG_ERR);
        return;
    }

    if (argc == 1) {
      RStauts = LoRaGetRadioStatus();
      e_printf("OK%d,%d,%d,%d,%d,%d,%d\r\n", RStauts->TxSuccessCnt,
               RStauts->TxErrCnt,
               RStauts->RxSuccessCnt,
               RStauts->RxTimeOutCnt,
               RStauts->RxErrCnt,
               RStauts->rssi,
               RStauts->snr);
      return;
    } 
    
    if (strcmp(argv[1], "0") == 0) {
      LoRaClrRadioStatus();
      e_printf("OK\r\n");
    } else {
      out_error(RAK_ARG_ERR);
    }  
    
    return;
}

static void lora_join(int argc, char *argv[])
{
    int ret;
        
    if (argc != 2) {
        out_error(RAK_ARG_ERR);
        return;
    }
	
#if 1
    if (read_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config)) < 0) {
        out_error(RAK_RD_CFG_ERR);
        return;
    }
#endif		  
    if (strcmp(argv[1], "otaa") == 0) {
        if (check_hex_invaild(g_lora_config.dev_eui, sizeof(g_lora_config.dev_eui)) != 0 ||
            check_hex_invaild(g_lora_config.app_key, sizeof(g_lora_config.app_key)) != 0 ||
            check_hex_invaild(g_lora_config.app_eui, sizeof(g_lora_config.app_eui)) != 0 ) {
            out_error(RAK_ARG_ERR);
            return;    
        }   
//e_printf("g_lora_config.join_cnt£º%d\r\n",g_lora_config.join_cnt);
        ret = rw_JoinNetworkOTAA(g_lora_config.dev_eui, g_lora_config.app_eui, g_lora_config.app_key, g_lora_config.join_cnt);
        if (ret < 0) {
            out_error(RAK_JOIN_OTAA_ERR);
            return; 
        } 
    } else if (strcmp(argv[1], "abp") == 0) {
        /* Check nwks_key */
        if (check_hex_invaild(g_lora_config.nwks_key, sizeof(g_lora_config.nwks_key)) != 0 ||
            check_hex_invaild(g_lora_config.apps_key, sizeof(g_lora_config.apps_key)) != 0 ||
            check_hex_invaild(g_lora_config.dev_addr, sizeof(g_lora_config.dev_addr)) != 0 ) {
            out_error(RAK_ARG_ERR);
            return;    
        }
        
        ret = rw_JoinNetworkABP((uint32_t *)&g_lora_config.dev_addr[0], g_lora_config.nwks_key, g_lora_config.apps_key);
        
        if (ret < 0) {
            out_error(RAK_JOIN_ABP_ERR);
            return; 
        }
    } else {
        out_error(RAK_ARG_ERR);
        return;
    }
    e_printf("OK\r\n");
}

static void lora_signal(int argc, char *argv[])
{
    int16_t rssi;
    uint8_t snr;
    
    LoRaGetRssiSnr(&rssi, &snr);

    e_printf("OK%d,%d\r\n",rssi, snr);
}

static void lora_version(int argc, char *argv[])
{
    char * ver = argv[0];

    rw_GetVersion(ver);
    e_printf("OK%s\r\n", ver);
}

static void lora_band(int argc, char *argv[])
{
  LoRaMacRegion_t region;
  
  if (argc == 1) {
        e_printf("OK.%s\r\n", g_lora_config.region);
        return;
  } else if (argc == 2) {
    if ( 0==strcmp(argv[1], g_lora_config.region)) { 
         e_printf("OK.\r\n");
    } else {		
        region = rw_Str2Region(argv[1]);
        if (region == REGION_NULL) {
          out_error(RAK_ARG_ERR);
          return;
        } 
#ifdef LORA_HF_BOARD
					if ((region == LORAMAC_REGION_EU433)||((region == LORAMAC_REGION_CN470)))
					{
						e_printf("Error!This Board surport region only such as:\r\n EU868, US915, AU915, KR920, AS923£¬IN865.\r\n");
						return;
					}	
#else 					
					if ((region != LORAMAC_REGION_EU433)&&((region != LORAMAC_REGION_CN470)))
					{
						e_printf("Error!This Board surport region only such as:CN470,EU433.\r\n");
						return;
					}	
#endif
        if (rw_restore_LoRaWAN_config(region, 0) < 0) {
          out_error(RAK_ARG_ERR);
          return;
        }
				e_printf("OK.\r\n");
        NVIC_SystemReset();
//				rw_ResetLoRaWAN();
        
    }
  
  } else {
    out_error(RAK_ARG_ERR);
  }
   
}

static void lora_link_check(int argc, char *argv[])
{
    int ret;
    ret = rw_LoRaLinkCheck();
    if (ret < 0) {
        if (ret != 0) {
            if (ret == LORAMAC_STATUS_BUSY) {
                ret = RAK_MAC_BUSY_ERR;
            } else if (ret == LORAMAC_STATUS_NO_NETWORK_JOINED) {
                ret = RAK_NOT_JOIN;
            } else {
                ret = RAK_UNKNOWN_ERR;
            }
            out_error(ret);
            return;
        } 
        out_error(ret);
        return;
    }
    e_printf("OK\r\n");
}


static void lora_reset(int argc, char *argv[])
{
  if (argc != 2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    if (strcmp(argv[1], "0") == 0) {
        e_printf("OK\r\n");
        rw_ResetMCU(1);
    } else if (strcmp(argv[1], "1") == 0) {
        rw_ResetLoRaWAN();
    } else {
        out_error(RAK_ARG_ERR);
        return; 
    }
    e_printf("OK\r\n");
}

static void start_boot(int argc, char *argv[])
{
    __disable_irq();
    //set_boot_config(CONFIG1_BOOT_E);
    e_printf("OK\r\n");
    rw_ResetMCU(0);
}

static void lora_reload(int argc, char *argv[])
{  
    rw_restore_LoRaWAN_config(LORAMAC_REGION_DEFAULT, 1);
    rw_ResetLoRaWAN();
    e_printf("OK\r\n");
}

static void lora_read_config(int argc, char *argv[])
{
    int ret;
    char *out;

    
    if (argc != 2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    ret = read_config(argv[1], &out);
    if (ret < 0) {
        out_error(RAK_ARG_NOT_FIND);
    } else {
		e_printf("OK\r\n");
#if DEBUG_FW 		
		e_printf("%s=",argv[1]);
#endif				
        e_printf_raw(out, ret);
        e_printf_raw("\r\n", 2);
    }

}

static void lora_write_config(int argc, char *argv[])
{
    int ret;
    
    if (argc < 2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    ret = write_config(argv[1]);
    if (ret < 0) {
        out_error(ret);
    } else {
        e_printf("OK\r\n");
    }
}

static void lora_tx_dr(int argc, char *argv[])
{
    int  dr;
    int  ret;
    
    if (argc == 1) {
      e_printf("OK%d\r\n", rw_GetTxDataRate());
    } 
    else if (argc == 2) 
    {
      dr = atoi(argv[1]);
      
      MibRequestConfirm_t mibReq;
      mibReq.Type = MIB_CHANNELS_DATARATE;
      mibReq.Param.ChannelsDatarate = dr; 
      ret = LoRaMacMibSetRequestConfirm( &mibReq );
      if ( ret !=LORAMAC_STATUS_OK ) {
          out_error(RAK_ARG_ERR);
          return;       
      }
      
      rw_SetTxDataRate(dr);
      e_printf("OK\r\n"); 
    } else {
      out_error(RAK_ARG_ERR);
    }
    
    return;
}

static void lora_send(int argc, char *argv[])
{
    int ret;
    bool confirm; 
    int app_port; 
    int app_len;
    uint8_t *app_data = (uint8_t *)argv[3];
    char hex_num[3] = {0};
    
    if (argc != 4) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    if (strcmp(argv[1], "0") == 0) {
        confirm = false;
    } else if (strcmp(argv[1], "1") == 0) {
        confirm = true;
    } else {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    app_port = atoi(argv[2]);
    
    if (app_port < 1 || app_port > 223) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    app_len = strlen(app_data);
	
    if (app_len > 128) {
        out_error(RAK_ARG_ERR);
        return;
    } 
	
    if (app_len%2) {
        out_error(RAK_ARG_ERR);
        return;
    }
    
    for (int i = 0; i < app_len; i++) {
        if (!isxdigit(app_data[i])) {
            out_error(RAK_ARG_ERR);
            return;   
        }
    }
    app_len = app_len/2;
    for (int i = 0; i < app_len; i++) {
        memcpy(hex_num, &app_data[i*2], 2);
        app_data[i] = strtoul(hex_num, NULL, 16);
    }    
    
    ret = rw_LoRaTxData(confirm, app_port, app_len, app_data);
    if (ret != 0) {
        if (ret == LORAMAC_STATUS_BUSY) {
            ret = RAK_MAC_BUSY_ERR;
        } else if (ret == LORAMAC_STATUS_NO_NETWORK_JOINED) {
            ret = RAK_NOT_JOIN;
        } else if (ret == LORAMAC_STATUS_LENGTH_ERROR) {
            ret = RAK_TX_LEN_LIMITE_ERR;
        } else {
            ret = RAK_TX_ERR;
        }
        out_error(ret);
        return;
    }
    e_printf("OK\r\n");   
}



void lora_recv(uint8_t event, uint8_t port, uint16_t size, char *buffer)
{
    char hex_str[3] = {0};
    
    e_printf("at+recv=%d,%d,%d", event, port, size);
    if ((buffer != NULL) && size) {
        e_printf(",");
        for (int i = 0; i < size; i++) {
            sprintf(hex_str, "%02x", buffer[i]);
            e_printf("%s", hex_str); 
        }
    }
    e_printf_raw("\r\n", 2);
}

void lora_recv_ex(uint8_t event, uint8_t port, int16_t Rssi, int8_t snr, uint16_t size,  char *buffer)
{
    char hex_str[3] = {0};
    
    if (g_lora_system.recv_rssi_en==0) {   
      e_printf("at+recv=%d,%d,%d", event, port, size);
    } else {
      e_printf("at+recv=%d,%d,%d,%d,%d", event, port, Rssi, snr, size);
    }
    
    if ((buffer != NULL) && size) {
        e_printf(",");
        for (int i = 0; i < size; i++) {
            sprintf(hex_str, "%02x", buffer[i]);
            e_printf("%s", hex_str); 
        }
    }
    e_printf_raw("\r\n", 2);
}



static void set_boot_config(uint32_t cmd)
{
#if 0
    uint32_t u32Configdata0, u32Configdata1;
    uint32_t config0_temp, config1_temp;
    uint32_t time_out = 0;
    uint8_t counter = 0;
    
    UNLOCKREG();
    __disable_irq();
    FMC->ISPCON |= FMC_ISPCON_ISPEN | FMC_ISPCON_CFGUEN;
    while (FMC_Read(CONFIG0, &u32Configdata0) != 0) {
        time_out++;
        if (time_out > 0x100)
            rw_ResetMCU(0);
    }
    time_out = 0;
    while (FMC_Read(CONFIG1, &u32Configdata1) != 0) {
        time_out++;
        if (time_out > 0x100)
            rw_ResetMCU(0);
    }
    u32Configdata1 &= (cmd & CONFIG1_MASK);

    do {
        time_out = 0;
        while (FMC_WriteConfig(u32Configdata0, u32Configdata1) != 0) {
            time_out++;
            if (time_out > 0x100)
                rw_ResetMCU(0);
        }
        time_out = 0;
        while (FMC_Read(CONFIG0, &config0_temp) != 0) {
            time_out++;
            if (time_out > 0x100)
                rw_ResetMCU(0);
        }
        time_out = 0;
        while (FMC_Read(CONFIG1, &config1_temp) != 0) {
            time_out++;
            if (time_out > 0x100)
                rw_ResetMCU(0);
        }
        if (config0_temp != u32Configdata0 || config1_temp != u32Configdata1) {
            counter++;
            if (counter > 10)
                rw_ResetMCU(0);
            else
                continue;
        }
        else
            break;
    }while(1);

    FMC->ISPCON &= ~(FMC_ISPCON_ISPEN | FMC_ISPCON_CFGUEN);
    DPRINTF("CONFIG0 = 0X%X\r\n",u32Configdata0);
    DPRINTF("CONFIG1 = 0X%X\r\n",u32Configdata1);
    __enable_irq();
    LOCKREG();
#endif
}


static void lora_uart_config(int argc, char *argv[])
{
    int ret;
    uart_config_t value;
        
    if (argc == 1) {
        ret = read_partition(PARTITION_1, (char *)&value, sizeof(value));
        if (ret < 0) {
            ret = RAK_INTER_ERR;
        } 
        
        if (value.stopBits == UART_2_STOP_BIT) {
            value.stopBits = (StopBits_t) 1;
        }
        
        e_printf("OK%d,%d,%d,%d,%d\r\n",  value.baudrate, 
                                          value.wordLength + 8,     
                                          value.parity,
                                          value.stopBits,
                                          value.flowCtrl?1:0);
        return;
    } else {
        if (argc != 6) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }
        memset(&value, 0, sizeof(value));
        
        value.baudrate = atoi(argv[1]);
        if (!(CHECK_UART_BAUD(value.baudrate))) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }
        
        value.wordLength = (WordLength_t) atoi(argv[2]);
        if (value.wordLength == 8) {
            value.wordLength = UART_8_BIT;
        }
        
        if (!(CHECK_UART_DATABIT(value.wordLength))) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }        
        
        value.parity = (Parity_t) atoi(argv[3]);
        
        if (!(CHECK_UART_PARITY(value.parity))) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }
        value.stopBits = (StopBits_t) atoi(argv[4]);
        
        if (value.stopBits == 1) {
            value.stopBits = UART_2_STOP_BIT;
        }
        
        if (!(CHECK_UART_STOPBIT(value.stopBits))) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }
        
        value.flowCtrl = (FlowCtrl_t) atoi(argv[5]);
        
        if (!(CHECK_UART_FLOW(value.flowCtrl))) {
            ret = RAK_ARG_ERR;
            goto ERROR;
        }
        
        if (value.flowCtrl) {
            value.flowCtrl = RTS_CTS_FLOW_CTRL;
        }
        
        write_partition(PARTITION_1, (char *)&value, sizeof(value));
        e_printf("OK\r\n");
        return;
    }
ERROR:
    out_error(ret);
}

static void lora_gpio(int argc, char *argv[])
{
    Gpio_t  gpio_pin;
    int     ret = 0 ;
    uint8_t pin;  
    uint8_t pinVal;
    
    if (argc == 2 || argc == 3 ) 
    {   
      pin = atoi(argv[1]);
      if ( pin > 32)
      {
          ret = RAK_ARG_ERR;
          goto ERROR;      
      }
      
      if (!(RAK811_PINS_MASK & (0x00000001 << (pin-1) )))
      {
          ret = RAK_ARG_ERR;
          goto ERROR;
      }  
    } else {
          ret = RAK_ARG_ERR;
          goto ERROR;
    }
    
    if (argc == 2) {   
        
      GpioInit( &gpio_pin, RAK811_pin_array[pin-1], PIN_INPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
      e_printf("OK%d\r\n", GpioRead( &gpio_pin));
      return;
    }
    else 
    { 
      GpioInit( &gpio_pin, RAK811_pin_array[pin-1], PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
      pinVal = atoi(argv[2]) ;
      if (pinVal >1) {
          ret = RAK_ARG_ERR;
          goto ERROR;      
      }
      GpioWrite( &gpio_pin, pinVal); 
      e_printf("OK\r\n");
      return;
    } 
    
ERROR:
    out_error(ret);
}

static void lora_rd_adc(int argc, char *argv[])
{
    Adc_t    Adc;
    PinNames pin; 
    int      ret = 0 ;
         
    if (argc == 2) {   
      
      pin = atoi(argv[1]);
      if ( pin > 32)
      {
          ret = RAK_ARG_ERR;
          goto ERROR;      
      }
      
      if (!(RAK811_ADC_PINS_MASK & (0x00000001 << (pin-1) )))
      {
          ret = RAK_ARG_ERR;
          goto ERROR;
      }  
      
      AdcInit( &Adc, RAK811_pin_array[pin-1]);
      e_printf("OK%d\r\n", AdcReadChannel( &Adc, RAK811_adc_pin_map[pin-1] ));
      return;
    }
    else 
    { 
       ret = RAK_ARG_ERR;
       goto ERROR;
    } 
    
ERROR:
    out_error(ret);
}
//at+rd_reg=<data_addr>[,<len>]
static void lora_rd_reg(int argc, char *argv[])
{
    I2c_t   i2c;
    int     ret = 0 ;
   uint8_t  dev_buf[64];
   uint16_t  addr;
   uint16_t  size;
   
    if (argc == 2 || argc == 3) {   
         
      ret = strtoul(argv[1], NULL, 16);
      if (ret > 0) {
        addr = ret;
        //e_printf("addr: %02x \n", ret);
      } else {
         ret = RAK_ARG_ERR;
         goto ERROR;       
      }
      
      if (argc == 3) {
         size = atoi(argv[2]);
      } else {
         size = 1;
      }
      
      if (size > 64) {
         ret = RAK_ARG_ERR;
         goto ERROR;      
      }

      SX1276ReadBuffer( addr, dev_buf, size );
      e_printf_raw("OK", 2);
      out_hex_buf( dev_buf, size);
      e_printf_raw("\r\n", 2);
   
      return;
    }
    else 
    { 
       ret = RAK_ARG_ERR;
       goto ERROR;
    } 
    
ERROR:
    out_error(ret);
}

static void lora_wr_reg(int argc, char *argv[])
{
  I2c_t   i2c;
  int     ret = 0 ;
  uint8_t*  app_data = (uint8_t* )argv[2];
  uint8_t   app_len = 0;
  uint16_t  addr;

  char hex_num[3] = {0};
  
  if (argc == 3) {   
       
    ret = strtoul(argv[1], NULL, 16);
    if (ret > 0) {
      addr = ret;
      //e_printf("addr: %02x \n", ret);
    } else {
      ret = RAK_ARG_ERR;
      goto ERROR;       
    }
    
    app_len = strlen(app_data);
    
    if (app_len > 128) {
      out_error(RAK_ARG_ERR);
      return;
    } 
    
    if (app_len%2) {
      out_error(RAK_ARG_ERR);
      return;
    }
    
    for (int i = 0; i < app_len; i++) {
      if (!isxdigit(app_data[i])) {
        out_error(RAK_ARG_ERR);
        return;   
      }
    }
    app_len = app_len/2;
    for (int i = 0; i < app_len; i++) {
      memcpy(hex_num, &app_data[i*2], 2);
      app_data[i] = strtoul(hex_num, NULL, 16);
    }   
    
    SX1276WriteBuffer( addr, app_data, app_len );  
    e_printf("OK\r\n");

    return;
  }
  else 
  { 
    ret = RAK_ARG_ERR;
    goto ERROR;
  } 
  
ERROR:
  out_error(ret);
}

//at+rd_iic=<dev_addr>,<data_addr>[,<len>]
static void lora_rd_iic(int argc, char *argv[])
{
    I2c_t   i2c;
    int     ret = 0 ;
   uint8_t  dev_buf[64];
   uint8_t   deviceAddr;
   uint16_t  addr;
   uint16_t  size;
   
    if (argc == 3 || argc == 4) {   
      
      ret = strtoul(argv[1], NULL, 16);
      if (ret > 0) {
        deviceAddr = ret;
        //e_printf("deviceAddr: %02x ", ret);
      } else {
         ret = RAK_ARG_ERR;
         goto ERROR;       
      }
       
      ret = strtoul(argv[2], NULL, 16);
      if (ret > 0) {
        addr = ret;
        //e_printf("addr: %02x \n", ret);
      } else {
      	 if(strcmp(argv[2],"00") == 0 || strcmp(argv[2],"0") == 0){
                addr = 0;
      	 } else {
                ret = RAK_ARG_ERR;
                goto ERROR; 
      	 }
      }
      
      if (argc == 4) {
         size = atoi(argv[3]);
      } else {
         size = 1;
      }
      
      if (size > 64) {
         ret = RAK_ARG_ERR;
         goto ERROR;      
      }
      
      I2cResetBus( &i2c );
      ret = I2cReadBuffer( &i2c, deviceAddr, addr, dev_buf, size );
      if (ret ==SUCCESS) {
          e_printf_raw("OK", 2);
          out_hex_buf( dev_buf, size);
          e_printf_raw("\r\n", 2);
      } else {
         ret = RAK_UNKNOWN_ERR;
         goto ERROR;      
      }
      
      return;
    }
    else 
    { 
       ret = RAK_ARG_ERR;
       goto ERROR;
    } 
    
ERROR:
    out_error(ret);
}

static void lora_wr_iic(int argc, char *argv[])
{
  I2c_t   i2c;
  int     ret = 0 ;
  uint8_t*  app_data = (uint8_t* )argv[3];
  uint8_t   app_len = 0;
  uint8_t   deviceAddr;
  uint16_t  addr;
  char hex_num[3] = {0};
  
  if (argc == 4) {   
    
    ret = strtoul(argv[1], NULL, 16);
    if (ret > 0) {
      deviceAddr = ret;
      //e_printf("deviceAddr: %02x ", ret);
    } else {
      ret = RAK_ARG_ERR;
      goto ERROR;       
    }
    
    ret = strtoul(argv[2], NULL, 16);
    if (ret > 0) {
      addr = ret;
      //e_printf("addr: %02x \n", ret);
    } else {
      if(strcmp(argv[2],"00") == 0 || strcmp(argv[2],"0") == 0){
            addr = 0;
      } else {
            ret = RAK_ARG_ERR;
            goto ERROR; 
      }      
    }
    
    app_len = strlen(app_data);
    
    if (app_len > 128) {
      out_error(RAK_ARG_ERR);
      return;
    } 
    
    if (app_len%2) {
      out_error(RAK_ARG_ERR);
      return;
    }
    
    for (int i = 0; i < app_len; i++) {
      if (!isxdigit(app_data[i])) {
        out_error(RAK_ARG_ERR);
        return;   
      }
    }
    app_len = app_len/2;
    for (int i = 0; i < app_len; i++) {
      memcpy(hex_num, &app_data[i*2], 2);
      app_data[i] = strtoul(hex_num, NULL, 16);
    }   
    
    I2cResetBus( &i2c );
    ret = I2cWriteBuffer( &i2c, deviceAddr, addr, app_data, app_len );   
    if (ret ==SUCCESS) {
      e_printf("OK\r\n");
    } else {
      ret = RAK_UNKNOWN_ERR;
      goto ERROR;      
    }
    
    return;
  }
  else 
  { 
    ret = RAK_ARG_ERR;
    goto ERROR;
  } 
  
ERROR:
  out_error(ret);
}

static void lora_recv_set(int argc, char *argv[])
{
   uint8_t recv_en = 0;
     
   if (argc > 2) {
        out_error(RAK_ARG_ERR);
        return;
    }

    if (argc ==1) {
	 e_printf("OK%d\r\n", g_lora_system.recv_rssi_en);
	 return;
    }
    recv_en = atoi(argv[1]);
   if (recv_en ==0 || recv_en ==1) {
      g_lora_system.recv_rssi_en = recv_en;
      e_printf("OK\r\n");
   } else {
      out_error(RAK_ARG_ERR);
      return;
   }
		   
}

static void lora_link_cnt(int argc, char *argv[])
{
     
  if (argc == 1) {
    
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_UPLINK_COUNTER;
    
    if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
    {
      g_lora_system.up_cnt = mibReq.Param.UpLinkCounter;
    }
    
    mibReq.Type = MIB_DOWNLINK_COUNTER;
    if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
    {
      g_lora_system.down_cnt = mibReq.Param.DownLinkCounter;
    }   
    
    e_printf("OK%d,%d\r\n", g_lora_system.up_cnt, g_lora_system.down_cnt);
  } 
  else if (argc == 3){
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_UPLINK_COUNTER;
    mibReq.Param.UpLinkCounter =  atoi(argv[1]);
      
    if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
    {
    }
    
    mibReq.Type = MIB_DOWNLINK_COUNTER;
    mibReq.Param.DownLinkCounter =  atoi(argv[2]);
     
    if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
    {
    }   
    
    e_printf("OK\r\n"); 
  } else {
     out_error(RAK_ARG_ERR);
  }
  
}

static void lora_abp_info(int argc, char *argv[])
{
     
  if (argc == 1) {
    
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NET_ID;
    LoRaMacMibGetRequestConfirm( &mibReq );
    e_printf("OK%x,", mibReq.Param.NetID );
  
    mibReq.Type = MIB_DEV_ADDR;
    LoRaMacMibGetRequestConfirm( &mibReq );
    e_printf("%x,", mibReq.Param.DevAddr);
    
    mibReq.Type = MIB_NWK_SKEY;
    LoRaMacMibGetRequestConfirm( &mibReq );
    out_hex_buf( mibReq.Param.NwkSKey, 16);
    e_printf(",");
    
    mibReq.Type = MIB_APP_SKEY;
    LoRaMacMibGetRequestConfirm( &mibReq );
    out_hex_buf( mibReq.Param.AppSKey, 16);
    e_printf("\r\n");
  } 
  else {
     out_error(RAK_ARG_ERR);
  }
  
}

void GPIOIRQ_Enable(void)
{
	HAL_NVIC_EnableIRQ( EXTI0_IRQn );
	HAL_NVIC_EnableIRQ( EXTI1_IRQn );
	HAL_NVIC_EnableIRQ( EXTI2_IRQn );
	HAL_NVIC_EnableIRQ( EXTI3_IRQn );
	HAL_NVIC_EnableIRQ( EXTI4_IRQn );
	HAL_NVIC_EnableIRQ( EXTI9_5_IRQn );
	HAL_NVIC_EnableIRQ( EXTI15_10_IRQn );
}
static void lora_sleep(int argc, char *argv[])
{
   
    if (argc != 1) {
        out_error(RAK_ARG_ERR);
    } else {
        e_printf("OK\r\n");
        DelayMs(10);
        
        SX1276SetSleep();
        SX1276Write(REG_OPMODE,SX1276Read(REG_OPMODE)& 0xF8);
          
        __HAL_RCC_RTC_DISABLE();
        
        __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);

          
        BoardDeInitMcu();
        SysEnterUltraPowerStopMode();
        BoardInitMcu();
        
        __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
        
        __HAL_RCC_RTC_ENABLE();

        SX127X_INIT();
			
				GPIOIRQ_Enable();
        
        rw_LoadUsrConfig(); 
        
        lora_recv(LORA_EVENT_WAKEUP, 0, 0, NULL);
        
        UartFlush(&Uart1);
        
    }
}

void enter_sleep()
{    
        __HAL_RCC_RTC_DISABLE();
        
        __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);
        
        BoardDeInitMcu();
        SysEnterUltraPowerStopMode();
        BoardInitMcu();
        
        __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
        
        __HAL_RCC_RTC_ENABLE();

        lora_recv(LORA_EVENT_WAKEUP, 0, 0, NULL);
        
        UartFlush(&Uart1);     
}
