#ifndef  _BOARD_CONFIG_H_
#define  _BOARD_CONFIG_H_

#define DEBUG_FW           0

#ifdef LORA_HF_BOARD
	#define P2P_FREQ_MIN                  860000000 
	#define P2P_FREQ_MAX                  1020000000

	#define LORAMAC_REGION_DEFAULT     LORAMAC_REGION_EU868

	#ifdef __cplusplus
	 extern "C" {
	#endif

		#define RAK811_PINS_MASK                            (0x076EE19E) //  byte0: 10011110
																		 //  byte1: 11100001
																		 //  byte2: 01101110
																		 //  byte3: 00000111 

		#define RAK811_ADC_PINS_MASK                        (0x0068000E) //  byte0: 00001110
																		 //  byte1: 00000000
																		 //  byte2: 01101000
																		 //  byte3: 00000000

		#define RAK811_PIN2                                 PB_12       //ADC_IN18  

		#define RAK811_PIN3                                 PB_14      //ADC_IN20
		   
		#define RAK811_PIN4                                 PB_15       //ADC_IN21

		#define RAK811_PIN5                                 PA_8
		   
		#define RAK811_PIN8                                 PA_12
		   
		#define RAK811_PIN9                                 PB_4
		   
		#define RAK811_PIN14                                 PA_15

		#define RAK811_PIN15                                 PB_3

		#define RAK811_PIN16                                 PB_5

		#define RAK811_PIN18                                 PB_8  //I2C1_SCL

		#define RAK811_PIN19                                 PB_9  //I2C1_SDA

		#define RAK811_PIN20                                 PA_2  //ADC_IN2

		#define RAK811_PIN22                                 PA_1  //ADC_IN1

		#define RAK811_PIN23                                 PA_0  //ADC_IN0

		#define RAK811_PIN25                                 PB_10 
		   
		#define RAK811_PIN26                                 PB_11 

		#define RAK811_PIN27                                 PB_2   
		   
		   
		const static PinNames RAK811_pin_array[32] ={ NC, RAK811_PIN2, RAK811_PIN3, RAK811_PIN4, RAK811_PIN5, NC, NC, RAK811_PIN8, 
													  RAK811_PIN9, NC, NC, NC, NC, RAK811_PIN14, RAK811_PIN15, RAK811_PIN16,
													  NC, RAK811_PIN18, RAK811_PIN19, RAK811_PIN20, NC, RAK811_PIN22, RAK811_PIN23, NC,
													  RAK811_PIN25, RAK811_PIN26, RAK811_PIN27, NC, NC, NC, NC, NC};

		const static uint32_t RAK811_adc_pin_map[32] ={0, ADC_CHANNEL_18, ADC_CHANNEL_20, ADC_CHANNEL_21, 0, 0, 0, 0,  //1-8
													   0, 0, 0, 0, 0, 0, 0, 0,                                         //9-16
													   0, 0, 0, ADC_CHANNEL_2, 0, ADC_CHANNEL_1, ADC_CHANNEL_0, 0,     //7-24
													   0, 0, 0, 0, 0, 0, 0, 0};                                        //25-32

	#ifdef __cplusplus
	}
	#endif
	
#else
	#define P2P_FREQ_MIN                  433000000
	#define P2P_FREQ_MAX                  510000000

	#define LORAMAC_REGION_DEFAULT      LORAMAC_REGION_EU433

	#ifdef __cplusplus
	 extern "C" {
	#endif

		#define RAK811_PINS_MASK                            (0x076EE19E) //  byte0: 10011110
																		 //  byte1: 11100001
																		 //  byte2: 01101110
																		 //  byte3: 00000111 

		#define RAK811_ADC_PINS_MASK                        (0x0068401E) //  byte0: 00011110
																		 //  byte1: 01000000
																		 //  byte2: 01101000
																		 //  byte3: 00000000

		#define RAK811_PIN2                                 PB_12       //ADC_IN18 / SPI2_NSS

		#define RAK811_PIN3                                 PB_14      //ADC_IN20 / SPI2_MISO
		   
		#define RAK811_PIN4                                 PB_15       //ADC_IN21 / SPI2_MOSI

		#define RAK811_PIN5                                 PB_13     // ADC_IN19 / SPI2_SCK
		   
		#define RAK811_PIN8                                 PA_12    // USB_DM
		   
		#define RAK811_PIN9                                 PA_11    // USB_DP
		   
		#define RAK811_PIN14                                 PA_15

		#define RAK811_PIN15                                 PA_3    // ADC_IN3 / UART2_RX

		#define RAK811_PIN16                                 PB_5

		#define RAK811_PIN18                                 PB_6  //I2C1_SCL

		#define RAK811_PIN19                                 PB_7  //I2C1_SDA

		#define RAK811_PIN20                                 PA_2  //ADC_IN2 / UART2_TX

		#define RAK811_PIN22                                 PA_0  //ADC_IN0

		#define RAK811_PIN23                                 PA_1  //ADC_IN1

		#define RAK811_PIN25                                 PB_10 // UART3_TX / I2C2_SCL
		   
		#define RAK811_PIN26                                 PB_11  // UART3_RX / I2C_SDA

		#define RAK811_PIN27                                 PB_2   // BOOT1
		   
		   
		const static PinNames RAK811_pin_array[32] ={ NC, RAK811_PIN2, RAK811_PIN3, RAK811_PIN4, RAK811_PIN5, NC, NC, RAK811_PIN8, 
													  RAK811_PIN9, NC, NC, NC, NC, RAK811_PIN14, RAK811_PIN15, RAK811_PIN16,
													  NC, RAK811_PIN18, RAK811_PIN19, RAK811_PIN20, NC, RAK811_PIN22, RAK811_PIN23, NC,
													  RAK811_PIN25, RAK811_PIN26, RAK811_PIN27, NC, NC, NC, NC, NC};

		const static uint32_t RAK811_adc_pin_map[32] ={0, ADC_CHANNEL_18, ADC_CHANNEL_20, ADC_CHANNEL_21, ADC_CHANNEL_19, 0, 0, 0,  //1-8
													   0, 0, 0, 0, 0, 0, ADC_CHANNEL_3, 0,                                         //9-16
													   0, 0, 0, ADC_CHANNEL_2, 0, ADC_CHANNEL_0, ADC_CHANNEL_1, 0,     //7-24
													   0, 0, 0, 0, 0, 0, 0, 0};                                        //25-32

	#ifdef __cplusplus
	}
	#endif
#endif
	

#define CHECK_UART_BAUD(baud) ((baud==9600) ||(baud==19200) ||(baud==38400) ||(baud==57600) ||(baud==115200)||(baud==230400) || (baud==460800) || (baud==921600))
#define CHECK_UART_DATABIT(databit) (databit==UART_8_BIT)
#define CHECK_UART_PARITY(parity) ((parity==NO_PARITY) || (parity==EVEN_PARITY) || (parity==ODD_PARITY))
#define CHECK_UART_STOPBIT(stop) ((stop==UART_1_STOP_BIT) || (stop==UART_2_STOP_BIT))
#define CHECK_UART_FLOW(flow) ((flow == 0) || (flow == 1))

/***check p2p parameters add by junhua****/
#define CHECK_P2P_FREQ(freq) ((freq >= P2P_FREQ_MIN) && (freq <= P2P_FREQ_MAX))
#define CHECK_P2P_SF(sf) ((sf >= 6) && (sf <= 12))
#define CHECK_P2P_BDW(bandw) ((bandw == 0) || (bandw == 1) || (bandw == 2))
#define CHECK_P2P_CR(cr) ((cr >= 1) && (cr <= 4))
#define CHECK_P2P_PREMLEN(prelen) ((prelen >= 5) && (prelen <= 65535))
#define CHECK_P2P_PWR(pwr) ((pwr >= 5) && (pwr <= 20))
	
#endif