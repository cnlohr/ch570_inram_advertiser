/* Example of code that is compiled to run from RAM, pushed to RAM, then run from there. */

#include "ch32fun.h"
#include "isler.h"
#include <stdio.h>

#define LED               PA9
#define LL_TX_POWER_0_DBM 0x12

extern void BLECoreInit(uint8_t TxPower);
extern void Advertise(uint8_t adv[], size_t len, uint8_t channel);

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

	BLECoreInit(LL_TX_POWER_0_DBM);

#if 1
	// Apple Find My Stuff Packet
	uint8_t adv[] = {0x66, 0x55, 0x44, 0x33, 0x22, 0xd1, // MAC (reversed)
					 0x1e, 0xff, 0x4c, 0x00, 0x12, 0x19, 0x00, // Apple FindMy stuff
					 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xef, 0xfe, 0xdd,0xcc, // key
					 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // more key
					 0x00, 0x00}; // status byte and one more
#else
	// Special payload for investigating RF registers.
	uint8_t adv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Normally, MAC
					 0x10, // Packet Length
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload (First byte in BLE is type)
					 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, };

#endif
	uint8_t adv_channels[] = {37,38,39};

	blink(5);
	printf("~ ch570 broadcaster !\n");
	while(1) {
		printf( "BCAST\n" );
#if 0
		// Only transmit on advertising channels.
		for(int c = 0; c < sizeof(adv_channels); c++) {
			Advertise(adv, sizeof(adv), adv_channels[c]);
		}
#else
		// Sequence across spectrum
		for(int c = 0; c < 41; c++) {
			Advertise(adv, sizeof(adv), c );
		}
#endif
		blink(1); // 33ms
		Delay_Ms(300);
	}
}
