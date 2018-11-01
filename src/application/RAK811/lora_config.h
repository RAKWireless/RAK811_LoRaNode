#ifndef _LORA_CONFIG_H_
#define _LORA_CONFIG_H_


typedef struct {
    uint8_t dr;
    uint32_t freq;
}rx2_t;



typedef struct _lorap2p_param{
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Spreading factor
     * [6, 7, 8, 9, 10, 11, 12]
     */
    uint8_t  Spreadfact;	
    /*!
     * Bandwidth
     * [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]                              
     */
    uint8_t  Bandwidth; 
    /*!
     * Coding Rate
     * [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     */
    uint8_t  Codingrate; 
    /*!
     * Preamble Length
     * [5-65535]
     */
    uint16_t  Preamlen; 
    /*!
     * Power dbm
     * [5-20]
     */
    uint8_t  Powerdbm;	
}S_LORAP2P_PARAM;

typedef struct{
    uint8_t loraWan_class;
    uint8_t region[16]; //region string "EU868"
    uint8_t dev_addr[4];
    uint8_t dev_eui[8];
    uint8_t app_eui[8];
    uint8_t app_key[16];
    uint8_t nwks_key[16];
    uint8_t apps_key[16];
    
    ChannelParams_t ch_list[96];
    uint8_t   max_nb_chs;
    uint16_t  ch_mask[6]; 
    bool     public_network;
    bool     adr;
    bool     duty_cycle;
    uint8_t  tx_power;
    uint8_t  tx_pwr_level; // TX_POWER_0
    uint8_t  Rx1DrOffset;
    uint32_t rx_delay1;   
    rx2_t    rx2;
    uint8_t  def_tx_dr;
    uint8_t  join_cnt;   // region different
    uint8_t  nb_trans;  // 1-15
    uint8_t  ack_retrans;
    
    uint16_t up_cnt;
    uint16_t down_cnt;   
    /**Add junhua**/
    uint8_t  reserv[64];
    uint8_t  lora_mode;
    S_LORAP2P_PARAM   lorap2p_param;
    
}lora_config_t;


typedef struct{
    uint8_t recv_rssi_en;
    uint16_t up_cnt;
    uint16_t down_cnt;
   
}lora_system_t;

int read_config(const char *in, char **out);


#endif