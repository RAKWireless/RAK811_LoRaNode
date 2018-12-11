#include "board.h"
#include "LoRaMac.h"
#include "Comissioning.h"

#include "app.h"
#include "rw_lora.h"
#include "region/Region.h"
#include "rw_sys.h"
/*!
* Join requests trials duty cycle.
*/
#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE           10000000  // 10 [s] value in us

/*!
* Defines the application data transmission duty cycle. 5s, value in [us].
*/
#define APP_TX_DUTYCYCLE                            5000000

/*!
* Defines a random delay for application data transmission duty cycle. 1s,
* value in [us].
*/
#define APP_TX_DUTYCYCLE_RND                        1000000

/*!
* LoRaWAN confirmed messages
*/
//#define LORAWAN_CONFIRMED_MSG_ON                    false

/*!
* LoRaWAN Adaptive Data Rate
*
* \remark Please note that when ADR is enabled the end-device should be static
*/
#define LORAWAN_ADR_ON                              1

//#if defined( REGION_EU868 ) || defined( REGION_AS923 )

#include "LoRaMacTest.h"

/*!
* LoRaWAN ETSI duty cycle control enable/disable
*
* \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
*/
#define LORAWAN_DUTYCYCLE_ON                        false

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 ) 

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }

#endif

//#endif

/*!
* LoRaWAN application port
*/
#define LORAWAN_APP_PORT                            2

/*!
* User application data buffer size
*/

#define LORAWAN_APP_DATA_SIZE                       16

#if( OVER_THE_AIR_ACTIVATION != 0 )

static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#else
static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif

/*!
* Application port
*/
//static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
* User application data size
*/
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;

/*!
* User application data buffer size
*/
#define LORAWAN_APP_DATA_MAX_SIZE                           64

/*!
* User application data
*/
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
* Indicates if the node is sending confirmed or unconfirmed messages
*/
//static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

/*!
* Defines the application data transmission duty cycle
*/
//static uint32_t TxDutyCycleTime;

/*!
* Timer to handle the application data transmission duty cycle
*/
static TimerEvent_t LoraP2PTxNextPacketTimer;
void rw_P2PTxContinueTimerEvent( void );

/*!
* \brief Function to be executed on Radio Tx Done event
*/
static void rw_P2PTxDone( void );

/*!
* \brief Function to be executed on Radio Rx Done event
*/
static void rw_P2PRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
* \brief Function executed on Radio Tx Timeout event
*/
static void rw_P2PTxTimeout( void );

/*!
* \brief Function executed on Radio Rx error event
*/
static void rw_P2PRxError( void );

/*!
* \brief Function executed on Radio Rx Timeout event
*/
static void rw_P2PRxTimeout( void );

/*!
* Specifies the state of the application LED
*/
//static bool AppLedStateOn = false;


/*!
* Indicates Tx data rate
*/
static uint8_t  TxDataRate;

static uint16_t P2PTxCnts = 0;
static uint32_t P2PTxInterVal = 0;

static uint8_t  P2PRxReportEn = 0;
/*!
* LoRaWAN compliance tests support data
*/
struct ComplianceTest_s
{
  bool Running;
  uint8_t State;
  bool IsTxConfirmed;
  uint8_t AppPort;
  uint8_t AppDataSize;
  uint8_t *AppDataBuffer;
  uint16_t DownLinkCounter;
  bool LinkCheck;
  uint8_t DemodMargin;
  uint8_t NbGateways;
}ComplianceTest;

extern void lora_recv(uint8_t event, uint8_t port, uint16_t size, char *buffer);
extern void lora_recv_ex(uint8_t event, uint8_t port, int16_t Rssi, int8_t snr, uint16_t size, char *buffer);
void rw_SetTxDataRate(uint8_t dr);
uint8_t rw_GetTxDataRate(void);

///*!
// * \brief   Prepares the payload of the frame
// */
//static void PrepareTxFrame( uint8_t port )
//{
//    switch( port )
//    {
//    case 2:
//        {
//#if defined( REGION_EU868 )
//            static uint16_t pressure = 0;
//            int16_t altitudeBar = 0;
//            int16_t temperature = 0;
//            int32_t latitude, longitude = 0;
//            uint16_t altitudeGps = 0xFFFF;
//            uint8_t batteryLevel = 0;
//
//            pressure++;//( uint16_t )( MPL3115ReadPressure( ) / 10 );             // in hPa / 10
//            temperature = 0x32;//( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in ? * 100
//            altitudeBar = 0x33;//( int16_t )( MPL3115ReadAltitude( ) * 10 );           // in m * 10
//            batteryLevel = BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
//            //GpsGetLatestGpsPositionBinary( &latitude, &longitude );
//            altitudeGps = 0;//GpsGetLatestGpsAltitude( );                           // in m
//
//            AppData[0] = AppLedStateOn;
//            AppData[1] = ( pressure >> 8 ) & 0xFF;
//            AppData[2] = pressure & 0xFF;
//            AppData[3] = ( temperature >> 8 ) & 0xFF;
//            AppData[4] = temperature & 0xFF;
//            AppData[5] = ( altitudeBar >> 8 ) & 0xFF;
//            AppData[6] = altitudeBar & 0xFF;
//            AppData[7] = batteryLevel;
//            AppData[8] = ( latitude >> 16 ) & 0xFF;
//            AppData[9] = ( latitude >> 8 ) & 0xFF;
//            AppData[10] = latitude & 0xFF;
//            AppData[11] = ( longitude >> 16 ) & 0xFF;
//            AppData[12] = ( longitude >> 8 ) & 0xFF;
//            AppData[13] = longitude & 0xFF;
//            AppData[14] = ( altitudeGps >> 8 ) & 0xFF;
//            AppData[15] = altitudeGps & 0xFF;
//#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
//            int16_t temperature = 0;
//            int32_t latitude, longitude = 0;
//            uint16_t altitudeGps = 0xFFFF;
//            uint8_t batteryLevel = 0;
//        
//            temperature = 0;//( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in ? * 100
//        
//            batteryLevel = 0;//BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
//            //GpsGetLatestGpsPositionBinary( &latitude, &longitude );
//            altitudeGps = 0;// GpsGetLatestGpsAltitude( );                           // in m
//        
//            AppData[0] = AppLedStateOn;
//            AppData[1] = temperature;
//            AppData[2] = batteryLevel;
//            AppData[3] = ( latitude >> 16 ) & 0xFF;
//            AppData[4] = ( latitude >> 8 ) & 0xFF;
//            AppData[5] = latitude & 0xFF;
//            AppData[6] = ( longitude >> 16 ) & 0xFF;
//            AppData[7] = ( longitude >> 8 ) & 0xFF;
//            AppData[8] = longitude & 0xFF;
//            AppData[9] = ( altitudeGps >> 8 ) & 0xFF;
//            AppData[10] = altitudeGps & 0xFF;
//#endif
//        }
//        break;
//    case 224:
//        if( ComplianceTest.LinkCheck == true )
//        {
//            ComplianceTest.LinkCheck = false;
//            AppDataSize = 3;
//            AppData[0] = 5;
//            AppData[1] = ComplianceTest.DemodMargin;
//            AppData[2] = ComplianceTest.NbGateways;
//            ComplianceTest.State = 1;
//        }
//        else
//        {
//            switch( ComplianceTest.State )
//            {
//            case 4:
//                ComplianceTest.State = 1;
//                break;
//            case 1:
//                AppDataSize = 2;
//                AppData[0] = ComplianceTest.DownLinkCounter >> 8;
//                AppData[1] = ComplianceTest.DownLinkCounter;
//                break;
//            }
//        }
//        break;
//    default:
//        break;
//    }
//}

///*!
// * \brief   Prepares the payload of the frame
// *
// * \retval  [0: frame could be send, 1: error]
// */
//static bool SendFrame( void )
//{
//    McpsReq_t mcpsReq;
//    LoRaMacTxInfo_t txInfo;
//    
//    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
//    {
//        // Send empty frame in order to flush MAC commands
//        mcpsReq.Type = MCPS_UNCONFIRMED;
//        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
//        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
//        mcpsReq.Req.Unconfirmed.Datarate = DR_0;
//    }
//    else
//    {
//        if( IsTxConfirmed == false )
//        {
//            mcpsReq.Type = MCPS_UNCONFIRMED;
//            mcpsReq.Req.Unconfirmed.fPort = AppPort;
//            mcpsReq.Req.Unconfirmed.fBuffer = AppData;
//            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
//            mcpsReq.Req.Unconfirmed.Datarate = DR_0;
//        }
//        else
//        {
//            mcpsReq.Type = MCPS_CONFIRMED;
//            mcpsReq.Req.Confirmed.fPort = AppPort;
//            mcpsReq.Req.Confirmed.fBuffer = AppData;
//            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
//            mcpsReq.Req.Confirmed.NbTrials = 8;
//            mcpsReq.Req.Confirmed.Datarate = DR_0;
//        }
//    }
//
//    if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
//    {
//        return false;
//    }
//    return true;
//}



/*!
* \brief   MCPS-Confirm event function
*
* \param   [IN] McpsConfirm - Pointer to the confirm structure,
*               containing confirm attributes.
*/
static void McpsConfirm( McpsConfirm_t *McpsConfirm )
{
  DPRINTF("McpsConfirm\r\n");
  
  if( McpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
  {
    switch( McpsConfirm->McpsRequest )
    {
    case MCPS_UNCONFIRMED:
      {
        lora_recv(LORA_EVENT_TX_UNCOMFIRMED, 0, 0, NULL);
        //DPRINTF("MCPS_UNCONFIRMED\r\n");
        // Check Datarate
        // Check TxPower
        break;
      }
    case MCPS_CONFIRMED:
      {
        lora_recv(LORA_EVENT_TX_COMFIRMED, 0, 0, NULL);
        //DPRINTF("MCPS_CONFIRMED\r\n");
        // Check Datarate
        // Check TxPower
        // Check AckReceived
        // Check NbTrials
        break;
      }
    case MCPS_PROPRIETARY:
      {
        DPRINTF("MCPS_PROPRIETARY\r\n");
        break;
      }
    default:
      DPRINTF("MCPS_UNKNOWN\r\n");
      break;
    }
  } else if (McpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT){
    lora_recv(LORA_EVENT_RX2_TIMEOUT, 0, 0, NULL);
  } else if (McpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL) {
    lora_recv(LORA_EVENT_JOINED_FAILED, 0, 0, NULL);      
  } else if (McpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT){
    lora_recv(LORA_EVENT_TX_TIMEOUT, 0, 0, NULL);   
  } else if (McpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED){
    lora_recv(LORA_EVENT_DOWNLINK_REPEATED, 0, 0, NULL);           
  } else {
    DPRINTF("LORA_EVENT_UNKNOWN %d\r\n", McpsConfirm->Status);
    lora_recv(LORA_EVENT_UNKNOWN, 0, 0, NULL);
  } 
  //NextTx = true;
}

/*!
* \brief   MCPS-Indication event function
*
* \param   [IN] McpsIndication - Pointer to the indication structure,
*               containing indication attributes.
*/
static void McpsIndication( McpsIndication_t *McpsIndication )
{
  DPRINTF("McpsIndication\r\n");
  if( McpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
  {
    return;
  }
  
  switch( McpsIndication->McpsIndication )
  {
  case MCPS_UNCONFIRMED:
    {
      break;
    }
  case MCPS_CONFIRMED:
    {
      break;
    }
  case MCPS_PROPRIETARY:
    {
      break;
    }
  case MCPS_MULTICAST:
    {
      break;
    }
  default:
    break;
  }
  
  if( McpsIndication->RxData == true ) {
		lora_recv_ex(LORA_EVENT_RECV_DATA, McpsIndication->Port, McpsIndication->Rssi, McpsIndication->Snr, McpsIndication->BufferSize, McpsIndication->Buffer);
//    if (McpsIndication->Port == 224) {
//      lora_recv_ex(LORA_EVENT_RECV_DATA, McpsIndication->Port, McpsIndication->Rssi, McpsIndication->Snr, McpsIndication->BufferSize, McpsIndication->Buffer);
//    } else {
//      if(McpsIndication->BufferSize) {
//        lora_recv_ex(LORA_EVENT_RECV_DATA, McpsIndication->Port, McpsIndication->Rssi, McpsIndication->Snr, McpsIndication->BufferSize, McpsIndication->Buffer);
//      }   
//    }
  }
  return;
  
  
  // Check Multicast
  // Check Port
  // Check Datarate
  // Check FramePending
  // Check Buffer
  // Check BufferSize
  // Check Rssi
  // Check Snr
  // Check RxSlot
  
  if( ComplianceTest.Running == true )
  {
    ComplianceTest.DownLinkCounter++;
  }
  
  if( McpsIndication->RxData == true )
  {
    switch( McpsIndication->Port )
    {
    case 1: // The application LED can be controlled on port 1 or 2
    case 2:
      if( McpsIndication->BufferSize == 1 )
      {
        //AppLedStateOn = McpsIndication->Buffer[0] & 0x01;
        //GpioWrite( &Led3, ( ( AppLedStateOn & 0x01 ) != 0 ) ? 0 : 1 );
      }
      break;
    case 224:
      
      //e_printf("ComplianceTest Running=%d \n", ComplianceTest.Running);
      
      if( ComplianceTest.Running == false )
      {
        // Check compliance test enable command (i)
        if( ( McpsIndication->BufferSize == 4 ) && 
           ( McpsIndication->Buffer[0] == 0x01 ) &&
             ( McpsIndication->Buffer[1] == 0x01 ) &&
               ( McpsIndication->Buffer[2] == 0x01 ) &&
                 ( McpsIndication->Buffer[3] == 0x01 ) )
        {
          //IsTxConfirmed = false;
          //AppPort = 224;
          AppDataSize = 2;
          ComplianceTest.DownLinkCounter = 0;
          ComplianceTest.LinkCheck = false;
          ComplianceTest.DemodMargin = 0;
          ComplianceTest.NbGateways = 0;
          ComplianceTest.Running = true;
          ComplianceTest.State = 1;
          
          MibRequestConfirm_t mibReq;
          mibReq.Type = MIB_ADR;
          mibReq.Param.AdrEnable = true;
          LoRaMacMibSetRequestConfirm( &mibReq );
          
//#if defined( REGION_EU868 ) || defined( REGION_AS923 ) 
//          LoRaMacTestSetDutyCycleOn( false );
//#endif
          if (rw_Str2Region(g_lora_config.region) == LORAMAC_REGION_EU868 || rw_Str2Region(g_lora_config.region) == LORAMAC_REGION_AS923)
          {
            LoRaMacTestSetDutyCycleOn( false );
          }
        }
      }
      else
      {
        ComplianceTest.State = McpsIndication->Buffer[0];
        switch( ComplianceTest.State )
        {
        case 0: // Check compliance test disable command (ii)
          //IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
          //AppPort = LORAWAN_APP_PORT;
          AppDataSize = LORAWAN_APP_DATA_SIZE;
          ComplianceTest.DownLinkCounter = 0;
          ComplianceTest.Running = false;
          
          MibRequestConfirm_t mibReq;
          mibReq.Type = MIB_ADR;
          mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
          LoRaMacMibSetRequestConfirm( &mibReq );
//#if defined( REGION_EU868 ) || defined( REGION_AS923 )
//          LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
//#endif
          if (rw_Str2Region(g_lora_config.region) == LORAMAC_REGION_EU868 || rw_Str2Region(g_lora_config.region) == LORAMAC_REGION_AS923)
          {
            LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
          }
          break;
        case 1: // (iii, iv)
          AppDataSize = 2;
          break;
        case 2: // Enable confirmed messages (v)
          //IsTxConfirmed = true;
          ComplianceTest.State = 1;
          break;
        case 3:  // Disable confirmed messages (vi)
          //IsTxConfirmed = false;
          ComplianceTest.State = 1;
          break;
        case 4: // (vii)
          AppDataSize = McpsIndication->BufferSize;
          
          AppData[0] = 4;
          for( uint8_t i = 1; i < AppDataSize; i++ )
          {
            AppData[i] = McpsIndication->Buffer[i] + 1;
          }
          break;
        case 5: // (viii)
          {
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_LINK_CHECK;
            LoRaMacMlmeRequest( &mlmeReq );
          }
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }
  }
}

/*!
* \brief   MLME-Confirm event function
*
* \param   [IN] MlmeConfirm - Pointer to the confirm structure,
*               containing confirm attributes.
*/
static void MlmeConfirm( MlmeConfirm_t *MlmeConfirm )
{
  DPRINTF("MlmeConfirm\r\n");
  
  if( MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
  {
    switch( MlmeConfirm->MlmeRequest )
    {
    case MLME_JOIN:
      {
        lora_recv(LORA_EVENT_JOINED_OTAA, 0, 0, NULL);
        // Status is OK, node has joined the network
        break;
      }
    case MLME_LINK_CHECK:
      {
        lora_recv(LORA_EVENT_LINK_CHECK, MlmeConfirm->DemodMargin, MlmeConfirm->NbGateways, NULL);
        //                e_printf("MLME_LINK_CHECK\r\n");
        // Check DemodMargin
        // Check NbGateways
        if( ComplianceTest.Running == true )
        {
          ComplianceTest.LinkCheck = true;
          ComplianceTest.DemodMargin = MlmeConfirm->DemodMargin;
          ComplianceTest.NbGateways = MlmeConfirm->NbGateways;
        }
        break;
      }
    default:
      break;
    }
  } else if (MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT){
    lora_recv(LORA_EVENT_RX2_TIMEOUT, 0, 0, NULL);
  } else if (MlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT){
    lora_recv(LORA_EVENT_TX_TIMEOUT, 0, 0, NULL);   
  } else {
    DPRINTF("LORA_EVENT_UNKNOWN %d\r\n", MlmeConfirm->Status);
    lora_recv(LORA_EVENT_UNKNOWN, 0, 0, NULL);
  } 
}


void rw_ReadUsrConfig(void)
{
  int ret ;
  
  ret = read_partition(PARTITION_0, (char *)&g_lora_config, sizeof(g_lora_config));
  DPRINTF("ret = %d\r\n",ret);
  if (ret >= 0) {
    if ( rw_Str2Region(g_lora_config.region) == REGION_NULL) {
       ret = rw_restore_LoRaWAN_config(LORAMAC_REGION_DEFAULT, 1);
    }
  } else {
    rw_restore_LoRaWAN_config(LORAMAC_REGION_DEFAULT, 1);
  }
//  rw_restore_LoRaWAN_config(LORAMAC_REGION_DEFAULT, 1);
}

void dump_hex2str(uint8_t *buf , uint8_t len)
{
  for(uint8_t i=0; i<len; i++) {
     e_printf("%02X ", buf[i]);
  }
   e_printf("\r\n");
}

void rw_LoadUsrConfig(void)
{    
  MibRequestConfirm_t mibReq;
  
  LoRaSetUserMode(g_lora_config.lora_mode);
  
//  if (LoRaGetUserMode() == LORAP2P_MODE) {
//    return;
//  }	
  g_lora_system.recv_rssi_en = 0;
    
  LoRaMacTestSetDutyCycleOn(g_lora_config.duty_cycle);
  
  mibReq.Type = MIB_DEVICE_CLASS;
  mibReq.Param.Class = g_lora_config.loraWan_class; 
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_ADR;
  mibReq.Param.AdrEnable = g_lora_config.adr;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_CHANNELS_NB_REP;
  mibReq.Param.ChannelNbRep = g_lora_config.nb_trans;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_CHANNELS_DEFAULT_TX_POWER;
  mibReq.Param.ChannelsTxPower = g_lora_config.tx_pwr_level;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_CHANNELS_DEFAULT_DATARATE;  
  mibReq.Param.ChannelsDatarate = g_lora_config.def_tx_dr;
  LoRaMacMibSetRequestConfirm( &mibReq );  
  
  mibReq.Type = MIB_CHANNELS_DATARATE;  
  mibReq.Param.ChannelsDatarate = g_lora_config.def_tx_dr;
  LoRaMacMibSetRequestConfirm( &mibReq );   
  
  rw_SetTxDataRate(g_lora_config.def_tx_dr);
  
  mibReq.Type = MIB_PUBLIC_NETWORK;
  mibReq.Param.EnablePublicNetwork = g_lora_config.public_network;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_RECEIVE_DELAY_1;  
  mibReq.Param.ReceiveDelay1 = g_lora_config.rx_delay1;
  LoRaMacMibSetRequestConfirm( &mibReq );    
  
  mibReq.Type = MIB_RECEIVE_DELAY_2;  
  mibReq.Param.ReceiveDelay2 = g_lora_config.rx_delay1 + 1000;
  LoRaMacMibSetRequestConfirm( &mibReq ); 
  
  mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;  
  mibReq.Param.ChannelsDefaultMask = g_lora_config.ch_mask;
  LoRaMacMibSetRequestConfirm( &mibReq ); 
 
  mibReq.Type = MIB_CHANNELS_MASK;  
  mibReq.Param.ChannelsDefaultMask = g_lora_config.ch_mask;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_RX2_CHANNEL;  
  mibReq.Param.Rx2Channel.Frequency = g_lora_config.rx2.freq;
  mibReq.Param.Rx2Channel.Datarate = g_lora_config.rx2.dr;
  LoRaMacMibSetRequestConfirm( &mibReq ); 
  
  bool PublicNetwork = false;
  if (LoRaGetUserMode() == LORAP2P_MODE) {
    PublicNetwork = false; //LoraP2P mode
  } else {
    PublicNetwork = true;
  }
  Radio.SetPublicNetwork( PublicNetwork );
  Radio.Sleep( );
}

char* rw_Region2Str(LoRaMacRegion_t region)
{
  switch(region) {
    case LORAMAC_REGION_AS923: return "AS923";
    case LORAMAC_REGION_AU915: return "AU915";
    case LORAMAC_REGION_CN470: return "CN470";
    case LORAMAC_REGION_CN779: return "CN779";
    case LORAMAC_REGION_EU433: return "EU433";
    case LORAMAC_REGION_EU868: return "EU868";
    case LORAMAC_REGION_KR920: return "KR920";
    case LORAMAC_REGION_IN865: return "IN865";
    case LORAMAC_REGION_US915: return "US915";
    case LORAMAC_REGION_US915_HYBRID: return "US915_H";
  default:
    return "";
  }
  
}

LoRaMacRegion_t rw_Str2Region(char* region)
{
  if ( 0==strcmp(region, "AS923")) {
     return LORAMAC_REGION_AS923;
  } else if (0==strcmp(region, "AU915")) {
     return LORAMAC_REGION_AU915;
  }else if (0==strcmp(region, "CN470")) {
     return LORAMAC_REGION_CN470;
  }else if (0==strcmp(region, "CN779")) {
     return LORAMAC_REGION_CN779;
  }else if (0==strcmp(region, "EU433")) {
     return LORAMAC_REGION_EU433;
  }else if (0==strcmp(region, "EU868")) {
     return LORAMAC_REGION_EU868;
  }else if (0==strcmp(region, "KR920")) {
     return LORAMAC_REGION_KR920;
  }else if (0==strcmp(region, "IN865")) {
     return LORAMAC_REGION_IN865;
  }else if (0==strcmp(region, "US915")) {
     return LORAMAC_REGION_US915;
  }else if (0==strcmp(region, "US915_H")) {
     return LORAMAC_REGION_US915_HYBRID;
  }else {
     return REGION_NULL;
  }
}


int rw_restore_LoRaWAN_config(LoRaMacRegion_t region, uint8_t No_retain)
{ 
  GetPhyParams_t getPhy;
  PhyParam_t phyParam;
  LoRaMacRegion_t LoRaMacRegion;
  
  LoRaMacRegion = region;
  
  // Verify if the region is supported
  if( RegionIsActive( LoRaMacRegion ) == false )
  {
    return -1;
  }
  
  read_partition(PARTITION_0, (char *)&g_lora_config, sizeof(lora_config_t)); 
  lora_config_t* a = &g_lora_config;
 
  if ( 1 == No_retain) {
     memset(a, 0, sizeof(lora_config_t));
     BoardGetUniqueId(a->dev_eui);  
	 DPRINTF("DEFAULT DEV_EUI = %02X%02X%02X%02X%02X%02X%02X%02X",
	 	            a->dev_eui[0],a->dev_eui[1],a->dev_eui[2],a->dev_eui[3],
	 	            a->dev_eui[4],a->dev_eui[5],a->dev_eui[6],a->dev_eui[7]);
  }

  a->loraWan_class = CLASS_A; 
  a->public_network = true;
  a->adr = LORAWAN_ADR_ON;
  
  strcpy(a->region, rw_Region2Str(LoRaMacRegion));
  
  RegionInitDefaults( LoRaMacRegion, INIT_TYPE_INIT );
  
  // Reset to defaults
  getPhy.Attribute = PHY_DUTY_CYCLE;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->duty_cycle = ( bool ) phyParam.Value;
  
  getPhy.Attribute = PHY_DEF_TX_POWER;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->tx_pwr_level = phyParam.Value;
  
  getPhy.Attribute = PHY_DEF_TX_DR;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->def_tx_dr = phyParam.Value;
  
  getPhy.Attribute = PHY_RECEIVE_DELAY1;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->rx_delay1 = phyParam.Value;
  
  getPhy.Attribute = PHY_DEF_DR1_OFFSET;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->Rx1DrOffset = phyParam.Value;
  
  getPhy.Attribute = PHY_DEF_RX2_FREQUENCY;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->rx2.freq = phyParam.Value; 
  
  getPhy.Attribute = PHY_DEF_RX2_DR;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->rx2.dr = phyParam.Value;
  
  getPhy.Attribute = PHY_DEF_NB_JOIN_TRIALS;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->join_cnt = ( uint8_t ) phyParam.Value;
  
  getPhy.Attribute = PHY_MAX_NB_CHANNELS;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  a->max_nb_chs = phyParam.Value;

  getPhy.Attribute = PHY_CHANNELS_DEFAULT_MASK;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  memset(a->ch_mask, 0, sizeof(a->ch_mask));
  RegionCommonChanMaskCopy( a->ch_mask, phyParam.ChannelsMask, (uint8_t)ceil(((double)a->max_nb_chs)/16)); // Copy channels default mask
            

  getPhy.Attribute = PHY_CHANNELS;
  phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
  memcpy(a->ch_list, phyParam.Channels, a->max_nb_chs*sizeof(ChannelParams_t)); 
  
  a->nb_trans = 1;
  a->ack_retrans = 8;
    
  a->lora_mode = LORAWAN_MODE;
  a->lorap2p_param.Frequency = 868100000;
  a->lorap2p_param.Spreadfact = 12;
  a->lorap2p_param.Bandwidth = 0;
  a->lorap2p_param.Codingrate = 1;
  a->lorap2p_param.Preamlen = 8;
  a->lorap2p_param.Powerdbm = 20;
  
  write_partition(PARTITION_0, (char *)a, sizeof(lora_config_t));
  DPRINTF("-restore-\r\n");
  return 0;
}


LoRaMacPrimitives_t LoRaMacPrimitives;
LoRaMacCallback_t LoRaMacCallbacks;

int rw_InitLoRaWAN(void)
{
  
  LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
  LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
  LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
  LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
  LoRaMacCallbacks.P2PTxDone       = rw_P2PTxDone;
  LoRaMacCallbacks.P2PTxTimeout    = rw_P2PTxTimeout;
  LoRaMacCallbacks.P2PRxDone       = rw_P2PRxDone;
  LoRaMacCallbacks.P2PRxTimeout    = rw_P2PRxTimeout;
  LoRaMacCallbacks.P2PRxError      = rw_P2PRxError;
  LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, rw_Str2Region(g_lora_config.region) );
  
  TimerInit( &LoraP2PTxNextPacketTimer, rw_P2PTxContinueTimerEvent);

  LoRaClrRadioStatus();
  
  return 0;
  
}

int rw_DeInitLoRaWAN(void)
{
  //BoardDeInitPeriph();
  LoRaMacDeInitialization();
  LoRaClrRadioStatus();
  
  return 0;
}    

void rw_SetTxDataRate(uint8_t dr)
{
  TxDataRate = dr;
}

uint8_t rw_GetTxDataRate(void)
{
  return TxDataRate ;
}

int rw_ResetLoRaWAN(void)
{
  rw_DeInitLoRaWAN();
  rw_ReadUsrConfig();
  rw_InitLoRaWAN();
  rw_LoadUsrConfig();     
  return 0;
}


int rw_JoinNetworkOTAA(uint8_t *dev_eui, uint8_t *app_eui, uint8_t *app_key, uint8_t nb_trials)
{
  MlmeReq_t mlmeReq;
  
  mlmeReq.Type = MLME_JOIN;
  
  mlmeReq.Req.Join.DevEui = dev_eui;
  mlmeReq.Req.Join.AppEui = app_eui;
  mlmeReq.Req.Join.AppKey = app_key;
  mlmeReq.Req.Join.NbTrials = nb_trials;
  
  LoRaMacMlmeRequest( &mlmeReq );
  
  return 0;
}


int rw_JoinNetworkABP(uint32_t *dev_addr, uint8_t *nwks_key, uint8_t *apps_key)
{
  MibRequestConfirm_t mibReq;
  
  if (dev_addr == NULL && nwks_key == NULL || apps_key == NULL) {
    return -1;
  }
  
  mibReq.Type = MIB_NET_ID;
  mibReq.Param.NetID = LORAWAN_NETWORK_ID;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_DEV_ADDR;
  mibReq.Param.DevAddr = *dev_addr;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_NWK_SKEY;
  mibReq.Param.NwkSKey = nwks_key;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_APP_SKEY;
  mibReq.Param.AppSKey = apps_key;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  mibReq.Type = MIB_NETWORK_JOINED;
  mibReq.Param.IsNetworkJoined = true;
  LoRaMacMibSetRequestConfirm( &mibReq );
  
  //rw_LoRaTxData(false, 2, 2, "55");         
  rw_LoadUsrConfig();
  
  return 0;
}

int rw_LoRaLinkCheck(void)
{
  int status;
  MlmeReq_t mlmeReq;
  mlmeReq.Type = MLME_LINK_CHECK;
  status = LoRaMacMlmeRequest( &mlmeReq );
  if (status != LORAMAC_STATUS_OK) {
    return status;
  }
  return -1;
}


int rw_LoRaTxData(bool confirm, uint8_t app_port, uint16_t app_len, uint8_t *app_data)
{
  LoRaMacStatus_t status;
  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  
  if (app_data == NULL) {
    return -1;
  }
  
  memcpy(AppData, app_data, app_len);
  
  if( LoRaMacQueryTxPossible( app_len, &txInfo ) != LORAMAC_STATUS_OK ) {
    
    //return LORAMAC_STATUS_LENGTH_ERROR;
    // Send empty frame in order to flush MAC commands
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fBuffer = NULL;
    mcpsReq.Req.Unconfirmed.fBufferSize = 0;
    mcpsReq.Req.Unconfirmed.Datarate = DR_0;
  } else {
    if (confirm == false) {
      mcpsReq.Type = MCPS_UNCONFIRMED;
      mcpsReq.Req.Unconfirmed.fPort = app_port;
      mcpsReq.Req.Unconfirmed.fBuffer = AppData;
      mcpsReq.Req.Unconfirmed.fBufferSize = app_len;
      mcpsReq.Req.Unconfirmed.Datarate = TxDataRate;
    } else {
      mcpsReq.Type = MCPS_CONFIRMED;
      mcpsReq.Req.Confirmed.fPort = app_port;
      mcpsReq.Req.Confirmed.fBuffer = AppData;
      mcpsReq.Req.Confirmed.fBufferSize = app_len;
      mcpsReq.Req.Confirmed.NbTrials = g_lora_config.ack_retrans;
      mcpsReq.Req.Confirmed.Datarate = TxDataRate;
    }
  }
  
  status = LoRaMacMcpsRequest(&mcpsReq);
  
  if (status != LORAMAC_STATUS_OK) {
    return status;
  }
  return 0;    
  
}

static void _rw_P2PTxLoop(void)
{	
  if (P2PTxCnts) {	
    TimerSetValue( &LoraP2PTxNextPacketTimer, P2PTxInterVal);
    TimerStart( &LoraP2PTxNextPacketTimer );
  } else {
    lora_recv(LORA_EVENT_P2PTX_COMPLETE, 0, 0, NULL);
    Radio.Sleep();
  }
}


static void rw_P2PTxDone( void )
{
  //DPRINTF("rw_P2PTxDone\r\n");
  _rw_P2PTxLoop();
}

static void rw_P2PRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  DPRINTF("==> size=%d, rssi=%d, snr=%d\r\n", size, rssi, snr);
  if (P2PRxReportEn) {
    lora_recv_ex(LORA_EVENT_RECV_DATA, 0, rssi, snr, size, payload);
  }
}

static void rw_P2PTxTimeout( void )
{
  //DPRINTF("rw_P2PTxTimeout\r\n");
  _rw_P2PTxLoop();
}

static void rw_P2PRxError( void )
{
  //DPRINTF("rw_P2PRxError\r\n");
}

static void rw_P2PRxTimeout( void )
{
  //DPRINTF("rw_P2PRxTimeout\r\n");
}


void rw_P2PTxContinueTimerEvent( void )
{
  //DPRINTF("rw_P2PTxContinueTimerEvent\r\n");
  TimerStop( &LoraP2PTxNextPacketTimer );
  
  SendRadioP2PFrame(g_lora_config.lorap2p_param, NULL, NULL);
  if (P2PTxCnts)
    P2PTxCnts--;
}

int rw_LoRaP2PTxContinue(uint16_t counts, uint32_t intervalMs, uint16_t tx_len, uint8_t *tx_data)
{
  TimerTime_t txtime;
  
  if (P2PTxCnts) {
    return LORAMAC_STATUS_BUSY;
  }
  
  if (tx_data == NULL) {
    return -1;
  } 	
  
  //memcpy(AppData, tx_data, tx_len);    
  P2PTxCnts = counts;
  P2PTxInterVal = intervalMs;
  
  txtime = SendRadioP2PFrame(g_lora_config.lorap2p_param, tx_data, tx_len);
  if (P2PTxCnts)
    P2PTxCnts--;
  
  return LORAMAC_STATUS_OK;    
}

void rw_LoRaP2PTxStop(void)
{
  P2PTxCnts = 0;
  P2PTxInterVal = 0;
  TimerStop( &LoraP2PTxNextPacketTimer );   
}

int rw_LoRaP2PRxContinue(uint8_t report)
{
  RadioState_t  state;
  
  P2PRxReportEn = report;
  
  state = RxRadioP2PFrame(g_lora_config.lorap2p_param);
  if (state != RF_RX_RUNNING) {
    return -1;
  }
  return 0;     
}
void rw_LoRaP2PRxStop(void)
{  
  Radio.Sleep();
}


void rw_GetVersion(char *ver)
{
  uint16_t len = 0;
  
#if DEBUG_FW
  len = sprintf(&ver[0],"%d.%d.%d.%d.%d",MAJOR_VER
                ,CUSTOM_VER
                  ,FUN_VER
                    ,BUG_VER
                      ,TEST_VER);
#else
  len = sprintf(&ver[0],"%d.%d.%d.%d",MAJOR_VER
                ,CUSTOM_VER
                  ,FUN_VER
                    ,BUG_VER);
#endif
  ver[len] = '\0';
}


void rw_ResetMCU(uint8_t mode)
{
  NVIC_SystemReset();
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	e_printf("The Hard Fault exception occurs,Reset!\r\n");
  NVIC_SystemReset();
  
}
