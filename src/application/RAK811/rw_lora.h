#ifndef _RW_LORA_H_
#define _RW_LORA_H_


int rw_InitLoRaWAN(void);
int rw_DeInitLoRaWAN(void);
int rw_ResetLoRaWAN(void);
void rw_ReadUsrConfig(void);
void rw_LoadUsrConfig(void);
int rw_JoinNetworkABP(uint32_t *dev_addr, uint8_t *nwks_key, uint8_t *apps_key);
int rw_JoinNetworkOTAA(uint8_t *dev_eui, uint8_t *app_eui, uint8_t *app_key, uint8_t nb_trials);
int rw_LoRaTxData(bool confirm, uint8_t app_port, uint16_t app_len, uint8_t *app_data);
void rw_GetVersion(char *ver);
void rw_ResetMCU(uint8_t mode);
int rw_LoRaLinkCheck(void);
char* rw_Region2Str(LoRaMacRegion_t region);
LoRaMacRegion_t rw_Str2Region(char* region);
int rw_restore_LoRaWAN_config(LoRaMacRegion_t region, uint8_t No_retain);

/***Lora P2P add***/
int rw_LoRaP2PTxContinue(uint16_t counts, uint32_t intervalMs, uint16_t tx_len, uint8_t *tx_data);
int rw_LoRaP2PRxContinue(uint8_t report);
void rw_LoRaP2PTxStop(void);
void rw_LoRaP2PRxStop(void);

#endif