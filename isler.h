#include "ch32fun.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define BB_BASE (0x4000c100)
#define LL_BASE (0x4000c200)
#define RF_BASE (0x4000d000)

#define BB ((BB_Type *) BB_BASE)
#define LL ((LL_Type *) LL_BASE)
#define RF ((RF_Type *) RF_BASE)

typedef struct{
	// bits 0..5 = Channel
	// bit 6 = disable whitening.
	// bit 8 = 1 during normal TX/operation, but clearing does not affect TX.  Note: 0 at reset, set in software.
	// bit 9 = settable, but unknown effect.
	// bit 10 = 1 during normal TX/operation, but clearing does not affect TX.  Note: 1 at reset, not touched in software.
	// bit 16 = cleared by firmware upon TX, but does not seem to have an effect on the TX.
	// bit 17 = settable, but unknown effect
	// bit 20 = settable, but unknown effect.
	// bit 24 = set at end of tx routine
	// bit 29-31 = settable, but unknown effect.
	volatile uint32_t CTRL_CFG;

	volatile uint32_t CRCINIT1;
	volatile uint32_t CRCPOLY1;
	volatile uint32_t ACCESSADDRESS1;
	volatile uint32_t BB4;
	volatile uint32_t BB5;
	volatile uint32_t BB6;
	volatile uint32_t BB7;
	volatile uint32_t BB8;
	volatile uint32_t BB9;
	volatile uint32_t BB10;
	volatile uint32_t BB11;
	volatile uint32_t BB12;

	// default, pre TX is a4000009
	// bit 0: Set normally, but cleared in software when TXing (maybe a ready bit?)
	// bit 1: Unset normally, but cleared anyway by software when TXing (maybe a fault bit?)
	// bit 2: Disables TX.
	// bit 4: Normally 0, but, if set to 1, seems to increase preamble length.
	// bit 8: Normally 0, but, if set, no clear effect.
	// bit 9: Normally 0, but, if set, no clear effect.
	// bits 24-30: TX Power.  Normally 0xA4
	// Oddly, bit 31 seems to maybe be always set.
	volatile uint32_t CTRL_TX;
	volatile uint32_t BB14;
	volatile uint32_t BB15;
	volatile uint32_t CTRL_RX_INT;
	volatile uint32_t BB17;
	volatile uint32_t BB18;
	volatile uint32_t BB19;
	volatile uint32_t BB20;
	volatile uint32_t BB21;
	volatile uint32_t CRCINIT2;
	volatile uint32_t CRCPOLY2;
	volatile uint32_t ACCESSADDRESS2;
} BB_Type;

typedef struct{
	volatile uint32_t LL0;
	volatile uint32_t LL1;
	volatile uint32_t STATUS;
	volatile uint32_t INT_EN;
	volatile uint32_t LL4;
	volatile uint32_t LL5;
	volatile uint32_t LL6;
	volatile uint32_t LL7;
	volatile uint32_t LL8;
	volatile uint32_t LL9;
	volatile uint32_t LL10;
	volatile uint32_t LL11;
	volatile uint32_t LL12;
	volatile uint32_t LL13;
	volatile uint32_t LL14;
	volatile uint32_t LL15;
	volatile uint32_t LL16;
	volatile uint32_t LL17;
	volatile uint32_t LL18;
	volatile uint32_t LL19;

	// Controls a lot of higher-level functions.
	//  For Tuning: 0x30558
	//  For  Idle:  0x30000
	//  For Sending:0x30258
	// Bit 3: Somehow, enables BB
	// Bit 4: Normally 1, controls length/send times of BB, if unset, BB will double-send part of signals.
	// Bit 6: Normally 1, Unknown effect.
	// Bit 9: If 0, no output.
	// Bit 10: Somehow required for TX?
	// Bit 16-17: Normally 1, unknown effect. Seems to suppress odd carrier burst after message.
	volatile uint32_t CTRL_MOD;
	volatile uint32_t LL21;
	volatile uint32_t LL22;
	volatile uint32_t LL23;
	volatile uint32_t LL24;

	// Immediately after tuning this value is 0x346/0x347,
	// a microsecond or so later, it's 0x338/0x339,
	// but after the PLL appears to have locked, this value is 0x1f8/0x1f9.
	volatile uint32_t TMR;
	volatile uint32_t LL26;
	volatile uint32_t LL27;
	volatile uint32_t LL28;
	volatile uint32_t LL29;
	volatile uint32_t FRAME_BUF;
	volatile uint32_t STATE_BUF;
} LL_Type;

typedef struct{
	volatile uint32_t RF0;
	volatile uint32_t RF1;
	volatile uint32_t RF2;
	volatile uint32_t RF3;
	volatile uint32_t RF4;
	volatile uint32_t RF5;
	volatile uint32_t RF6;
	volatile uint32_t RF7;
	volatile uint32_t RF8;
	volatile uint32_t RF9;
	volatile uint32_t RF10;
	volatile uint32_t RF11;
	volatile uint32_t RF12;
	volatile uint32_t RF13;
	volatile uint32_t TXTUNE_CTRL;
	volatile uint32_t RF15;
	volatile uint32_t RF16;
	volatile uint32_t RF17;
	volatile uint32_t RF18;
	volatile uint32_t RF19;
	volatile uint32_t RF20;
	volatile uint32_t RF21;
	volatile uint32_t RF22;
	volatile uint32_t RF23;
	volatile uint32_t RF24;
	volatile uint32_t RF25;
	volatile uint32_t RF26;
	volatile uint32_t RF27;
	volatile uint32_t RF28;
	volatile uint32_t RF29;
	volatile uint32_t RF30;
	volatile uint32_t RF31;
	volatile uint32_t RF32;
	volatile uint32_t RF33;
	volatile uint32_t RF34;
	volatile uint32_t RF35;
	volatile uint32_t TXCTUNE_CO_CTRL;
	volatile uint32_t TXCTUNE_GA_CTRL;
	volatile uint32_t RF38;
	volatile uint32_t RXTUNE;
	volatile uint32_t TXCTUNE_CO[10];
	volatile uint32_t TXCTUNE_GA[3];
} RF_Type;

uint8_t channel_map[] = {1,2,3,4,5,6,7,8,9,10,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,0,11,39};
#define CO_MID (uint8_t)(RF->TXTUNE_CTRL & ~0xffffffc0)
#define GA_MID (uint8_t)((RF->TXTUNE_CTRL & ~0x80ffffff) >> 24)

uint32_t tuneFilter;
__attribute__((aligned(4))) uint32_t LLE_BUF[0x10c];

volatile int txt, txt1;

void DevSetMode(uint32_t mode);

__attribute__((interrupt))
void LLE_IRQHandler() {
	LL->STATUS &= LL->INT_EN;
	BB->CTRL_TX = (BB->CTRL_TX & 0xfffffffc) | 1;
//	printf( "A\n" );
	txt1++;
}


__attribute__((interrupt)) void BB_IRQHandler() {
	//printf( "B  %08x\n", BB->CTRL_RX_INT );
	static int ikk = 0;
	// bottom 24 bits = cause
	//  0x148 = First trigger
	//  0x3ec = Second trigger (If you delay in here)
	BB->CTRL_RX_INT=0x00000fff;
	DevSetMode(0x379);
	//LL->CTRL_MOD = (0x30000 | ikk);
	//LL->CTRL_MOD = (0x30000 | 0x79);
	ikk++;
//	printf( "B' %08x\n", BB->CTRL_RX_INT );
	txt++;
}



void DevInit(uint8_t TxPower) {
	LL->LL5 = 0x8c;
	LL->LL7 = 0x76;
	LL->LL9 = 0x8c;
	LL->LL11 = 0x6c;
	LL->LL13 = 0x8c;
	LL->LL15 = 0x6c;
	LL->LL17 = 0x8c;
	LL->LL19 = 0x76;
	LL->LL1 = 0x78;
	LL->LL21 = 0;
	LL->STATE_BUF = (uint32_t)LLE_BUF;
	LL->STATUS = 0xffffffff;
	LL->INT_EN = 0x16000f;

	RF->RF10 = 0x480;
	RF->RF12 &= 0xfff9ffff;
	RF->RF12 |= 0x70000000;
	RF->RF15 = (RF->RF15 & 0xf8ffffff) | 0x2000000;
	RF->RF15 = (RF->RF15 & 0x1fffffff) | 0x40000000;
	RF->RF18 &= 0xfff8ffff;
	RF->RF20 = (RF->RF20 & 0xfffff8ff) | 0x300;
	RF->RF23 |= 0x70000;
	RF->RF23 |= 0x700000;

	BB->BB14 = 0x2020c;
	BB->BB15 = 0x50;

// Enabled in BLECoreInit
//	NVIC->VTFIDR[3] = 0x14;

	BB->CTRL_TX = (BB->CTRL_TX & 0x1ffffff) | (TxPower | 0x40) << 0x19;
	BB->CTRL_CFG &= 0xfffffcff;
}

void DevSetMode(uint32_t mode) {
	if(mode) {
		BB->CTRL_CFG = (BB->CTRL_CFG & 0xfffcffff) | 0x20000;
		RF->RF2 |= 0x330000;
	}
	else {
		BB->CTRL_CFG = (BB->CTRL_CFG & 0xfffcffff) | 0x10000;
		RF->RF2 &= 0xffcdffff;
	}
	LL->CTRL_MOD = (0x30000 | mode);
}

uint32_t RFEND_TXCTune(uint8_t channel) {
	// 0xbf = 2401 MHz
	RF->RF1 &= 0xfffffffe;
	RF->TXTUNE_CTRL = (RF->TXTUNE_CTRL & 0xfffe00ff) | (0xbf00 + (channel_map[channel] << 8));
	RF->RF1 |= 1;

	LL->TMR = 8000;
	while(!(RF->TXCTUNE_CO_CTRL & (1 << 25)) || !(RF->TXCTUNE_CO_CTRL & (1 << 26))) {
		if(LL->TMR == 0) {
			break;
		}
	}

	uint8_t nCO = (uint8_t)RF->TXCTUNE_CO_CTRL & 0x3f;
	uint8_t nGA = (uint8_t)(RF->TXCTUNE_GA_CTRL >> 10) & 0x7f;

	// printf("nCO,nGA ch:%u idx:%u %u,%u\n", channel, channel_map[channel], nCO,nGA);

	return (nGA << 24) | nCO;
}

void RFEND_TXTune() {
	RF->RF1 &= 0xfffffeff;
	RF->RF10 &= 0xffffefff;
	RF->RF11 &= 0xffffffef;
	RF->RF2 |= 0x20000;
	RF->RF1 |= 0x10;

	// 2401 MHz
	uint32_t tune2401 = RFEND_TXCTune(37);
	uint8_t nCO2401 = (uint8_t)(tune2401 & 0x3f);
	uint8_t nGA2401 = (uint8_t)(tune2401 >> 24) & 0x7f;

	// 2480 MHz
	uint32_t tune2480 = RFEND_TXCTune(39);
	uint8_t nCO2480 = (uint8_t)(tune2480 & 0x3f);
	uint8_t nGA2480 = (uint8_t)(tune2480 >> 24) & 0x7f;

	// 2440 MHz
	uint32_t tune2440 = RFEND_TXCTune(18);
	uint8_t nCO2440 = (uint8_t)(tune2440 & 0x3f);
	uint8_t nGA2440 = (uint8_t)(tune2440 >> 24) & 0x7f;

	uint32_t dCO0140 = nCO2401 - nCO2440;
	uint32_t dCO4080 = nCO2440 - nCO2480;
	uint8_t tune = 0;
	uint8_t int_points = sizeof(RF->TXCTUNE_CO) /2;
	uint8_t txctune_co[sizeof(RF->TXCTUNE_CO)] = {0};
	for(int f = 0; f < int_points; f++) {
		tune = (dCO0140 * (int_points -f)) / int_points;
		txctune_co[f] = tune | (tune << 4);
	}
	for(int f = int_points; f < sizeof(RF->TXCTUNE_CO); f++) {
		tune = (dCO4080 * (f -int_points)) / int_points;
		txctune_co[f] = tune | (tune << 4);
	}
	for(int i = 0; i < sizeof(txctune_co) /4; i++) {
		RF->TXCTUNE_CO[i] = ((uint32_t*)txctune_co)[i];
	}

	// This GA interpolating is not exactly what is done in EVT
	// Actually the reception on a BLE monitor is better when this is left out completely
	// This will need some proper experimentation by people with 2.4GHz SDRs
#if 0
	uint32_t dGA0140 = nGA2401 - nGA2440;
	uint32_t dGA4080 = nGA2440 - nGA2480;
	int_points = sizeof(RF->TXCTUNE_GA) /2;
	uint8_t txctune_ga[sizeof(RF->TXCTUNE_GA)] = {0};
	for(int f = 1; f < int_points; f++) {
		tune = (dGA0140 * (int_points -f)) / int_points;
		txctune_ga[f] = tune | (tune << 4);
	}
	for(int f = int_points; f < sizeof(RF->TXCTUNE_GA) -1; f++) {
		tune = (dGA4080 * (f -int_points)) / int_points;
		txctune_ga[f] = tune | (tune << 4);
	}
	for(int i = 0; i < (sizeof(txctune_ga) /4); i++) {
		RF->TXCTUNE_GA[i] = ((uint32_t*)txctune_ga)[i];
	}
#endif


#if 1
	printf("2401 2440 2480 CO: %u %u %u, GA: %u %u %u\n", nCO2401, nCO2440, nCO2480, nGA2401, nGA2440, nGA2480);
	for(int i = 0; i < 10; i++ ) {
		printf( "%d: %08lx\n", i, RF->TXCTUNE_CO[i] );
	}
	for(int i = 0; i < 3; i++ ) {
		printf( "%d: %08lx\n", i, RF->TXCTUNE_GA[i] );
	}
#endif

	RF->RF1 &= 0xffffffef;
	RF->RF1 &= 0xfffffffe;
	RF->RF10 |= 0x1000;
	RF->RF11 |= 0x10;
	RF->TXTUNE_CTRL = (RF->TXTUNE_CTRL & 0xffffffc0) | (tune2440 & 0x3f);
	RF->TXTUNE_CTRL = (RF->TXTUNE_CTRL & 0x80ffffff) | (tune2440 & 0x7f000000);

	// FTune
	RF->RF1 |= 0x100;
}


void RFEND_RXTune() {
	RF->RF20 &= 0xfffeffff;
	RF->RF2 |= 0x200000;
	RF->RF3 = (RF->RF3 & 0xffffffef) | 0x10;
	RF->RF1 |= 0x1000;

	LL->TMR = 100;
	while(LL->TMR && ((RF->RXTUNE >> 8) & 1));

	tuneFilter = RF->RXTUNE & 0x1f;
	RF->RF20 |= 0x10000;
	RF->RF20 = (RF->RF20 & 0xffffffe0) | tuneFilter;
	RF->RF2 &= 0xffdfffff;

	// RXADC
	RF->RF22 &= 0xfffeffff;
	RF->RF2 |= 0x10000;
	RF->RF3 = (RF->RF3 & 0xfffffeff) | 0x100;
	RF->RF1 = (RF->RF1 & 0xfffeffff) | 0x100000;
}


void RegInit() {
	DevSetMode(0x0558);
	RFEND_TXTune();
	RFEND_RXTune();
	DevSetMode(0);
}

void BLECoreInit(uint8_t TxPower) {
	DevInit(TxPower);
	RegInit();

	NVIC_EnableIRQ( BB_IRQn );  /* BLEB */
	NVIC_EnableIRQ( LLE_IRQn ); /* BLEL */
}

void DevSetChannel(uint8_t channel) {
	RF->RF11 &= 0xfffffffd;
	BB->CTRL_CFG = (BB->CTRL_CFG & 0xffffff80) | (channel & 0x7f);
}

void Advertise(uint8_t adv[], size_t len, uint8_t channel) {
	__attribute__((aligned(4))) uint8_t  ADV_BUF[len+2]; // for the advertisement, which is 37 bytes + 2 header bytes

	BB->CTRL_TX = (BB->CTRL_TX & 0xfffffffc) | 1;

	DevSetChannel(channel);

	// Uncomment to disable whitening to debug RF.
	//BB->CTRL_CFG |= (1<<6);
	DevSetMode(0x0258);

	BB->ACCESSADDRESS1 = 0x8E89BED6; // access address
	BB->ACCESSADDRESS2 = 0x8E89BED6;
	BB->CRCINIT1 = 0x555555; // crc init
	BB->CRCINIT2 = 0x555555;
	BB->CRCPOLY1 = (BB->CRCPOLY1 & 0xff000000) | 0x80032d; // crc poly
	BB->CRCPOLY2 = (BB->CRCPOLY2 & 0xff000000) | 0x80032d;

	LL->LL1 |= 1; // Unknown why this needs to happen.

	ADV_BUF[0] = 0x02; // PDU 0x00, 0x02, 0x06 seem to work, with only 0x02 showing up on the phone
	ADV_BUF[1] = len ;
	memcpy(&ADV_BUF[2], adv, len);
	LL->FRAME_BUF = (uint32_t)ADV_BUF;

	// Wait for tuning bit to clear.
	for( int timeout = 3000; !(RF->RF26 & 0x1000000) && timeout >= 0; timeout-- );

	//PHYSetTxMode(len);
	BB->CTRL_CFG = (BB->CTRL_CFG & 0xfffffcff) | 0x100;

	// Confiugre 1MHz mode.  Unset 0x2000000 to switch to 2MHz bandwidth mode.)
	// Note: There's probably something else that must be set if in 2MHz mode.
	BB->BB9 = (BB->BB9 & 0xf9ffffff) | 0x2000000;

	// This clears bit 17 (If set, seems to have no impact.)
	LL->LL4 &= 0xfffdffff;

	LL->STATUS = 0x20000;
	LL->TMR = (uint32_t)(((len *8) + 0xee) *2);

	BB->CTRL_CFG |= 0x1000000;
	BB->CTRL_TX &= 0xfffffffc;

	LL->LL0 = 0x02; // Not sure what this does.  Does not seem to be critical.

	while(LL->TMR); // wait for tx buffer to empty
	DevSetMode(0);
	LL->CTRL_MOD &= 0xfffff8ff;
	LL->LL0 |= 0x08;
}

void Frame_RX(uint8_t adv[], uint8_t channel) {

	// What is this?
/*
	if(LL->LL0 & 3) {
		LL->CTRL_MOD &= 0xfffff8ff;
		LL->LL0 |= 0x08;
	}
*/
	LL->TMR = 0;

	DevSetChannel(channel);

	BB->CTRL_CFG = (BB->CTRL_CFG & 0xfffffcff) | 0x100; // Must set this code, other codes do not RX.

	// PLEASE NOTE: THIS DOES NOT ACTUALLY WORK ON RX - do not know why we can't RX 2MBPS mode.
	int is2MBPS = 0;
	BB->BB9 = (BB->BB9 & 0xf9ffffff) | (is2MBPS?0:0x2000000);
	int filter = tuneFilter;
	if(is2MBPS)
	{
		filter += 2;
		if( filter > 0x1f ) filter = 0x1f;
	}

	RF->RF20 = (RF->RF20 & 0xffffffe0) | (filter & 0x1f);
	BB->BB5 = (BB->BB5 & 0xffffffc0) | 0xb;
	BB->BB6 = 0x78; // this is probably wrong
	BB->BB7 = (BB->BB7 & 0xfffffc00) | 0x9c;

	BB->ACCESSADDRESS1 = 0x8E89BED6; // access address
	BB->ACCESSADDRESS2 = 0x8E89BED6;
	BB->CRCINIT1 = 0x555555; // crc init
	BB->CRCINIT2 = 0x555555;
	BB->CRCPOLY1 = (BB->CRCPOLY1 & 0xff000000) | 0x80032d; // crc poly
	BB->CRCPOLY2 = (BB->CRCPOLY2 & 0xff000000) | 0x80032d;

	// If you set this to 1, you MUST execute the two statements at the end.
	// Otherwise it just goooooes
	LL->LL1 = (LL->LL1 & 0xfffffffe) | 0;

	// I seriously question if this is right or if FRAME_BUF is for TX only.
	LL->FRAME_BUF = (uint32_t)adv;

	// TODO: For the future: There is a very interesting thing that happens when you set BB15
	// to some larger value, you get an absolute monster spew of interrupts.  I think some bit in
	// there let's you skeet ahead.

	// valid modes = 059-359, but needs to be 59
	// Interesting setting a few other bits works too, like 0x3ff
	DevSetMode(0x379);

	LL->LL0 = 1; // Not sure what this does, but on TX it's 2 -- this definitely controls TX vs RX

	//while(LL->TMR);

//	Delay_Ms(100);
//	DevSetMode(0);

	// This will trigger an LL ISR, not sure why.  Maybe it's so you can know you can switch to a TX packet?
	// I.e. when you want to send a packet, you can call this, then send the packet and switch back to RX?
//	LL->CTRL_MOD &= 0xfffff8ff;
//	LL->LL0 |= 0x08;
}

