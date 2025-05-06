/* Example of code that is compiled to run from RAM, pushed to RAM, then run from there. */

#include "ch32fun.h"
#include <stdio.h>

#define LED                PA9
#define BLE_MEMHEAP_SIZE   (512*7)
#define  TX_MODE_TX_FINISH 0x01
#define  TX_MODE_TX_FAIL   0x11

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4]; // try to get rid of this
uint32_t g_LLE_IRQLibHandlerLocation;

typedef void (*pfnRFStatusCB_t)( uint8_t sta, uint8_t rsr, uint8_t *rxBuf );
typedef struct tag_rf_config {
	uint8_t LLEMode;                  //!< BIT0   0=basic, 1=auto def@LLE_MODE_TYPE
									  //!< BIT1   0=whitening on, 1=whitening off def@LLE_WHITENING_TYPE
									  //!< BIT4-5 00-1M  01-2M  10/11-resv def@LLE_PHY_TYPE
									  //!< BIT6   0=data channel(0-39)
									  //!<        1=rf frequency (2400000kHz-2483500kHz)
									  //!< BIT7   0=the first byte of the receive buffer is rssi
									  //!<        1=the first byte of the receive buffer is package type
	uint8_t Channel;                  //!< rf channel(0-39)
	uint32_t Frequency;               //!< rf frequency (2400000kHz-2483500kHz)
	uint32_t accessAddress;           //!< access address,32bit PHY address
	uint32_t CRCInit;                 //!< crc initial value
	pfnRFStatusCB_t rfStatusCB;       //!< status call back
	uint8_t RxMaxlen;                 //!< Maximum data length received in rf-mode(default 251)
	uint8_t TxMaxlen;                 //!< Maximum data length transmit in rf-mode(default 251)
} rfConfig_t;

typedef uint32_t (*pfnSrandCB)( void );
typedef uint32_t (*pfnIdleCB)( uint32_t );
typedef void (*pfnLibStatusErrorCB)( uint8_t code, uint32_t status );
typedef uint32_t (*pfnFlashReadCB)( uint32_t addr, uint32_t num, uint32_t *pBuf );
typedef uint32_t (*pfnFlashWriteCB)( uint32_t addr, uint32_t num, uint32_t *pBuf );
typedef struct tag_ble_config
{
	uint32_t MEMAddr;               // library memory start address
	uint16_t MEMLen;                // library memory size
	uint32_t SNVAddr;               // SNV flash start address( if NULL,bonding information will not be saved )
	uint16_t SNVBlock;              // SNV flash block size ( default 256 )
	uint8_t SNVNum;                 // SNV flash block number ( default 1 )
	uint8_t BufNumber;              // Maximum number of sent and received packages cached by the controller( default 3 )
									// Must be greater than the number of connections.
	uint16_t BufMaxLen;             // Maximum length (in octets) of the data portion of each HCI data packet( default 27 )
									// ATT_MTU = BufMaxLen-4,Range[23,ATT_MAX_MTU_SIZE]
	uint8_t TxNumEvent;             // Maximum number of TX data in a connection event ( default 1 )
	uint8_t RxNumEvent;             // Maximum number of RX data in a connection event ( default equal to BufNumber )
	uint8_t TxPower;                // Transmit power level( default LL_TX_POWEER_0_DBM(0dBm) )
	uint8_t WindowWidening;         // Wait rf start window(us)
	uint8_t WaitWindow;             // Wait event arrive window in one system clock
	uint8_t MacAddr[6];             // MAC address,little-endian
	pfnSrandCB srandCB;             // Register a program that generate a random seed
	pfnIdleCB idleCB;               // Register a program that set idle
	pfnLibStatusErrorCB staCB;      // Register a program that library status callback
	pfnFlashReadCB readFlashCB;     // Register a program that read flash
	pfnFlashWriteCB writeFlashCB;   // Register a program that write flash
} bleConfig_t; // Library initialization call BLE_LibInit function

typedef uint8_t bStatus_t;
extern bStatus_t RF_Config( rfConfig_t *pConfig );
extern bStatus_t RF_Tx( uint8_t *txBuf, uint8_t txLen, uint8_t pktTxType, uint8_t pktRxType );
extern bStatus_t RF_RoleInit( void );
extern bStatus_t BLE_LibInit( bleConfig_t* pCfg );
extern void LLE_IRQLibHandler( void );

volatile uint8_t tx_end_flag = 0;
rfConfig_t rf_Config = {0};

__attribute__((section(".highcode")))
__attribute__((noinline))
void RF_Wait_Tx_End() {
	uint32_t i = 0;
	while(!tx_end_flag) {
		i++;
		__NOP();
		__NOP();
		if(i > (FUNCONF_SYSTEM_CORE_CLOCK/1000)) {
			tx_end_flag = 1;
		}
	}
}

void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf) {
	switch(sta) {
	case TX_MODE_TX_FINISH:
	case TX_MODE_TX_FAIL:
		tx_end_flag = 1;
		break;
	default:
		break;
	}
}

void send_adv(uint8_t adv[], size_t len, uint8_t channel) {
	rf_Config.Channel = channel;
	RF_Config(&rf_Config);
	tx_end_flag = 0;
	if(!RF_Tx(adv, len, 0x02, 0xFF)) {
		RF_Wait_Tx_End();
	}
}

void blink(int n) {
	for(int i = n-1; i >= 0; i--) {
		funDigitalWrite( LED, FUN_LOW ); // Turn on LED
		Delay_Ms(33);
		funDigitalWrite( LED, FUN_HIGH ); // Turn off LED
		if(i) Delay_Ms(33);
	}
}

int main()
{
	SystemInit();

	funGpioInitAll();
	funPinMode( LED, GPIO_CFGLR_OUT_2Mhz_PP );

	g_LLE_IRQLibHandlerLocation = (uint32_t)LLE_IRQLibHandler;
	bleConfig_t blecfg = {0};
	blecfg.MEMAddr = (uint32_t)MEM_BUF;
	blecfg.MEMLen = (uint32_t)BLE_MEMHEAP_SIZE;
	BLE_LibInit(&blecfg);
	// RF_RoleInit(); // we need this for stability, but it does not fit in the 12k RAM

	rf_Config.accessAddress = 0x8E89BED6;
	rf_Config.CRCInit = 0x555555;
	rf_Config.LLEMode = 0; //LLE_MODE_BASIC;
	rf_Config.rfStatusCB = RF_2G4StatusCallBack;
	RF_Config(&rf_Config);

	uint8_t adv[] = {0x66, 0x55, 0x44, 0x33, 0x22, 0xd1, // MAC (reversed)
					 0x1e, 0xff, 0x4c, 0x00, 0x12, 0x19, 0x00, // Apple FindMy stuff
					 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xef, 0xfe, 0xdd,0xcc, // key
					 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // more key
					 0x00, 0x00}; // status byte and one more

	blink(5);
	printf("~ ch570 broadcaster ~\n");
	while(1)
	{
		blink(1); // 33ms
		send_adv(adv, sizeof(adv), 37);
		send_adv(adv, sizeof(adv), 38);
		send_adv(adv, sizeof(adv), 39);
		Delay_Ms(300);
	}
}
