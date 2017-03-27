
/************************************

PicchioEngine

Copyright(c)2008 Emanuele Bettidi

************************************/

/* CPU.cpp */

/* HuC6280 - CPU (Central Processing Unit) */

// #include <fstream>
// #include <ios>
// #include <iostream>

/*
#include "Types.h"
#include "Timer.h"
#include "IntCtrl.h"
#include "IOPort.h"
#include "PCE.h"
#include "PSG.h"
#include "VDC.h"
#include "CPU.h"
*/
#include "../pce.h"

extern CPce *pce;

#include "huc6280.h"

#include <stdio.h>

namespace VDC {
	void resync() {

	}

}

namespace Timer {
	void resync() {

	}

}

const uint8 BT_TIA = 1;
const uint8 BT_TAI = 2;
const uint8 BT_TDD = 3;
const uint8 BT_TII = 4;
const uint8 BT_TIN = 5;

CPicHuc6280::CPicHuc6280(CPce *p):CPceDevice(p, (char*)"PiccoloEngine Huc6280")
{

}

CPicHuc6280::~CPicHuc6280()
{

}

/* initialization */
void CPicHuc6280::Reset()
{
	/* typical values at the power-up */
	A = 0xFF;
	X = 0xFF;
	Y = 0xFF;
	S = 0xFF;
	P = 0x97;  // flags T, D and I impossible to test
	PC = 0x0000;  // impossible to test
	MPR[0] = 0x0E;
	MPR[1] = 0x04;
	MPR[2] = 0xAC;
	MPR[3] = 0x04;
	MPR[4] = 0x84;
	MPR[5] = 0x06;
	MPR[6] = 0x04;
	MPRs_buffer = 0x55;
	read_buffer = 0x00;  // impossible to test
	IO_buffer = 0x00;
	old_P = 0x97;
	/* ... */
	clock = 0;
	//		Timer::init();
	//		IntCtrl::init();
	//		IOPort::init();
	block_transfer = 0;

	cycle_length = 4;
	cycles = 0;
	MPR[7] = 0x00;
	IRQ_lines = 0;
	old_irq_mask = 0;
	start();
}

void CPicHuc6280::Tick(int clocks)
{

}

int CPicHuc6280::Execute(int clocks)
{
	clock += clocks;
	cycles += clocks * cycle_length;

	if (block_transfer != 0)
	{
		switch (block_transfer)
		{
		case BT_TIA: goto continue_TIA;
		case BT_TAI: goto continue_TAI;
		case BT_TDD: goto continue_TDD;
		case BT_TII: goto continue_TII;
		case BT_TIN: goto continue_TIN;
		}
	}

	while (clock > 0)
	{
		if (IRQ_lines != 0)
		{
			if ((old_P & I_FLAG) == 0)
			{
				uint16 vector = 0;
				if (((IRQ_lines & IRQ2_LINE) & (old_irq_mask ^ 0x07)) != 0) vector = IRQ2_VECTOR;
				if (((IRQ_lines & IRQ1_LINE) & (old_irq_mask ^ 0x07)) != 0) vector = IRQ1_VECTOR;
				if (((IRQ_lines & TIRQ_LINE) & (old_irq_mask ^ 0x07)) != 0) vector = TIRQ_VECTOR;
				if (vector != 0)
				{
					INTERRUPT(vector);
					old_irq_mask = irq_mask;
					if (clock <= 0) return(0);
				}
			}
		}
		old_P = P;
		old_irq_mask = irq_mask;

//		printf("PC:%04X A:%02X X:%02X Y:%02X S:%02X P:%02X C:%d MPR:%02d %02d %02d %02d %02d %02d %02d %02d\n", PC, A, X, Y, S, P, clock, MPR[0], MPR[1], MPR[2], MPR[3], MPR[4], MPR[5], MPR[6], MPR[7]);

//		Disasm(PC);

		FETCH_OPCODE();
//		printf("opcode = %02X\n", read_buffer);
		switch (read_buffer)  // meglio '(opcode)'?
		{
			#include "Ops.h"
		}
	//	Timer::resync();
	//	VDC::resync();
	}
	return(0);
}

void CPicHuc6280::Step()
{
	do {
		clock = 0;
	} while (Execute(1) == 1);
}

void CPicHuc6280::SetIrq(uint32_t line)
{
	if (line & HUC6280_IRQ2) IRQ_lines |= IRQ2_LINE;
	if (line & HUC6280_IRQ1) IRQ_lines |= IRQ1_LINE;
	if (line & HUC6280_TIRQ) IRQ_lines |= TIRQ_LINE;
}

void CPicHuc6280::ClearIrq(uint32_t line)
{
	if (line & HUC6280_IRQ2) IRQ_lines &= ~IRQ2_LINE;
	if (line & HUC6280_IRQ1) IRQ_lines &= ~IRQ1_LINE;
	if (line & HUC6280_TIRQ) IRQ_lines &= ~TIRQ_LINE;
}


void CPicHuc6280::SetUserData(void *user)
{
	userdata = user;
}

void CPicHuc6280::SetReadFunc(int page, readfunc_t rd)
{
	if (rd == 0)
		rd = ReadMemPage;
	read_mem_funcs[page & 0xFF] = rd;
}

void CPicHuc6280::SetWriteFunc(int page, writefunc_t wr)
{
	if (wr == 0)
		wr = WriteMemPage;
	write_mem_funcs[page & 0xFF] = wr;
}

void CPicHuc6280::SetMem(int page, uint8_t *p, int flags)
{
	if(flags & MEM_READ)
		read_mem_pages[page & 0xFF] = p;
	if(flags & MEM_WRITE)
		write_mem_pages[page & 0xFF] = p;
}

#define READ_MEM(addr) \
	(read_mem_funcs[MPR[((addr) >> 13) & 7]])(userdata, (addr) & 0x1FFF)
#define READ_MEM16(a)   (READ_MEM(a) | (READ_MEM((a) + 1) << 8))

uint32_t CPicHuc6280::Disasm(uint32_t dpc, char *outbuf, bool show_regs)
{
	uint32_t start = dpc;
	uint8_t opcode = READ_MEM(dpc);
	opcode_t *o = findop(opcode);
	char opbytes[64];
	char opdis[64];
	char buf[64];
	int oplen = 0;
	int i, offs;
	int maxlen = 7;
	static int counter = -1;

	//    printf("disasm from %04X (opcode = %02X)\n",dpc, opcode);
	if (o == 0) {
		printf("not found in oplist(opcode = %02X)\n", opcode);
		return ((uint32_t)-1);
	}

	memset(opbytes, ' ', 32);

	sprintf(opbytes, "%04X:     %02X %02X %02X %02X %02X %02X %02X %02X", dpc, READ_MEM(dpc), READ_MEM(dpc + 1), READ_MEM(dpc + 2), READ_MEM(dpc + 3), READ_MEM(dpc + 4), READ_MEM(dpc + 5), READ_MEM(dpc + 6), READ_MEM(dpc + 7));

	switch (o->am) {
	case a_imp:
		oplen = 1;
		sprintf(opdis, "%s  ", o->name);
		break;

	case a_acc:
		oplen = 1;
		sprintf(opdis, "%s  a", o->name);
		break;

	case a_rel:
		oplen = 2;
		offs = (int32_t)(dpc + 2);
		offs += (int16_t)(int8_t)READ_MEM(dpc + 1);
		sprintf(opdis, "%s  $%04X", o->name, offs);
		break;

	case a_imm:
		oplen = 2;
		sprintf(opdis, "%s  #$%02X", o->name, READ_MEM(dpc + 1));
		break;

	case a_zpg:
		oplen = 2;
		sprintf(opdis, "%s  $%02X", o->name, READ_MEM(dpc + 1));
		break;

	case a_zpx:
		oplen = 2;
		sprintf(opdis, "%s  $%02X, x", o->name, READ_MEM(dpc + 1));
		break;

	case a_zpy:
		oplen = 2;
		sprintf(opdis, "%s  $%02X, y", o->name, READ_MEM(dpc + 1));
		break;

	case a_abs:
		oplen = 3;
		sprintf(opdis, "%s  $%04X", o->name, READ_MEM16(dpc + 1));
		break;

	case a_tab:
		oplen = 4;
		sprintf(opdis, "%s  #$%02X, $%04X", o->name, READ_MEM(dpc + 1), READ_MEM16(dpc + 2));
		break;

	case a_abx:
		oplen = 3;
		sprintf(opdis, "%s  $%04X, x", o->name, READ_MEM16(dpc + 1));
		break;

	case a_aby:
		oplen = 3;
		sprintf(opdis, "%s  $%04X, y", o->name, READ_MEM16(dpc + 1));
		break;

	case a_ind:
		oplen = 2;
		sprintf(opdis, "%s  ($%02X)", o->name, READ_MEM(dpc + 1));
		break;

	case a_inx:
		oplen = 2;
		sprintf(opdis, "%s  ($%02X), x", o->name, READ_MEM(dpc + 1));
		break;

	case a_iny:
		oplen = 2;
		sprintf(opdis, "%s  ($%02X, y)", o->name, READ_MEM(dpc + 1));
		break;

	case a_bbr:
		oplen = 3;
		offs = (int32_t)(dpc + 2);
		offs += (int16_t)(int8_t)READ_MEM(dpc + 2);
		sprintf(opdis, "%s%d #$%02X, $%04X", o->name, o->op & 7, READ_MEM(dpc + 1), offs);
		break;

	case a_tra:
		oplen = 7;
		sprintf(opdis, "%s  $%06X, $%06X, $%04X", o->name, xlat(READ_MEM16(dpc + 1)), xlat(READ_MEM16(dpc + 3)), READ_MEM16(dpc + 5));
		break;

	case a_zpr:
		oplen = 3;
		offs = (int32_t)(dpc + 3);
		offs += (int16_t)(int8_t)READ_MEM(dpc + 2);
		sprintf(opdis, "%s  ($%02X), $%04X", o->name, READ_MEM16(dpc + 1), offs);
		break;

	default:
		oplen = 1;
		sprintf(opdis, "%s", o->name);
		break;
	}

	dpc += oplen;

	offs = oplen * 3 + 10;
	for (i = offs; i < (10 + maxlen * 3); i++) {
		opbytes[i] = ' ';
	}
	opbytes[(10 + maxlen * 3)] = 0;

	for (i = strlen(opdis); i < 32; i++) {
		opdis[i] = ' ';
	}
	opdis[i] = 0;

/*	printf(opbytes);
	printf(opdis);

	//print registers too (currently debugging the core...)
	printf("A:%02X X:%02X Y:%02X S:%02X PC=%04X P:%s", A, X, Y, S, PC, get_flags(buf));

	printf("\n");*/

	if (show_regs) {
		sprintf(outbuf, "%s%s""A:%02X X:%02X Y:%02X S:%02X PC=%04X P:%s", opbytes, opdis, A, X, Y, S, PC, get_flags(buf));
	}

	else {
		sprintf(outbuf, "%s%s", opbytes, opdis);
	}

	return(dpc);
}

