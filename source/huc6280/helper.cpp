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

#include "../pce.h"
extern CPce *pce;

static void clocktick()
{
	//acknowledge interrupts
	prev_irqstate = irqstate & ((flag_i == 0) ? 0xF : HUC6280_NMI);

	//increment counters
	cycles += clocklen;
    clocks++;

	//tick the rest of the system
	//pce_tick(clocklen);
	pce->Tick(clocklen);
}

static uint8_t memread_real(uint32_t addr)
{
	uint8_t seg;

	seg = (addr >> 13) & 0xFF;
	if (read_mem_pages[seg])
		tmpreg = read_mem_pages[seg][addr & 0x1FFF];
	else
		tmpreg = read_mem_funcs[seg](userdata, addr);
	return(tmpreg);
}

static void memwrite_real(uint32_t addr, uint8_t data)
{
	uint8_t seg;

	seg = (addr >> 13) & 0xFF;
	if (write_mem_pages[seg])
		write_mem_pages[seg][addr & 0x1FFF] = data;
	else
		write_mem_funcs[seg](userdata, addr, data);
}

static uint8_t memread(uint32_t addr)
{
	uint32_t realaddr;

	clocktick();
	realaddr = (mpr[(addr >> 13) & 7] << 13) | (addr & 0x1FFF);
//	printf("reading from: %04X (%06X) (mpr[%d] = %02X)\n", addr, realaddr, (addr >> 13) & 7, mpr[(addr >> 13) & 7]);
	return(memread_real(realaddr));
}

uint8_t huc6280_read(uint32_t addr)
{
	return(memread_real((mpr[(addr >> 13) & 7] << 13) | (addr & 0x1FFF)));
}

void huc6280_write(uint32_t addr, uint8_t data)
{
	memwrite_real((mpr[(addr >> 13) & 7] << 13) | (addr & 0x1FFF), data);
}

static void memwrite(uint32_t addr, uint8_t data)
{
	uint32_t realaddr;

	realaddr = (mpr[(addr >> 13) & 7] << 13) | (addr & 0x1FFF);
	clocktick();
	//	printf("writing to: %06X = %02X\n", addr, data);

	if (addr == 0x2228 || addr == 0x222D || addr == 0x2232) {
		static FILE *fp = 0;

		if (fp == 0) fp = fopen("ctrllog.txt", "wt");
		fprintf(fp, "write to %04X (%06X) = %02X\n", addr, realaddr, data);
		fflush(fp);
	}

	memwrite_real(realaddr, data);
}

static void expand_flags()
{
    flag_c = (p & 0x01) >> 0;
    flag_z = (p & 0x02) >> 1;
    flag_i = (p & 0x04) >> 2;
    flag_d = (p & 0x08) >> 3;
    flag_b = (p & 0x10) >> 4;
    flag_t = (p & 0x20) >> 5;
    flag_v = (p & 0x40) >> 6;
    flag_n = (p & 0x80) >> 7;
}

static void compact_flags()
{
    p = (flag_c) << 0;
    p |= (flag_z) << 1;
    p |= (flag_i) << 2;
    p |= (flag_d) << 3;
    p |= (flag_b) << 4;
    p |= (flag_t) << 5;
    p |= (flag_v) << 6;
    p |= (flag_n) << 7;
}

//check value for n/z and set flags
static void checknz(uint8_t n)
{
    flag_t = 0;
    flag_n = (n >> 7) & 1;
    flag_z = (n == 0) ? 1 : 0;
}

//push data to stack
static void push(uint8_t data)
{
    uint32_t addr = s-- | STACKPAGE;

//    printf("push: %04X = %02X\n", addr, data);
    memwrite(addr, data);
}

//pop data from stack
static uint8_t pop()
{
    uint32_t addr = ++s | STACKPAGE;
    uint8_t data = memread(addr);

//    printf("pop: %04X = %02X\n", addr, data);
    return(data);
}

static void execute_irq(uint32_t irqvec)
{
    memread(pc);
    memread(pc);
    push((uint8_t)(pc >> 8));
    push((uint8_t)pc);
    compact_flags();
    push(p);
	flag_d = 0;
	flag_t = 0;
    flag_i = 1;
    pc = memread(irqvec);
    pc |= memread(irqvec + 1) << 8;
    clocktick();
}
