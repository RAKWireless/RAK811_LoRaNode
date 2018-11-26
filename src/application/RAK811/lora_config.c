
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "app.h"

#define MAX_ARGV        6

typedef enum {
  CFG_READ,
  CFG_WRITE
}cfg_op;

static int handle_dev_addr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_dev_eui(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_app_eui(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_app_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_nwks_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_apps_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_tx_power(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_txpwr_level(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_adr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_def_tx_dr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_rx_delay1(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_rx2(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_channel_list(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_max_channels(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_channel_mask(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_down_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_up_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_public_network(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_join_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_nb_trans(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_ack_retrans(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_duty_cycle(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
static int handle_lorawan_class(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);

/** Structure for registering CONFIG commands */
struct config_cmd {
    /** The name of the CONFIG command */
    char *name;
    /** The help text associated with the command */
    //const char *help;
    /** The function that should be invoked for this command. */
    int (*function) (lora_config_t *config, int argc, char *argv[], char *in, cfg_op op);
};

struct config_cmd config_cmds[] = {
                                       "dev_addr",              handle_dev_addr,
                                       "dev_eui",               handle_dev_eui,
                                       "app_eui",               handle_app_eui,
                                       "app_key",               handle_app_key,
                                       "nwks_key",              handle_nwks_key,
                                       "apps_key",              handle_apps_key,
                                       "tx_power",              handle_tx_power,
                                       "pwr_level",             handle_txpwr_level,
                                       "adr",                   handle_adr,
                                       "dr",                    handle_def_tx_dr,
                                       "public_net",            handle_public_network,
                                       "rx_delay1",             handle_rx_delay1,
                                       "ch_list",               handle_channel_list,
                                       "ch_mask",               handle_channel_mask,
                                       "max_chs",               handle_max_channels,
                                       "rx2",                   handle_rx2,
                                       "join_cnt",              handle_join_cnt,
                                       "nbtrans",               handle_nb_trans,
                                       "retrans",               handle_ack_retrans,
                                       "duty",                  handle_duty_cycle,
                                       "class",                 handle_lorawan_class,               
                                       //"cnt_up",                handle_up_cnt,
                                       //"cnt_down",              handle_down_cnt,
                                  };

char config_buf[1024];

static int parse_args(char* str, char* argv[], char **end)
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

        while(*ch != ',' && *ch != '\0' && *ch != '&') {
            if(*ch == ':' && i == 1) {
                break;
            }
            else
                ch++;
        }
        
        if (*ch == '&') {
            *ch = '\0';
            *end = ++ch;
            break;
        } else if (*ch == '\0'){
            *end = NULL;
            break;
        }
        
        *ch = '\0';
        ch++;
        while(*ch == ',') {
            ch++;
        }
    }
    return i;
}

static int read_config_string(lora_config_t *config, const char *in, char *out)
{
    int i;
    int ret;
    char *buffer;
    
    if (!out) {
        return -1;
    }
    buffer = out;
    
    if (in != NULL) {
        for (i = 0; i < sizeof(config_cmds)/sizeof(struct config_cmd); i++) {
            if (strcmp(in, config_cmds[i].name) == 0) {
                ret = config_cmds[i].function(config, 0, &buffer, NULL, CFG_READ);
                if (ret < 0) {
                    return ret;
                }
                break;
            }
        }  
        if (i == sizeof(config_cmds)/sizeof(struct config_cmd)) {
            return RAK_ARG_ERR;
        }  
        buffer = out + ret;
    } else {
        for (i = 0; i < sizeof(config_cmds)/sizeof(struct config_cmd); i++) {
            ret = config_cmds[i].function(config, 0, &buffer, config_cmds[i].name, CFG_READ);
            if (ret < 0) {
                return ret;
            }
            if (ret > 0 && i < sizeof(config_cmds)/sizeof(struct config_cmd) - 1) {
                buffer[ret++] = '&';
            }
            buffer += ret; 
        }    
    }
    
    return (buffer - out);
}

static int write_config_string(lora_config_t *config, char *in)
{
    int i;
    int ret;
    int argc;
    char *argv[MAX_ARGV];
    char *end;
    
    do {
        argc = parse_args(in, argv, &end);
        if (argc < 2) {
            return -1;
        }
        in = end;
        for (i = 0; i < sizeof(config_cmds)/sizeof(struct config_cmd); i++) {
            if (strcmp(argv[0], config_cmds[i].name) == 0) {
                ret = config_cmds[i].function(config, argc - 1, &argv[1], NULL, CFG_WRITE);
                if (ret < 0) {
                    return ret;
                }
                break;
            }
        }  
        if (i == sizeof(config_cmds)/sizeof(struct config_cmd)) {
            return -1;
        }
    }while (end != NULL);
    
    return 0; 
}


int write_config(char *in)
{
    int ret;
    lora_config_t config;
    
    ret = read_partition(PARTITION_0, (char *)&config, sizeof(config));
    if (ret < 0) {
        return ret;
    }
    ret = write_config_string(&config, in);
    if (ret < 0) {
        return ret;
    }
    
    ret = write_partition(PARTITION_0, (char *)&config, sizeof(config));
    return ret;
}

int read_config(const char *in, char **out)
{
    int ret;
    lora_config_t config;
    
    ret = read_partition(PARTITION_0, (char *)&config, sizeof(config));
    if (ret < 0) {
        return ret;
    }
    
    ret = read_config_string(&config, in, config_buf);
    if (ret < 0) {
        return ret;
    }
    
    *out = config_buf;
    return ret;
}

static int handle_dev_addr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    //(void)test;
    
    if (op == CFG_READ) {
        int len = 0;
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->dev_addr)) {
            if (config->dev_addr[i] == 0xff) {
                a++;
            }
            
            if (config->dev_addr[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->dev_addr) || b == sizeof(config->dev_addr)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }        
        
        len += sprintf(buffer + len, "%02X%02X%02X%02X", 
                       config->dev_addr[3],config->dev_addr[2],
                       config->dev_addr[1],config->dev_addr[0]);
        ret = len;
    } else {
        uint32_t dev_addr;
        
        if (argc != 1) {
            goto END;
        }
        
        dev_addr = strtoul(buffer, NULL, 16);

        memcpy(config->dev_addr, &dev_addr, sizeof(dev_addr));
        
        ret = RAK_OK;
    }
END:
    return ret;
}


static int handle_dev_eui(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }   
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->dev_eui)) {
            if (config->dev_eui[i] == 0xff) {
                a++;
            }
            
            if (config->dev_eui[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->dev_eui) || b == sizeof(config->dev_eui)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }              
        
        len += sprintf(buffer + len, "%02X%02X%02X%02X%02X%02X%02X%02X", 
                       config->dev_eui[0],config->dev_eui[1],
                       config->dev_eui[2],config->dev_eui[3],
                       config->dev_eui[4],config->dev_eui[5],
                       config->dev_eui[6],config->dev_eui[7]);
        ret = len;
    } else {
        char hex_num[3] = {0};
        if (argc != 1) {
            goto END;
        }  
        
        if (strlen(buffer) != 16) {
            goto END;
        }
        for (int i = 0; i < 16; i++) {
            if (!isxdigit(buffer[i])) {
                goto END;    
            }
        }
        for (int i = 0; i < 8; i++) {
            memcpy(hex_num, &buffer[i*2], 2);
            config->dev_eui[i] = strtoul(hex_num, NULL, 16);
        }
        ret = RAK_OK;
    }
END:    
    return ret;
}

static int handle_app_eui(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;    
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->app_eui)) {
            if (config->app_eui[i] == 0xff) {
                a++;
            }
            
            if (config->app_eui[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->app_eui) || b == sizeof(config->app_eui)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }  
        
        len += sprintf(buffer + len, "%02X%02X%02X%02X%02X%02X%02X%02X", 
                       config->app_eui[0],config->app_eui[1],
                       config->app_eui[2],config->app_eui[3],
                       config->app_eui[4],config->app_eui[5],
                       config->app_eui[6],config->app_eui[7]);
        ret = len;
    } else {
        char hex_num[3] = {0};
        if (argc != 1) {
            goto END;
        }        
        
        if (strlen(buffer) != 16) {
            goto END;
        }
        for (int i = 0; i < 16; i++) {
            if (!isxdigit(buffer[i])) {
                goto END;    
            }
        }
        for (int i = 0; i < 8; i++) {
            memcpy(hex_num, &buffer[i*2], 2);
            config->app_eui[i] = strtoul(hex_num, NULL, 16);
        }
        ret = RAK_OK;
    }

END:    
    return ret;
}

static int handle_app_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->app_key)) {
            if (config->app_key[i] == 0xff) {
                a++;
            }
            
            if (config->app_key[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->app_key) || b == sizeof(config->app_key)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }
        
        len += sprintf(buffer + len, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
                       config->app_key[0],config->app_key[1],
                       config->app_key[2],config->app_key[3],
                       config->app_key[4],config->app_key[5],
                       config->app_key[6],config->app_key[7],
                       config->app_key[8],config->app_key[9],
                       config->app_key[10],config->app_key[11],
                       config->app_key[12],config->app_key[13],
                       config->app_key[14],config->app_key[15]);
        ret = len;
    } else {
        char hex_num[3] = {0};
        if (argc != 1) {
            goto END;
        }
        
        if (strlen(buffer) != 32) {
            goto END;
        }
        for (int i = 0; i < 32; i++) {
            if (!isxdigit(buffer[i])) {
                goto END;    
            }
        }
        for (int i = 0; i < 16; i++) {
            memcpy(hex_num, &buffer[i*2], 2);
            config->app_key[i] = strtoul(hex_num, NULL, 16);
        }
        ret = RAK_OK;
    }

END:    
    return ret;
}


static int handle_nwks_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{

    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->nwks_key)) {
            if (config->nwks_key[i] == 0xff) {
                a++;
            }
            
            if (config->nwks_key[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->nwks_key) || b == sizeof(config->nwks_key)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }
        
        len += sprintf(buffer + len, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
                       config->nwks_key[0],config->nwks_key[1],
                       config->nwks_key[2],config->nwks_key[3],
                       config->nwks_key[4],config->nwks_key[5],
                       config->nwks_key[6],config->nwks_key[7],
                       config->nwks_key[8],config->nwks_key[9],
                       config->nwks_key[10],config->nwks_key[11],
                       config->nwks_key[12],config->nwks_key[13],
                       config->nwks_key[14],config->nwks_key[15]);
        ret = len;
    } else {
        char hex_num[3] = {0};
        if (argc != 1) {
            goto END;
        }
        
        if (strlen(buffer) != 32) {
            goto END;
        }
        for (int i = 0; i < 32; i++) {
            if (!isxdigit(buffer[i])) {
                goto END;    
            }
        }
        for (int i = 0; i < 16; i++) {
            memcpy(hex_num, &buffer[i*2], 2);
            config->nwks_key[i] = strtoul(hex_num, NULL, 16);
        }
        ret = RAK_OK;
    }

END:    
    return ret;

}

static int handle_apps_key(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        int i = 0, a = 0, b = 0;
        
        while (i < sizeof(config->apps_key)) {
            if (config->apps_key[i] == 0xff) {
                a++;
            }
            
            if (config->apps_key[i] == 0x00) {
                b++;
            }  
            i++;
        }
        
        if (a == sizeof(config->apps_key) || b == sizeof(config->apps_key)) {
            if (in) {
                ret = RAK_OK;
            } else {
                ret = RAK_ARG_NOT_FIND;
            }
            goto END;
        }

        
        len += sprintf(buffer + len, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
                       config->apps_key[0],config->apps_key[1],
                       config->apps_key[2],config->apps_key[3],
                       config->apps_key[4],config->apps_key[5],
                       config->apps_key[6],config->apps_key[7],
                       config->apps_key[8],config->apps_key[9],
                       config->apps_key[10],config->apps_key[11],
                       config->apps_key[12],config->apps_key[13],
                       config->apps_key[14],config->apps_key[15]);
    
        ret = len;
    } else {
        char hex_num[3] = {0};
        if (argc != 1) {
            goto END;
        }  
        
        if (strlen(buffer) != 32) {
            goto END;
        }
        for (int i = 0; i < 32; i++) {
            if (!isxdigit(buffer[i])) {
                goto END;    
            }
        }
        for (int i = 0; i < 16; i++) {
            memcpy(hex_num, &buffer[i*2], 2);
            config->apps_key[i] = strtoul(hex_num, NULL, 16);
        }
        ret = RAK_OK;
    }

END:    
    return ret;
}


static int handle_tx_power(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int i;
    int power;
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;   

        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        len += sprintf(buffer + len, "%d", config->tx_power);
        
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }
        
        power = atoi(buffer);
            
        config->tx_power = power;
        
        ret = RAK_OK;
    }
END:
    return ret;
    
}

static int handle_txpwr_level(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int i;
    int power;
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;   

        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        len += sprintf(buffer + len, "%d", config->tx_pwr_level);
        
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }
        
        power = atoi(buffer);
        
        MibRequestConfirm_t mibReq;
  
        mibReq.Type = MIB_CHANNELS_DEFAULT_TX_POWER;
        mibReq.Param.ChannelsTxPower = power;
        if ( LORAMAC_STATUS_OK !=LoRaMacMibSetRequestConfirm( &mibReq )) {
           goto END;
        }
            
        config->tx_pwr_level = power;
        
        ret = RAK_OK;
    }
END:
    return ret;
    

}

static int handle_def_tx_dr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->def_tx_dr);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        int dr = atoi(argv[0]);
        
        MibRequestConfirm_t mibReq;
  
        mibReq.Type = MIB_CHANNELS_DEFAULT_DATARATE;
        mibReq.Param.ChannelsDatarate = dr;
        if ( LORAMAC_STATUS_OK !=LoRaMacMibSetRequestConfirm( &mibReq )) {
           goto END;
        }     
        config->def_tx_dr = dr;	    
        ret = RAK_OK;
    }
END:
    return ret;

}
static int handle_adr(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        char *switch_str[] = {"off", "on"};
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%s", switch_str[config->adr]);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        
        if (strcmp(buffer, "off") == 0) {
            config->adr = false;
        } else if (strcmp(buffer, "on") == 0) {
            config->adr = true;
        } else {
            goto END;
        }
        ret = RAK_OK;
    }
END:
    return ret;

}

static int handle_rx_delay1(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->rx_delay1);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        uint32_t delay = atoi(argv[0]);
        
        if (delay == 0) {
            goto END;
        }
        config->rx_delay1 = delay;
        ret = RAK_OK;
    }
END:
    return ret;

}

static int handle_max_channels(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->max_nb_chs);
        ret = len;
    } else {
       
    }
END:
    return ret;

}

static int handle_rx2(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0; 
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d,%ld", config->rx2.dr, config->rx2.freq);
        ret = len;
    } else {
        
        if (argc != 2) {
            goto END;
        }               
            
        int dr = atoi(argv[0]);
        uint64_t freq = atol(argv[1]);
        
        MibRequestConfirm_t mibReq;
 
        mibReq.Type = MIB_RX2_CHANNEL;  
        mibReq.Param.Rx2Channel.Frequency = freq;
        mibReq.Param.Rx2Channel.Datarate = dr;
        if ( LORAMAC_STATUS_OK !=LoRaMacMibSetRequestConfirm( &mibReq )) {
           goto END;
        } 
  
        config->rx2.freq = freq;
        config->rx2.dr = dr;
        ret = RAK_OK;
    }
END:
    return ret;
}


static int handle_join_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    VerifyParams_t verify;
    
    if (op == CFG_READ) {
        int len = 0; 
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->join_cnt);
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }               
        
        verify.NbJoinTrials = atoi(argv[0]);

        if( RegionVerify( rw_Str2Region(g_lora_config.region), &verify, PHY_NB_JOIN_TRIALS ) == false ) {
           goto END;
        }
               
        config->join_cnt = verify.NbJoinTrials;
        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_ack_retrans(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0; 
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->ack_retrans);
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }               
        
        uint8_t retry = atoi(argv[0]);

        if (retry < 1 || retry > 255) {
            goto END;
        }

        config->ack_retrans = retry;
        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_nb_trans(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0; 
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->nb_trans);
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }               
        
        uint8_t retry = atoi(argv[0]);

        if (retry < 1 || retry > 15) {
            goto END;
        }

        config->nb_trans = retry;
        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_channel_list(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    
    if (op == CFG_READ) {
        int len = 0; 

        if (in == NULL) {
            for ( uint8_t i = 0, k = 0; i < config->max_nb_chs; i += 16, k++) 
            {                    
              for ( uint8_t j = 0; j < 16; j++) 
              {            
                  if ( (config->ch_mask[k] & ( 1 << j )) != 0) {
                      len += sprintf(buffer+len, "%d,%s,%d,%d,%d", 
                                                  i+j, 
                                                  "on", 
                                                  config->ch_list[i+j].Frequency, 
                                                  config->ch_list[i+j].DrRange.Fields.Min,                                               
                                                  config->ch_list[i+j].DrRange.Fields.Max);
                  } else {
                      len += sprintf(buffer+len, "%d,%s", i+j, "off");
                  }
                  if ((i + j)< config->max_nb_chs - 1) {
                      buffer[len++] = ';';
                  } else {
                    break;
                  }
               }                
            }
        }
        ret = len;
    } else {
        
        if (argc != 5 && argc != 2) {
            goto END;
        }          
        
        int id = atoi(argv[0]);
        char *status = argv[1];
        
        
        if (id > config->max_nb_chs - 1) { 
            goto END;
        } 
        
        if (argc == 5) {
            uint32_t freq = atoi(argv[2]);
            int min = atoi(argv[3]);
            int max = atoi(argv[4]);
                      
            if (strcmp(status, "on") == 0) {
                config->ch_mask[id / 16] |= ( 1 << id % 16 );
                config->ch_list[id].Frequency = freq;
                config->ch_list[id].DrRange.Fields.Min = min;
                config->ch_list[id].DrRange.Fields.Max = max;  
                //Todo
            } else {
                goto END;
            }
        } else if (argc == 2) {
            if (strcmp(status, "off") == 0) {
               config->ch_mask[id / 16] &= ~( 1 << id % 16 );
            } else {
                goto END;
            }
        } 
        ret = RAK_OK;
    }
END:    
    return ret;
}



static int handle_channel_mask(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    uint8_t mask_len = ceil(((double)config->max_nb_chs)/16);
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0; 

        if (in == NULL) {
          
            for(int i = 0; i < mask_len; i++) { 

                len += sprintf(buffer+len, "%d,%04x", i, config->ch_mask[i]);
   
                if (i < mask_len - 1) {
                    buffer[len++] = ';';
                }                
            }
        }
        ret = len;
    } else {
        
        if (argc != 2 ) {
            goto END;
        }          
        
        int id = atoi(argv[0]);
        int mask = strtoul(argv[1], NULL, 16);
        if (mask < 0) {
             goto END;
        } 
      
        if (id > mask_len -1) {
            goto END;
        }   
        
        config->ch_mask[id] = mask;
         
        ret = RAK_OK;
    }
END:    
    return ret;
}



static int handle_up_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;   

        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        len += sprintf(buffer + len, "%d", config->up_cnt);
        
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }
        config->up_cnt = atoi(argv[0]);

        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_down_cnt(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;   

        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        len += sprintf(buffer + len, "%d", config->down_cnt);
        
        ret = len;
    } else {
        
        if (argc != 1) {
            goto END;
        }
        config->down_cnt = atoi(argv[0]);

        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_public_network(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        char *switch_str[] = {"off", "on"};
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%s", switch_str[config->public_network]);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        
        if (strcmp(buffer, "off") == 0) {
            config->public_network = false;
        } else if (strcmp(buffer, "on") == 0) {
            config->public_network = true;
        } else {
            goto END;
        }
        ret = RAK_OK;
    }
END:
    return ret;
}

static int handle_duty_cycle(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        char *switch_str[] = {"off", "on"};
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%s", switch_str[config->duty_cycle]);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        
        if (strcmp(buffer, "off") == 0) {
            config->duty_cycle = false;
        } else if (strcmp(buffer, "on") == 0) {
            config->duty_cycle = true;
        } else {
            goto END;
        }
        ret = RAK_OK;
    }
END:
    return ret;

}

static int handle_lorawan_class(lora_config_t *config, int argc, char *argv[], char *in, cfg_op op)
{
    int ret = RAK_ARG_ERR;
    char *buffer = argv[0];
    
    if (op == CFG_READ) {
        int len = 0;     
        char *switch_str[] = {"off", "on"};
        
        if (in) {
            len += sprintf(buffer + len, "%s:", in);
        }
        
        len += sprintf(buffer + len, "%d", config->loraWan_class);
        ret = len;
    } else {
        if (argc != 1) {
            goto END;
        }        
        e_printf("%s\r\n",argv[0]);
        config->loraWan_class = atoi(argv[0]);
				e_printf("%d\r\n",config->loraWan_class);
        if (config->loraWan_class <= CLASS_C) {
           ret = RAK_OK;
        }
    }
END:
    return ret;

}

