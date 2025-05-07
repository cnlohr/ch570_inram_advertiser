#include "ch32fun.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

uint32_t volatile *gptrLLEReg;
uint32_t volatile *gptrRFENDReg;
uint32_t volatile *gptrBBReg;

__attribute__((aligned(4))) uint32_t LLE_BUF[0x10c];
__attribute__((aligned(4))) uint8_t  ADV_BUF[40]; // for the advertisement, which is 37 bytes + 2 header bytes

__attribute__((interrupt))
__attribute__((section(".highcode")))
void LLE_IRQHandler() {
	gptrLLEReg[2] = 0xffffffff; // STATUS
}

void DevInit(uint8_t TxPower) {
	gptrLLEReg[5] = 0x8c;
	gptrLLEReg[7] = 0x76;
	gptrLLEReg[9] = 0x8c;
	gptrLLEReg[11] = 0x6c;
	gptrLLEReg[13] = 0x8c;
	gptrLLEReg[15] = 0x6c;
	gptrLLEReg[17] = 0x8c;
	gptrLLEReg[19] = 0x76;
	gptrLLEReg[1] = 0x78;
	gptrLLEReg[21] = 0;
	gptrLLEReg[31] = (uint32_t)LLE_BUF;
	gptrLLEReg[2] = 0xffffffff;
	gptrLLEReg[3] = 0x16000f;

	gptrRFENDReg[10] = 0x480;
	gptrRFENDReg[12] &= 0xfff9ffff;
	gptrRFENDReg[12] |= 0x70000000;
	gptrRFENDReg[15] = gptrRFENDReg[15] & 0xf8ffffff | 0x2000000;
	gptrRFENDReg[15] = gptrRFENDReg[15] & 0x1fffffff | 0x40000000;
	gptrRFENDReg[18] &= 0xfff8ffff;
	gptrRFENDReg[20] = gptrRFENDReg[20] & 0xfffff8ff | 0x300;
	gptrRFENDReg[23] |= 0x70000;
	gptrRFENDReg[23] |= 0x700000;

	gptrBBReg[14] = 0x2020c;
	gptrBBReg[15] = 0x50;

	NVIC->VTFIDR[3] = 0x14;

	gptrBBReg[13] = gptrBBReg[13] & 0x1ffffff | (TxPower | 0x40) << 0x19;
	gptrBBReg[0] &= 0xfffffcff;
}

void RFEND_TxTuneWait() {
	gptrLLEReg[25] = 8000;
	while((-1 < (int32_t)gptrRFENDReg[36] << 5) || (-1 < (int32_t)gptrRFENDReg[36] << 6)) {
		if(gptrLLEReg[25] == 0) {
			break;
		}
	}
}

void RFEND_TXTune() {
	gptrRFENDReg[1] &= 0xfffffeff;
	gptrRFENDReg[10] &= 0xffffefff;
	gptrRFENDReg[11] &= 0xffffffef;
	gptrRFENDReg[2] |= 0x20000;
	gptrRFENDReg[1] |= 0x10;

	// 2401 MHz
	gptrRFENDReg[1] &= 0xfffffffe;
	gptrRFENDReg[14] = gptrRFENDReg[14] & 0xfffe00ff | 0xbf00;
	gptrRFENDReg[1] |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2401 = (uint8_t)gptrRFENDReg[36] & 0x3f;
	uint8_t nGA2401 = (uint8_t)(gptrRFENDReg[37] >> 10) & 0x7f;

	// 2480 MHz
	gptrRFENDReg[1] &= 0xfffffffe;
	gptrRFENDReg[14] = gptrRFENDReg[14] & 0xfffe00ff | 0xe700;
	gptrRFENDReg[1] |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2480 = (uint8_t)gptrRFENDReg[36] & 0x3f;
	uint8_t nGA2480 = (uint8_t)(gptrRFENDReg[37] >> 10) & 0x7f;

	// 2440 MHz
	gptrRFENDReg[1] &= 0xfffffffe;
	gptrRFENDReg[14] = gptrRFENDReg[14] & 0xfffe00ff | 0xd300;
	gptrRFENDReg[1] |= 1;
	RFEND_TxTuneWait();
	uint8_t nCO2440 = (uint8_t)gptrRFENDReg[36] & 0x3f;
	uint8_t nGA2440 = (uint8_t)(gptrRFENDReg[37] >> 10) & 0x7f;

	uint32_t dCO0140 = nCO2401 - nCO2440;
	gptrRFENDReg[40] = gptrRFENDReg[40] & 0xfffffff0 | dCO0140 & 0xf;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x26) / 0x27 & 0xfU) << 4 | gptrRFENDReg[40] & 0xffffff0f;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x25) / 0x27 & 0xfU) << 8 | gptrRFENDReg[40] & 0xfffff0ff;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x24) / 0x27 & 0xfU) << 0xc | gptrRFENDReg[40] & 0xffff0fff;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x23) / 0x27 & 0xfU) << 0x10 | gptrRFENDReg[40] & 0xfff0ffff;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x22) / 0x27 & 0xfU) << 0x14 | gptrRFENDReg[40] & 0xff0fffff;
	gptrRFENDReg[40] = ((int)(dCO0140 * 0x21) / 0x27 & 0xfU) << 0x18 | gptrRFENDReg[40] & 0xf0ffffff;
	gptrRFENDReg[40] = gptrRFENDReg[40] & 0xfffffff | (int)(dCO0140 * 0x20) / 0x27 << 0x1c;

	gptrRFENDReg[41] = gptrRFENDReg[41] & 0xfffffff0 | (int)(dCO0140 * 0x1f) / 0x27 & 0xfU;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x1e) / 0x27 & 0xfU) << 4 | gptrRFENDReg[41] & 0xffffff0f;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x1d) / 0x27 & 0xfU) << 8 | gptrRFENDReg[41] & 0xfffff0ff;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x1c) / 0x27 & 0xfU) << 0xc | gptrRFENDReg[41] & 0xffff0fff;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x1b) / 0x27 & 0xfU) << 0x10 | gptrRFENDReg[41] & 0xfff0ffff;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x1a) / 0x27 & 0xfU) << 0x14 | gptrRFENDReg[41] & 0xff0fffff;
	gptrRFENDReg[41] = ((int)(dCO0140 * 0x19) / 0x27 & 0xfU) << 0x18 | gptrRFENDReg[41] & 0xf0ffffff;
	gptrRFENDReg[41] = gptrRFENDReg[41] & 0xfffffff | (int)(dCO0140 * 0x18) / 0x27 << 0x1c;

	gptrRFENDReg[42] = gptrRFENDReg[42] & 0xfffffff0 | (int)(dCO0140 * 0x17) / 0x27 & 0xfU;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x16) / 0x27 & 0xfU) << 4 | gptrRFENDReg[42] & 0xffffff0f;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x15) / 0x27 & 0xfU) << 8 | gptrRFENDReg[42] & 0xfffff0ff;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x14) / 0x27 & 0xfU) << 0xc | gptrRFENDReg[42] & 0xffff0fff;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x13) / 0x27 & 0xfU) << 0x10 | gptrRFENDReg[42] & 0xfff0ffff;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x12) / 0x27 & 0xfU) << 0x14 | gptrRFENDReg[42] & 0xff0fffff;
	gptrRFENDReg[42] = ((int)(dCO0140 * 0x11) / 0x27 & 0xfU) << 0x18 | gptrRFENDReg[42] & 0xf0ffffff;
	gptrRFENDReg[42] = gptrRFENDReg[42] & 0xfffffff | (int)(dCO0140 * 0x10) / 0x27 << 0x1c;

	gptrRFENDReg[43] = gptrRFENDReg[43] & 0xfffffff0 | (int)(dCO0140 * 0xf) / 0x27 & 0xfU;
	gptrRFENDReg[43] = ((int)(dCO0140 * 0xe) / 0x27 & 0xfU) << 4 | gptrRFENDReg[43] & 0xffffff0f;
	gptrRFENDReg[43] = gptrRFENDReg[43] & 0xfffff0ff | ((int)dCO0140 / 3 & 0xfU) << 8;
	gptrRFENDReg[43] = ((int)(dCO0140 * 0xc) / 0x27 & 0xfU) << 0xc | gptrRFENDReg[43] & 0xffff0fff;
	gptrRFENDReg[43] = ((int)(dCO0140 * 0xb) / 0x27 & 0xfU) << 0x10 | gptrRFENDReg[43] & 0xfff0ffff;
	gptrRFENDReg[43] = ((int)(dCO0140 * 10) / 0x27 & 0xfU) << 0x14 | gptrRFENDReg[43] & 0xff0fffff;
	gptrRFENDReg[43] = ((int)(dCO0140 * 9) / 0x27 & 0xfU) << 0x18 | gptrRFENDReg[43] & 0xf0ffffff;
	gptrRFENDReg[43] = gptrRFENDReg[43] & 0xfffffff | (int)(dCO0140 * 8) / 0x27 << 0x1c;

	gptrRFENDReg[44] = gptrRFENDReg[44] & 0xfffffff0 | (int)(dCO0140 * 7) / 0x27 & 0xfU;
	gptrRFENDReg[44] = ((int)(dCO0140 * 6) / 0x27 & 0xfU) << 4 | gptrRFENDReg[44] & 0xffffff0f;
	gptrRFENDReg[44] = ((int)(dCO0140 * 5) / 0x27 & 0xfU) << 8 | gptrRFENDReg[44] & 0xfffff0ff;
	gptrRFENDReg[44] = ((int)(dCO0140 * 4) / 0x27 & 0xfU) << 0xc | gptrRFENDReg[44] & 0xffff0fff;
	gptrRFENDReg[44] = gptrRFENDReg[44] & 0xfff0ffff | ((int)dCO0140 / 0xd & 0xfU) << 0x10;
	gptrRFENDReg[44] = ((int)(dCO0140 * 2) / 0x27 & 0xfU) << 0x14 | gptrRFENDReg[44] & 0xff0fffff;
	gptrRFENDReg[44] = gptrRFENDReg[44] & 0xf0ffffff | ((int)dCO0140 / 0x27 & 0xfU) << 0x18;
	gptrRFENDReg[44] = gptrRFENDReg[44] & 0xfffffff;

	uint32_t dCO4080 = nCO2440 - nCO2480;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xfffffff0 | dCO4080 / 0x28 & 0xfU;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xffffff0f | (dCO4080 / 0x14 & 0xfU) << 4;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xfffff0ff | ((dCO4080 * 3) / 0x28 & 0xfU) << 8;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xffff0fff | (dCO4080 / 10 & 0xfU) << 0xc;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xfff0ffff | (dCO4080 / 8 & 0xfU) << 0x10;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xff0fffff | ((dCO4080 * 6) / 0x28 & 0xfU) << 0x14;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xf0ffffff | ((dCO4080 * 7) / 0x28 & 0xfU) << 0x18;
	gptrRFENDReg[45] = gptrRFENDReg[45] & 0xfffffff | dCO4080 / 5 << 0x1c;

	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xfffffff0 | (dCO4080 * 9) / 0x28 & 0xfU;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xffffff0f | (dCO4080 / 4 & 0xfU) << 4;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xfffff0ff | ((dCO4080 * 0xb) / 0x28 & 0xfU) << 8;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xffff0fff | ((dCO4080 * 0xc) / 0x28 & 0xfU) << 0xc;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xfff0ffff | ((dCO4080 * 0xd) / 0x28 & 0xfU) << 0x10;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xff0fffff | ((dCO4080 * 0xe) / 0x28 & 0xfU) << 0x14;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xf0ffffff | ((dCO4080 * 0xf) / 0x28 & 0xfU) << 0x18;
	gptrRFENDReg[46] = gptrRFENDReg[46] & 0xfffffff | (dCO4080 * 0x10) / 0x28 << 0x1c;

	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xfffffff0 | (dCO4080 * 0x11) / 0x28 & 0xfU;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xffffff0f | ((dCO4080 * 0x12) / 0x28 & 0xfU) << 4;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xfffff0ff | ((dCO4080 * 0x13) / 0x28 & 0xfU) << 8;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xffff0fff | (dCO4080 / 2 & 0xfU) << 0xc;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xfff0ffff | ((dCO4080 * 0x15) / 0x28 & 0xfU) << 0x10;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xff0fffff | ((dCO4080 * 0x16) / 0x28 & 0xfU) << 0x14;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xf0ffffff | ((dCO4080 * 0x17) / 0x28 & 0xfU) << 0x18;
	gptrRFENDReg[47] = gptrRFENDReg[47] & 0xfffffff | (dCO4080 * 0x18) / 0x28 << 0x1c;

	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xfffffff0 | (dCO4080 * 0x19) / 0x28 & 0xfU;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xffffff0f | ((dCO4080 * 0x1a) / 0x28 & 0xfU) << 4;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xfffff0ff | ((dCO4080 * 0x1b) / 0x28 & 0xfU) << 8;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xffff0fff | ((dCO4080 * 0x1c) / 0x28 & 0xfU) << 0xc;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xfff0ffff | ((dCO4080 * 0x1d) / 0x28 & 0xfU) << 0x10;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xff0fffff | ((dCO4080 * 0x1e) / 0x28 & 0xfU) << 0x14;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xf0ffffff | ((dCO4080 * 0x1f) / 0x28 & 0xfU) << 0x18;
	gptrRFENDReg[48] = gptrRFENDReg[48] & 0xfffffff | (dCO4080 * 0x20) / 0x28 << 0x1c;

	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xfffffff0 | (dCO4080 * 0x21) / 0x28 & 0xfU;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xffffff0f | ((dCO4080 * 0x22) / 0x28 & 0xfU) << 4;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xfffff0ff | ((dCO4080 * 0x23) / 0x28 & 0xfU) << 8;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xffff0fff | ((dCO4080 * 0x24) / 0x28 & 0xfU) << 0xc;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xfff0ffff | ((dCO4080 * 0x25) / 0x28 & 0xfU) << 0x10;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xff0fffff | ((dCO4080 * 0x26) / 0x28 & 0xfU) << 0x14;
	gptrRFENDReg[49] = ((dCO4080 * 0x27) / 0x28 & 0xfU) << 0x18 | gptrRFENDReg[49] & 0xf0ffffff;
	gptrRFENDReg[49] = gptrRFENDReg[49] & 0xfffffff | dCO4080 * 0x10000000;

	gptrRFENDReg[50] = (dCO4080 * 0x29) / 0x28 & 0xfU | gptrRFENDReg[50] & 0xfffffff0;
	gptrRFENDReg[50] = ((dCO4080 * 0x2a) / 0x28 & 0xfU) << 4 | gptrRFENDReg[50] & 0xffffff0f;

	uint32_t dGA4080 = nGA2440 - nGA2480;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xfffff0ff | ((dGA4080 * 10) / dCO4080 & 0xfU | 8) << 8;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xffff0fff | ((dGA4080 * 9) / dCO4080 & 0xfU | 8) << 0xc;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xfff0ffff | ((dGA4080 * 8) / dCO4080 & 0xfU | 8) << 0x10;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xff0fffff | ((dGA4080 * 7) / dCO4080 & 0xfU | 8) << 0x14;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xf0ffffff | ((dGA4080 * 6) / dCO4080 & 0xfU | 8) << 0x18;
	gptrRFENDReg[50] = gptrRFENDReg[50] & 0xfffffff | ((dGA4080 * 5) / dCO4080 | 8U) << 0x1c;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xfffffff0 | (dGA4080 * 4) / dCO4080 & 0xfU | 8;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xffffff0f | ((dGA4080 * 3) / dCO4080 & 0xfU | 8) << 4;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xfffff0ff | ((dGA4080 * 2) / dCO4080 & 0xfU | 8) << 8;
	gptrRFENDReg[51] = (dGA4080 / dCO4080 & 0xfU | 8) << 0xc | gptrRFENDReg[51] & 0xffff0fff;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xfff0ffff;

	uint32_t dGA0140 = nGA2401 - nGA2440;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xff0fffff | (dGA0140 / (int)dCO0140 & 0xfU) << 0x14;
	gptrRFENDReg[51] = ((dGA0140 * 2) / (int)dCO0140 & 0xfU) << 0x18 | gptrRFENDReg[51] & 0xf0ffffff;
	gptrRFENDReg[51] = gptrRFENDReg[51] & 0xfffffff | (dGA0140 * 3) / (int)dCO0140 << 0x1c;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xfffffff0 | (dGA0140 * 4) / (int)dCO0140 & 0xfU;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xffffff0f | ((dGA0140 * 5) / (int)dCO0140 & 0xfU) << 4;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xfffff0ff | ((dGA0140 * 6) / (int)dCO0140 & 0xfU) << 8;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xffff0fff | ((dGA0140 * 7) / (int)dCO0140 & 0xfU) << 0xc;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xfff0ffff | ((dGA0140 * 8) / (int)dCO0140 & 0xfU) << 0x10;
	gptrRFENDReg[52] = gptrRFENDReg[52] & 0xff0fffff | ((dGA0140 * 9) / (int)dCO0140 & 0xfU) << 0x14;
	gptrRFENDReg[52] = ((dGA0140 * 10) / (int)dCO0140 & 0xfU) << 0x18 | gptrRFENDReg[52] & 0xf0ffffff;

	gptrRFENDReg[1] = gptrRFENDReg[1] & 0xffffffef;
	gptrRFENDReg[1] = gptrRFENDReg[1] & 0xfffffffe;
	gptrRFENDReg[10] = gptrRFENDReg[10] | 0x1000;
	gptrRFENDReg[11] = gptrRFENDReg[11] | 0x10;
	gptrRFENDReg[14] = gptrRFENDReg[14] & 0xffffffc0 | nCO2440 & 0x3f;
	gptrRFENDReg[14] = ((nGA2440 & 0x7f) << 0x18) | (gptrRFENDReg[14] & 0x80ffffff);

	// FTune
	gptrRFENDReg[1] |= 0x100;
}

void RegInit() {
	gptrBBReg[0] &= 0xfffeffff;
	gptrRFENDReg[2] |= 0x330000;
	gptrLLEReg[20] = 0x30558;
	RFEND_TXTune();
	gptrBBReg[0] &= 0xfffdffff;
	gptrRFENDReg[2] &= 0xffcdffff;
	gptrLLEReg[20] = 0x30000;
}

void BLECoreInit(uint8_t TxPower) {
	gptrBBReg = (uint32_t *)0x4000c100;
	gptrLLEReg = (uint32_t *)0x4000c200;
	gptrRFENDReg = (uint32_t *)0x4000d000;
	DevInit(TxPower);
	RegInit();
	NVIC->IPRIOR[0x15] |= 0x80;
	NVIC->IENR[0] = 0x200000;
}

__attribute__((section(".highcode")))
void DevSetChannel(uint8_t channel) {
	gptrRFENDReg[11] &= 0xfffffffd;
	gptrBBReg[0] = (gptrBBReg[0] & 0xffffff80) | (channel & 0x7f);
}

__attribute__((section(".highcode")))
void PHYSetTxMode(size_t len) {
	gptrBBReg[0] = gptrBBReg[0] & 0xfffffcff | 0x100;
	gptrBBReg[9] = gptrBBReg[9] & 0xf9ffffff | 0x2000000;
	gptrLLEReg[4] &= 0xfffdffff;
	gptrLLEReg[2] = 0x20000;
	gptrLLEReg[25] = (uint32_t)(((len *8) + 0xee) *2);
}

__attribute__((section(".highcode")))
void RF_Stop() {
	gptrBBReg[0] &= 0xfffdffff;
	gptrRFENDReg[2] &= 0xffcdffff;
	gptrLLEReg[20] &= 0x30000;

	gptrLLEReg[20] &= 0xfffff8ff;
	gptrLLEReg[0] |= 0x08;
}

__attribute__((section(".highcode")))
void Advertise(uint8_t adv[], size_t len, uint8_t channel) {
	gptrBBReg[13] = gptrBBReg[13] & 0xfffffffc | 1;

	DevSetChannel(channel);

	gptrBBReg[0] &= 0xfffeffff;
	gptrRFENDReg[2] |= 0x330000;
	gptrLLEReg[20] = 0x30258;

	gptrBBReg[3] = 0x8E89BED6; // access address
	gptrBBReg[24] = 0x8E89BED6;
	gptrBBReg[1] = 0x555555; // crc init
	gptrBBReg[22] = 0x555555;
	gptrBBReg[2] =  gptrBBReg[2] & 0xff000000 | 0x80032d; // crc poly
	gptrBBReg[23] =  gptrBBReg[23] & 0xff000000 | 0x80032d;
	gptrLLEReg[1] = gptrLLEReg[1] & 0xfffffffe | 1;

	ADV_BUF[0] = 0x02; //TxPktType 0x00, 0x02, 0x06 seem to work, with only 0x02 showing up on the phone
	ADV_BUF[1] = len ;
	memcpy(&ADV_BUF[2], adv, len);
	gptrLLEReg[30] = (uint32_t)ADV_BUF;

	PHYSetTxMode(len);

	gptrBBReg[0] |= 0x1000000;
	gptrBBReg[13] &= 0xfffffffc;
	gptrLLEReg[0] = 0x02;

	while(gptrLLEReg[25]); // wait for tx buffer to empty
	RF_Stop();
}
