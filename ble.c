#include "ch32fun.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define BB_BASE (0x4000c100)
#define LL_BASE (0x4000c200)
#define RF_BASE (0x4000d000)

#define BB ((BB_Type *) BB_BASE)
#define LL ((LL_Type *) LL_BASE)
#define RF ((RF_Type *) RF_BASE)

typedef struct{
    volatile uint32_t BB0;
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
    volatile uint32_t BB13;
    volatile uint32_t BB14;
    volatile uint32_t BB15;
    volatile uint32_t BB16;
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
    volatile uint32_t LL3;
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
    volatile uint32_t LL20;
    volatile uint32_t LL21;
    volatile uint32_t LL22;
    volatile uint32_t LL23;
    volatile uint32_t LL24;
    volatile uint32_t LL25;
    volatile uint32_t LL26;
    volatile uint32_t LL27;
    volatile uint32_t LL28;
    volatile uint32_t LL29;
    volatile uint32_t LL30;
    volatile uint32_t LL31;
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
    volatile uint32_t RF14;
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
    volatile uint32_t RF_TXCTUNE_CO;
    volatile uint32_t RF_TXCTUNE_GA;
    volatile uint32_t RF38;
    volatile uint32_t RF39;
    volatile uint32_t RF_TXCTUNE_0;
    volatile uint32_t RF_TXCTUNE_1;
    volatile uint32_t RF_TXCTUNE_2;
    volatile uint32_t RF_TXCTUNE_3;
    volatile uint32_t RF_TXCTUNE_4;
    volatile uint32_t RF_TXCTUNE_5;
    volatile uint32_t RF_TXCTUNE_6;
    volatile uint32_t RF_TXCTUNE_7;
    volatile uint32_t RF_TXCTUNE_8;
    volatile uint32_t RF_TXCTUNE_9;
    volatile uint32_t RF_TXCTUNE_10;
    volatile uint32_t RF_TXCTUNE_11;
    volatile uint32_t RF_TXCTUNE_12;
} RF_Type;

__attribute__((aligned(4))) uint32_t LL_BUF[0x10c];
__attribute__((aligned(4))) uint8_t  ADV_BUF[40]; // for the advertisement, which is 37 bytes + 2 header bytes

__attribute__((interrupt))
__attribute__((section(".highcode")))
void LLE_IRQHandler() {
	LL->STATUS = 0xffffffff;
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
	LL->LL31 = (uint32_t)LL_BUF;
	LL->STATUS = 0xffffffff;
	LL->LL3 = 0x16000f;

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

	NVIC->VTFIDR[3] = 0x14;

	BB->BB13 = (BB->BB13 & 0x1ffffff) | (TxPower | 0x40) << 0x19;
	BB->BB0 &= 0xfffffcff;
}

void RFEND_TxTuneWait() {
	LL->LL25 = 8000;
	while((-1 < (int32_t)RF->RF_TXCTUNE_CO << 5) || (-1 < (int32_t)RF->RF_TXCTUNE_CO << 6)) {
		if(LL->LL25 == 0) {
			break;
		}
	}
}

void RFEND_TXTune() {
	RF->RF1 &= 0xfffffeff;
	RF->RF10 &= 0xffffefff;
	RF->RF11 &= 0xffffffef;
	RF->RF2 |= 0x20000;
	RF->RF1 |= 0x10;

	// 2401 MHz
	RF->RF1 &= 0xfffffffe;
	RF->RF14 = (RF->RF14 & 0xfffe00ff) | 0xbf00;
	RF->RF1 |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2401 = (uint8_t)RF->RF_TXCTUNE_CO & 0x3f;
	uint8_t nGA2401 = (uint8_t)(RF->RF_TXCTUNE_GA >> 10) & 0x7f;

	// 2480 MHz
	RF->RF1 &= 0xfffffffe;
	RF->RF14 = (RF->RF14 & 0xfffe00ff) | 0xe700;
	RF->RF1 |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2480 = (uint8_t)RF->RF_TXCTUNE_CO & 0x3f;
	uint8_t nGA2480 = (uint8_t)(RF->RF_TXCTUNE_GA >> 10) & 0x7f;

	// 2440 MHz
	RF->RF1 &= 0xfffffffe;
	RF->RF14 = (RF->RF14 & 0xfffe00ff) | 0xd300;
	RF->RF1 |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2440 = (uint8_t)RF->RF_TXCTUNE_CO & 0x3f;
	uint8_t nGA2440 = (uint8_t)(RF->RF_TXCTUNE_GA >> 10) & 0x7f;

	uint32_t dCO0140 = nCO2401 - nCO2440;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xfffffff0) | (dCO0140 & 0xf);
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xffffff0f) | ((int)(dCO0140 * 0x26) / 0x27 & 0xfU) << 4;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xfffff0ff) | ((int)(dCO0140 * 0x25) / 0x27 & 0xfU) << 8;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xffff0fff) | ((int)(dCO0140 * 0x24) / 0x27 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xfff0ffff) | ((int)(dCO0140 * 0x23) / 0x27 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xff0fffff) | ((int)(dCO0140 * 0x22) / 0x27 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0xf0ffffff) | ((int)(dCO0140 * 0x21) / 0x27 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_0 = (RF->RF_TXCTUNE_0 & 0x0fffffff) | (int)(dCO0140 * 0x20) / 0x27 << 0x1c;

	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xfffffff0) | ((int)(dCO0140 * 0x1f) / 0x27 & 0xfU);
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xffffff0f) | ((int)(dCO0140 * 0x1e) / 0x27 & 0xfU) << 4;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xfffff0ff) | ((int)(dCO0140 * 0x1d) / 0x27 & 0xfU) << 8;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xffff0fff) | ((int)(dCO0140 * 0x1c) / 0x27 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xfff0ffff) | ((int)(dCO0140 * 0x1b) / 0x27 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xff0fffff) | ((int)(dCO0140 * 0x1a) / 0x27 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0xf0ffffff) | ((int)(dCO0140 * 0x19) / 0x27 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_1 = (RF->RF_TXCTUNE_1 & 0x0fffffff) | (int)(dCO0140 * 0x18) / 0x27 << 0x1c;

	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xfffffff0) | ((int)(dCO0140 * 0x17) / 0x27 & 0xfU);
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xffffff0f) | ((int)(dCO0140 * 0x16) / 0x27 & 0xfU) << 4;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xfffff0ff) | ((int)(dCO0140 * 0x15) / 0x27 & 0xfU) << 8;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xffff0fff) | ((int)(dCO0140 * 0x14) / 0x27 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xfff0ffff) | ((int)(dCO0140 * 0x13) / 0x27 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xff0fffff) | ((int)(dCO0140 * 0x12) / 0x27 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0xf0ffffff) | ((int)(dCO0140 * 0x11) / 0x27 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_2 = (RF->RF_TXCTUNE_2 & 0x0fffffff) | (int)(dCO0140 * 0x10) / 0x27 << 0x1c;

	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xfffffff0) | ((int)(dCO0140 * 0xf) / 0x27 & 0xfU);
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xffffff0f) | ((int)(dCO0140 * 0xe) / 0x27 & 0xfU) << 4;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xfffff0ff) | ((int)dCO0140 / 3 & 0xfU) << 8;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xffff0fff) | ((int)(dCO0140 * 0xc) / 0x27 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xfff0ffff) | ((int)(dCO0140 * 0xb) / 0x27 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xff0fffff) | ((int)(dCO0140 * 10) / 0x27 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0xf0ffffff) | ((int)(dCO0140 * 9) / 0x27 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_3 = (RF->RF_TXCTUNE_3 & 0x0fffffff) | (int)(dCO0140 * 8) / 0x27 << 0x1c;

	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xfffffff0) | ((int)(dCO0140 * 7) / 0x27 & 0xfU);
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xffffff0f) | ((int)(dCO0140 * 6) / 0x27 & 0xfU) << 4;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xfffff0ff) | ((int)(dCO0140 * 5) / 0x27 & 0xfU) << 8;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xffff0fff) | ((int)(dCO0140 * 4) / 0x27 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xfff0ffff) | ((int)dCO0140 / 0xd & 0xfU) << 0x10;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xff0fffff) | ((int)(dCO0140 * 2) / 0x27 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0xf0ffffff) | ((int)dCO0140 / 0x27 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_4 = (RF->RF_TXCTUNE_4 & 0x0fffffff);

	uint32_t dCO4080 = nCO2440 - nCO2480;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xfffffff0) | (dCO4080 / 0x28 & 0xfU);
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xffffff0f) | (dCO4080 / 0x14 & 0xfU) << 4;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xfffff0ff) | ((dCO4080 * 3) / 0x28 & 0xfU) << 8;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xffff0fff) | (dCO4080 / 10 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xfff0ffff) | (dCO4080 / 8 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xff0fffff) | ((dCO4080 * 6) / 0x28 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0xf0ffffff) | ((dCO4080 * 7) / 0x28 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_5 = (RF->RF_TXCTUNE_5 & 0x0fffffff) | dCO4080 / 5 << 0x1c;

	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xfffffff0) | ((dCO4080 * 9) / 0x28 & 0xfU);
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xffffff0f) | (dCO4080 / 4 & 0xfU) << 4;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xfffff0ff) | ((dCO4080 * 0xb) / 0x28 & 0xfU) << 8;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xffff0fff) | ((dCO4080 * 0xc) / 0x28 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xfff0ffff) | ((dCO4080 * 0xd) / 0x28 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xff0fffff) | ((dCO4080 * 0xe) / 0x28 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0xf0ffffff) | ((dCO4080 * 0xf) / 0x28 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_6 = (RF->RF_TXCTUNE_6 & 0x0fffffff) | (dCO4080 * 0x10) / 0x28 << 0x1c;

	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xfffffff0) | ((dCO4080 * 0x11) / 0x28 & 0xfU);
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xffffff0f) | ((dCO4080 * 0x12) / 0x28 & 0xfU) << 4;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xfffff0ff) | ((dCO4080 * 0x13) / 0x28 & 0xfU) << 8;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xffff0fff) | (dCO4080 / 2 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xfff0ffff) | ((dCO4080 * 0x15) / 0x28 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xff0fffff) | ((dCO4080 * 0x16) / 0x28 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0xf0ffffff) | ((dCO4080 * 0x17) / 0x28 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_7 = (RF->RF_TXCTUNE_7 & 0x0fffffff) | (dCO4080 * 0x18) / 0x28 << 0x1c;

	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xfffffff0) | ((dCO4080 * 0x19) / 0x28 & 0xfU);
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xffffff0f) | ((dCO4080 * 0x1a) / 0x28 & 0xfU) << 4;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xfffff0ff) | ((dCO4080 * 0x1b) / 0x28 & 0xfU) << 8;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xffff0fff) | ((dCO4080 * 0x1c) / 0x28 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xfff0ffff) | ((dCO4080 * 0x1d) / 0x28 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xff0fffff) | ((dCO4080 * 0x1e) / 0x28 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0xf0ffffff) | ((dCO4080 * 0x1f) / 0x28 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_8 = (RF->RF_TXCTUNE_8 & 0x0fffffff) | (dCO4080 * 0x20) / 0x28 << 0x1c;

	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xfffffff0) | ((dCO4080 * 0x21) / 0x28 & 0xfU);
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xffffff0f) | ((dCO4080 * 0x22) / 0x28 & 0xfU) << 4;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xfffff0ff) | ((dCO4080 * 0x23) / 0x28 & 0xfU) << 8;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xffff0fff) | ((dCO4080 * 0x24) / 0x28 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xfff0ffff) | ((dCO4080 * 0x25) / 0x28 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xff0fffff) | ((dCO4080 * 0x26) / 0x28 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0xf0ffffff) | ((dCO4080 * 0x27) / 0x28 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_9 = (RF->RF_TXCTUNE_9 & 0x0fffffff) | dCO4080 * 0x10000000;

	uint32_t dGA4080 = nGA2440 - nGA2480;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xfffffff0) | ((dCO4080 * 0x29) / 0x28 & 0xfU);
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xffffff0f) | ((dCO4080 * 0x2a) / 0x28 & 0xfU) << 4;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xfffff0ff) | (((dGA4080 * 10) / dCO4080 & 0xfU) | 8) << 8;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xffff0fff) | (((dGA4080 * 9) / dCO4080 & 0xfU) | 8) << 0xc;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xfff0ffff) | (((dGA4080 * 8) / dCO4080 & 0xfU) | 8) << 0x10;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xff0fffff) | (((dGA4080 * 7) / dCO4080 & 0xfU) | 8) << 0x14;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0xf0ffffff) | (((dGA4080 * 6) / dCO4080 & 0xfU) | 8) << 0x18;
	RF->RF_TXCTUNE_10 = (RF->RF_TXCTUNE_10 & 0x0fffffff) | ((dGA4080 * 5) / dCO4080 | 8U) << 0x1c;
	
	uint32_t dGA0140 = nGA2401 - nGA2440;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xfffffff0) | ((dGA4080 * 4) / dCO4080 & 0xfU) | 8;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xffffff0f) | (((dGA4080 * 3) / dCO4080 & 0xfU) | 8) << 4;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xfffff0ff) | (((dGA4080 * 2) / dCO4080 & 0xfU) | 8) << 8;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xffff0fff) | ((dGA4080 / dCO4080 & 0xfU) | 8) << 0xc;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xfff0ffff);
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xff0fffff) | (dGA0140 / (int)dCO0140 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0xf0ffffff) | ((dGA0140 * 2) / (int)dCO0140 & 0xfU) << 0x18;
	RF->RF_TXCTUNE_11 = (RF->RF_TXCTUNE_11 & 0x0fffffff) | (dGA0140 * 3) / (int)dCO0140 << 0x1c;

	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xfffffff0) | ((dGA0140 * 4) / (int)dCO0140 & 0xfU);
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xffffff0f) | ((dGA0140 * 5) / (int)dCO0140 & 0xfU) << 4;
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xfffff0ff) | ((dGA0140 * 6) / (int)dCO0140 & 0xfU) << 8;
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xffff0fff) | ((dGA0140 * 7) / (int)dCO0140 & 0xfU) << 0xc;
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xfff0ffff) | ((dGA0140 * 8) / (int)dCO0140 & 0xfU) << 0x10;
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xff0fffff) | ((dGA0140 * 9) / (int)dCO0140 & 0xfU) << 0x14;
	RF->RF_TXCTUNE_12 = (RF->RF_TXCTUNE_12 & 0xf0ffffff) | ((dGA0140 * 10) / (int)dCO0140 & 0xfU) << 0x18;

	RF->RF1 &= 0xffffffef;
	RF->RF1 &= 0xfffffffe;
	RF->RF10 |= 0x1000;
	RF->RF11 |= 0x10;
	RF->RF14 = (RF->RF14 & 0xffffffc0) | (nCO2440 & 0x3f);
	RF->RF14 = ((nGA2440 & 0x7f) << 0x18) | (RF->RF14 & 0x80ffffff);

	// FTune
	RF->RF1 |= 0x100;
}

void RegInit() {
	BB->BB0 &= 0xfffeffff;
	RF->RF2 |= 0x330000;
	LL->LL20 = 0x30558;
	RFEND_TXTune();
	BB->BB0 &= 0xfffdffff;
	RF->RF2 &= 0xffcdffff;
	LL->LL20 = 0x30000;
}

void BLECoreInit(uint8_t TxPower) {
	DevInit(TxPower);
	RegInit();
	NVIC->IPRIOR[0x15] |= 0x80;
	NVIC->IENR[0] = 0x200000;
}

__attribute__((section(".highcode")))
void DevSetChannel(uint8_t channel) {
	RF->RF11 &= 0xfffffffd;
	BB->BB0 = (BB->BB0 & 0xffffff80) | (channel & 0x7f);
}

__attribute__((section(".highcode")))
void PHYSetTxMode(size_t len) {
	BB->BB0 = (BB->BB0 & 0xfffffcff) | 0x100;
	BB->BB9 = (BB->BB9 & 0xf9ffffff) | 0x2000000;
	LL->LL4 &= 0xfffdffff;
	LL->STATUS = 0x20000;
	LL->LL25 = (uint32_t)(((len *8) + 0xee) *2);
}

__attribute__((section(".highcode")))
void RF_Stop() {
	BB->BB0 &= 0xfffdffff;
	RF->RF2 &= 0xffcdffff;
	LL->LL20 &= 0x30000;

	LL->LL20 &= 0xfffff8ff;
	LL->LL0 |= 0x08;
}

__attribute__((section(".highcode")))
void Advertise(uint8_t adv[], size_t len, uint8_t channel) {
	BB->BB13 = (BB->BB13 & 0xfffffffc) | 1;

	DevSetChannel(channel);

	BB->BB0 &= 0xfffeffff;
	RF->RF2 |= 0x330000;
	LL->LL20 = 0x30258;

	BB->ACCESSADDRESS1 = 0x8E89BED6; // access address
	BB->ACCESSADDRESS2 = 0x8E89BED6;
	BB->CRCINIT1 = 0x555555; // crc init
	BB->CRCINIT2 = 0x555555;
	BB->CRCPOLY1 = (BB->CRCPOLY1 & 0xff000000) | 0x80032d; // crc poly
	BB->CRCPOLY2 = (BB->CRCPOLY2 & 0xff000000) | 0x80032d;
	LL->LL1 = (LL->LL1 & 0xfffffffe) | 1;

	ADV_BUF[0] = 0x02; //TxPktType 0x00, 0x02, 0x06 seem to work, with only 0x02 showing up on the phone
	ADV_BUF[1] = len ;
	memcpy(&ADV_BUF[2], adv, len);
	LL->LL30 = (uint32_t)ADV_BUF;

	PHYSetTxMode(len);

	BB->BB0 |= 0x1000000;
	BB->BB13 &= 0xfffffffc;
	LL->LL0 = 0x02;

	while(LL->LL25); // wait for tx buffer to empty
	RF_Stop();
}
