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
	volatile uint32_t CTRL_TX;
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
	volatile uint32_t CTRL_MOD;
	volatile uint32_t LL21;
	volatile uint32_t LL22;
	volatile uint32_t LL23;
	volatile uint32_t LL24;
	volatile uint32_t LL25;
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
	volatile uint32_t RF_TXCTUNE[13];
} RF_Type;

__attribute__((aligned(4))) uint32_t LLE_BUF[0x10c];
__attribute__((aligned(4))) uint8_t  ADV_BUF[40]; // for the advertisement, which is 37 bytes + 2 header bytes

__attribute__((interrupt))
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

	NVIC->VTFIDR[3] = 0x14;

	BB->CTRL_TX = (BB->CTRL_TX & 0x1ffffff) | (TxPower | 0x40) << 0x19;
	BB->CTRL_CFG &= 0xfffffcff;
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
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xfffffff0) | ((dCO0140 * 39) / 39 & 0xfU);
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xffffff0f) | ((dCO0140 * 38) / 39 & 0xfU) << 4;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xfffff0ff) | ((dCO0140 * 37) / 39 & 0xfU) << 8;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xffff0fff) | ((dCO0140 * 36) / 39 & 0xfU) << 12;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xfff0ffff) | ((dCO0140 * 35) / 39 & 0xfU) << 16;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xff0fffff) | ((dCO0140 * 34) / 39 & 0xfU) << 20;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0xf0ffffff) | ((dCO0140 * 33) / 39 & 0xfU) << 24;
	RF->RF_TXCTUNE[0] = (RF->RF_TXCTUNE[0] & 0x0fffffff) | ((dCO0140 * 32) / 39 & 0xfU) << 28;

	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xfffffff0) | ((dCO0140 * 31) / 39 & 0xfU);
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xffffff0f) | ((dCO0140 * 30) / 39 & 0xfU) << 4;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xfffff0ff) | ((dCO0140 * 29) / 39 & 0xfU) << 8;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xffff0fff) | ((dCO0140 * 28) / 39 & 0xfU) << 12;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xfff0ffff) | ((dCO0140 * 27) / 39 & 0xfU) << 16;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xff0fffff) | ((dCO0140 * 26) / 39 & 0xfU) << 20;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0xf0ffffff) | ((dCO0140 * 25) / 39 & 0xfU) << 24;
	RF->RF_TXCTUNE[1] = (RF->RF_TXCTUNE[1] & 0x0fffffff) | ((dCO0140 * 24) / 39 & 0xfU) << 28;

	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xfffffff0) | ((dCO0140 * 23) / 39 & 0xfU);
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xffffff0f) | ((dCO0140 * 22) / 39 & 0xfU) << 4;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xfffff0ff) | ((dCO0140 * 21) / 39 & 0xfU) << 8;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xffff0fff) | ((dCO0140 * 20) / 39 & 0xfU) << 12;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xfff0ffff) | ((dCO0140 * 19) / 39 & 0xfU) << 16;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xff0fffff) | ((dCO0140 * 18) / 39 & 0xfU) << 20;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0xf0ffffff) | ((dCO0140 * 17) / 39 & 0xfU) << 24;
	RF->RF_TXCTUNE[2] = (RF->RF_TXCTUNE[2] & 0x0fffffff) | ((dCO0140 * 16) / 39 & 0xfU) << 28;

	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xfffffff0) | ((dCO0140 * 15) / 39 & 0xfU);
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xffffff0f) | ((dCO0140 * 14) / 39 & 0xfU) << 4;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xfffff0ff) | ((dCO0140 * 13) / 39 & 0xfU) << 8;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xffff0fff) | ((dCO0140 * 12) / 39 & 0xfU) << 12;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xfff0ffff) | ((dCO0140 * 11) / 39 & 0xfU) << 16;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xff0fffff) | ((dCO0140 * 10) / 39 & 0xfU) << 20;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0xf0ffffff) | ((dCO0140 * 9) / 39 & 0xfU) << 24;
	RF->RF_TXCTUNE[3] = (RF->RF_TXCTUNE[3] & 0x0fffffff) | ((dCO0140 * 8) / 39 & 0xfU) << 28;

	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xfffffff0) | ((dCO0140 * 7) / 39 & 0xfU);
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xffffff0f) | ((dCO0140 * 6) / 39 & 0xfU) << 4;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xfffff0ff) | ((dCO0140 * 5) / 39 & 0xfU) << 8;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xffff0fff) | ((dCO0140 * 4) / 39 & 0xfU) << 12;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xfff0ffff) | ((dCO0140 * 3) / 39 & 0xfU) << 16;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xff0fffff) | ((dCO0140 * 2) / 39 & 0xfU) << 20;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0xf0ffffff) | ((dCO0140 * 1) / 39 & 0xfU) << 24;
	RF->RF_TXCTUNE[4] = (RF->RF_TXCTUNE[4] & 0x0fffffff) | ((dCO0140 * 0) / 39 & 0xfU) << 28;

	uint32_t dCO4080 = nCO2440 - nCO2480;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xfffffff0) | ((dCO4080 * 1) / 40 & 0xfU);
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xffffff0f) | ((dCO4080 * 2) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xfffff0ff) | ((dCO4080 * 3) / 40 & 0xfU) << 8;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xffff0fff) | ((dCO4080 * 4) / 40 & 0xfU) << 12;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xfff0ffff) | ((dCO4080 * 5) / 40 & 0xfU) << 16;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xff0fffff) | ((dCO4080 * 6) / 40 & 0xfU) << 20;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0xf0ffffff) | ((dCO4080 * 7) / 40 & 0xfU) << 24;
	RF->RF_TXCTUNE[5] = (RF->RF_TXCTUNE[5] & 0x0fffffff) | ((dCO4080 * 8) / 30 & 0xfU) << 28;

	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xfffffff0) | ((dCO4080 * 9) / 40 & 0xfU);
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xffffff0f) | ((dCO4080 * 10) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xfffff0ff) | ((dCO4080 * 11) / 40 & 0xfU) << 8;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xffff0fff) | ((dCO4080 * 12) / 40 & 0xfU) << 12;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xfff0ffff) | ((dCO4080 * 13) / 40 & 0xfU) << 16;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xff0fffff) | ((dCO4080 * 14) / 40 & 0xfU) << 20;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0xf0ffffff) | ((dCO4080 * 15) / 40 & 0xfU) << 24;
	RF->RF_TXCTUNE[6] = (RF->RF_TXCTUNE[6] & 0x0fffffff) | ((dCO4080 * 16) / 40 & 0xfU) << 28;

	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xfffffff0) | ((dCO4080 * 17) / 40 & 0xfU);
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xffffff0f) | ((dCO4080 * 18) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xfffff0ff) | ((dCO4080 * 19) / 40 & 0xfU) << 8;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xffff0fff) | ((dCO4080 * 20) / 40 & 0xfU) << 12;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xfff0ffff) | ((dCO4080 * 21) / 40 & 0xfU) << 16;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xff0fffff) | ((dCO4080 * 22) / 40 & 0xfU) << 20;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0xf0ffffff) | ((dCO4080 * 23) / 40 & 0xfU) << 24;
	RF->RF_TXCTUNE[7] = (RF->RF_TXCTUNE[7] & 0x0fffffff) | ((dCO4080 * 24) / 40 & 0xfU) << 28;

	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xfffffff0) | ((dCO4080 * 25) / 40 & 0xfU);
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xffffff0f) | ((dCO4080 * 26) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xfffff0ff) | ((dCO4080 * 27) / 40 & 0xfU) << 8;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xffff0fff) | ((dCO4080 * 28) / 40 & 0xfU) << 12;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xfff0ffff) | ((dCO4080 * 29) / 40 & 0xfU) << 16;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xff0fffff) | ((dCO4080 * 30) / 40 & 0xfU) << 20;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0xf0ffffff) | ((dCO4080 * 31) / 40 & 0xfU) << 24;
	RF->RF_TXCTUNE[8] = (RF->RF_TXCTUNE[8] & 0x0fffffff) | ((dCO4080 * 32) / 40 & 0xfU) << 28;

	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xfffffff0) | ((dCO4080 * 33) / 40 & 0xfU);
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xffffff0f) | ((dCO4080 * 34) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xfffff0ff) | ((dCO4080 * 35) / 40 & 0xfU) << 8;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xffff0fff) | ((dCO4080 * 36) / 40 & 0xfU) << 12;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xfff0ffff) | ((dCO4080 * 37) / 40 & 0xfU) << 16;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xff0fffff) | ((dCO4080 * 38) / 40 & 0xfU) << 20;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0xf0ffffff) | ((dCO4080 * 39) / 40 & 0xfU) << 24;
	RF->RF_TXCTUNE[9] = (RF->RF_TXCTUNE[9] & 0x0fffffff) | dCO4080 * 0x10000000;

	uint32_t dGA4080 = nGA2440 - nGA2480;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xfffffff0) | ((dCO4080 * 41) / 40 & 0xfU);
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xffffff0f) | ((dCO4080 * 42) / 40 & 0xfU) << 4;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xfffff0ff) | (((dGA4080 * 10) / dCO4080 & 0xfU) | 8) << 8;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xffff0fff) | (((dGA4080 * 9) / dCO4080 & 0xfU) | 8) << 12;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xfff0ffff) | (((dGA4080 * 8) / dCO4080 & 0xfU) | 8) << 16;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xff0fffff) | (((dGA4080 * 7) / dCO4080 & 0xfU) | 8) << 20;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0xf0ffffff) | (((dGA4080 * 6) / dCO4080 & 0xfU) | 8) << 24;
	RF->RF_TXCTUNE[10] = (RF->RF_TXCTUNE[10] & 0x0fffffff) | (((dGA4080 * 5) / dCO4080 & 0xfU) | 8) << 28;
	
	uint32_t dGA0140 = nGA2401 - nGA2440;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xfffffff0) | (((dGA4080 * 4) / dCO4080 & 0xfU) | 8);
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xffffff0f) | (((dGA4080 * 3) / dCO4080 & 0xfU) | 8) << 4;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xfffff0ff) | (((dGA4080 * 2) / dCO4080 & 0xfU) | 8) << 8;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xffff0fff) | (((dGA4080 * 1) / dCO4080 & 0xfU) | 8) << 12;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xfff0ffff);
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xff0fffff) | ((dGA0140 * 1) / (int)dCO0140 & 0xfU) << 20;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0xf0ffffff) | ((dGA0140 * 2) / (int)dCO0140 & 0xfU) << 24;
	RF->RF_TXCTUNE[11] = (RF->RF_TXCTUNE[11] & 0x0fffffff) | ((dGA0140 * 3) / (int)dCO0140 & 0xfU)<< 28;

	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xfffffff0) | ((dGA0140 * 4) / (int)dCO0140 & 0xfU);
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xffffff0f) | ((dGA0140 * 5) / (int)dCO0140 & 0xfU) << 4;
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xfffff0ff) | ((dGA0140 * 6) / (int)dCO0140 & 0xfU) << 8;
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xffff0fff) | ((dGA0140 * 7) / (int)dCO0140 & 0xfU) << 12;
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xfff0ffff) | ((dGA0140 * 8) / (int)dCO0140 & 0xfU) << 16;
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xff0fffff) | ((dGA0140 * 9) / (int)dCO0140 & 0xfU) << 20;
	RF->RF_TXCTUNE[12] = (RF->RF_TXCTUNE[12] & 0xf0ffffff) | ((dGA0140 * 10) / (int)dCO0140 & 0xfU) << 24;

	RF->RF1 &= 0xffffffef;
	RF->RF1 &= 0xfffffffe;
	RF->RF10 |= 0x1000;
	RF->RF11 |= 0x10;
	RF->RF14 = (RF->RF14 & 0xffffffc0) | (nCO2440 & 0x3f);
	RF->RF14 = (RF->RF14 & 0x80ffffff) | ((nGA2440 & 0x7f) << 0x18);

	// FTune
	RF->RF1 |= 0x100;
}

void RegInit() {
	BB->CTRL_CFG &= 0xfffeffff;
	RF->RF2 |= 0x330000;
	LL->CTRL_MOD = 0x30558;
	RFEND_TXTune();
	BB->CTRL_CFG &= 0xfffdffff;
	RF->RF2 &= 0xffcdffff;
	LL->CTRL_MOD = 0x30000;
}

void BLECoreInit(uint8_t TxPower) {
	DevInit(TxPower);
	RegInit();
	NVIC->IPRIOR[0x15] |= 0x80;
	NVIC->IENR[0] = 0x200000;
}

void DevSetChannel(uint8_t channel) {
	RF->RF11 &= 0xfffffffd;
	BB->CTRL_CFG = (BB->CTRL_CFG & 0xffffff80) | (channel & 0x7f);
}

void PHYSetTxMode(size_t len) {
	BB->CTRL_CFG = (BB->CTRL_CFG & 0xfffffcff) | 0x100;
	BB->BB9 = (BB->BB9 & 0xf9ffffff) | 0x2000000;
	LL->LL4 &= 0xfffdffff;
	LL->STATUS = 0x20000;
	LL->LL25 = (uint32_t)(((len *8) + 0xee) *2);
}

void RF_Stop() {
	BB->CTRL_CFG &= 0xfffdffff;
	RF->RF2 &= 0xffcdffff;
	LL->CTRL_MOD &= 0x30000;

	LL->CTRL_MOD &= 0xfffff8ff;
	LL->LL0 |= 0x08;
}

void Advertise(uint8_t adv[], size_t len, uint8_t channel) {
	BB->CTRL_TX = (BB->CTRL_TX & 0xfffffffc) | 1;

	DevSetChannel(channel);

	BB->CTRL_CFG &= 0xfffeffff;
	RF->RF2 |= 0x330000;
	LL->CTRL_MOD = 0x30258;

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
	LL->FRAME_BUF = (uint32_t)ADV_BUF;

	PHYSetTxMode(len);

	BB->CTRL_CFG |= 0x1000000;
	BB->CTRL_TX &= 0xfffffffc;
	LL->LL0 = 0x02;

	while(LL->LL25); // wait for tx buffer to empty
	RF_Stop();
}
