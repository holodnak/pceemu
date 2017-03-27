/***************************************************************************
 *   Copyright (C) 2017 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


/*
 * TODO: validate cycle times for ST0, ST1, and ST2 (and all other opcodes...)
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "huc6280.h"

#define OPDECL(nn)  static __inline void op_##nn()

#define ZEROPAGE    0x2000
#define STACKPAGE   0x2100

#define IRQ_RESET   0xFFFE
#define IRQ_NMI     0xFFFC
#define IRQ_TIRQ    0xFFFA
#define IRQ_IRQ1    0xFFF8
#define IRQ_IRQ2    0xFFF6

#define READ_MEM(a)     memread_real((mpr[(a) >> 13] << 13) | ((a) & 0x1FFF))
#define READ_MEM16(a)   (READ_MEM(a) | (READ_MEM((a) + 1) << 8))

static uint32_t clocklen;
static uint32_t cycles, clocks;

static uint16_t pc, ea;
static uint8_t a, x, y, s, p;
static uint8_t mpr[8], mpr_buf;
static uint16_t src, dst, len;

static uint8_t flag_n, flag_z, flag_v, flag_c;
static uint8_t flag_i, flag_b, flag_t, flag_d;

static uint8_t tmpreg;
static uint8_t opcode;
static uint8_t irqstate, prev_irqstate;

static readfunc_t read_mem_funcs[256];
static writefunc_t write_mem_funcs[256];

static uint8_t *read_mem_pages[256];
static uint8_t *write_mem_pages[256];

static void *userdata;

readfunc_t read_mem;
writefunc_t write_mem;

uint32_t huc6280_disasm(uint32_t dpc);

#include "helper.cpp"
#include "addrmodes.cpp"

#include "opcodes/alu.cpp"
#include "opcodes/branch.cpp"
#include "opcodes/compare.cpp"
#include "opcodes/flag.cpp"
#include "opcodes/incdec.cpp"
#include "opcodes/loadstore.cpp"
#include "opcodes/misc.cpp"
#include "opcodes/stack.cpp"
#include "opcodes/transfer.cpp"

#include "execute.cpp"
#include "disasm.cpp"

static uint8_t null_read_func(void *user, uint32_t addr)
{
	printf("null read: page %X, addr: %06X\n", addr >> 13, addr);
	return(0xFF);
}

static void null_write_func(void *user, uint32_t addr, uint8_t data)
{
	printf("null write: page %X, addr: %06X = %02X\n", addr >> 13, addr, data);
}

void huc6280_init()
{
	int i;

	userdata = 0;
	for (i = 0; i < 256; i++) {
		read_mem_funcs[i] = null_read_func;
		read_mem_pages[i] = 0;
		write_mem_funcs[i] = null_write_func;
		write_mem_pages[i] = 0;
	}
}

void huc6280_reset()
{
    cycles = 0;
    clocklen = 3;

	mpr[7] = 0;
	irqstate = 0;

	s = 0xFF;
	a = 0xFF;
	x = 0xFF;
	y = 0xFF;
	p = 0x97;
	expand_flags();

	mpr[0] = 0x0E;
	mpr[1] = 0x04;
	mpr[2] = 0xAC;
	mpr[3] = 0x04;
	mpr[4] = 0x84;
	mpr[5] = 0x06;
	mpr[6] = 0x04;
	mpr[7] = 0x00;

	execute_irq(IRQ_RESET);


	printf("vectors:\n");
	printf("   reset:  %04X\n", pc);
	printf("   tirq:   %04X\n", READ_MEM16(IRQ_TIRQ));
	printf("   irq1:   %04X\n", READ_MEM16(IRQ_IRQ1));
	printf("   irq2:   %04X\n", READ_MEM16(IRQ_IRQ2));
	printf("   nmi:    %04X\n", READ_MEM16(IRQ_NMI));

	memset(op_count, 0, 256 * sizeof(int));
}

void huc6280_set_irq(int line)
{
	irqstate |= line;
}

void huc6280_clear_irq(int line)
{
	irqstate &= ~line;
}

void huc6280_setuser(void *user) { userdata = user; }

void huc6280_setreadfunc(int page, readfunc_t rd) { read_mem_funcs[page & 0xFF] = rd; }
void huc6280_setwritefunc(int page, writefunc_t wr) { write_mem_funcs[page & 0xFF] = wr; }
void huc6280_setreadmem(int page, uint8_t *p) { read_mem_pages[page & 0xFF] = p; }
void huc6280_setwritemem(int page, uint8_t *p) { write_mem_pages[page & 0xFF] = p; }
void huc6280_setmem(int page, uint8_t *p) { read_mem_pages[page & 0xFF] = write_mem_pages[page & 0xFF] = p; }

uint32_t huc6280_getpc() {
	return(pc);
}
