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

static void op_lsr()
{
    memread(ea);
    clocktick();
    tmpreg >>= 1;
    memwrite(ea,tmpreg);
	flag_c = 0;
	checknz(tmpreg);
}

static void op_and() {
    uint8_t t;

    memread(ea);
    if(flag_t) {
        ea = ZEROPAGE + x;
        t = memread(ea);
        clocktick();
        t &= tmpreg;
        checknz(t);
        memwrite(ea, t);
    }
    else {
        a &= tmpreg;
        checknz(a);
    }
}

static void op_ora() {
    uint8_t t;

    memread(ea);
    if(flag_t) {
        ea = ZEROPAGE + x;
        t = memread(ea);
        clocktick();
        t |= tmpreg;
        checknz(t);
        memwrite(ea, t);
    }
    else {
        a |= tmpreg;
        checknz(a);
    }
}

static void op_eor() {
    uint8_t t;

    memread(ea);
    if(flag_t) {
        ea = ZEROPAGE + x;
        t = memread(ea);
        clocktick();
        t ^= tmpreg;
        checknz(t);
        memwrite(ea, t);
    }
    else {
        a ^= tmpreg;
        checknz(a);
    }
}

static void op_asl()
{
    memread(ea);
    clocktick();
    flag_c = (tmpreg >> 7) & 1;
    tmpreg <<= 1;
    memwrite(ea, tmpreg);
    checknz(tmpreg);
}

static void op_adc()
{
    int tmpi;

    memread(ea);
    tmpi = a + tmpreg + flag_c;
    flag_c = (tmpi & 0xFF00) ? 1 : 0;
    flag_v = (((a ^ tmpi) & (tmpreg ^ tmpi)) & 0x80) ? 1 : 0;
    a = (uint8_t)tmpi;

	if(flag_d) {
        flag_c = 0;
        if((a & 0x0f) > 0x09)
            a += 0x06;
        if((a & 0xf0) > 0x90) {
            a += 0x60;
            flag_c = 1;
        }
    }

	checknz(a);
}

static void op_sbc()
{
    int tmpi;

    memread(ea);
    tmpi = a - tmpreg - (1 - flag_c);
    flag_c = ((tmpi & 0xFF00) == 0) ? 1 : 0;
    flag_v = (((a ^ tmpreg) & (a ^ tmpi)) & 0x80) ? 1 : 0;
    a = (uint8_t)tmpi;

	if(flag_d) {
        a -= 0x66;
        flag_c = 0;
        if((a & 0x0f) > 0x09)
            a += 0x06;
        if((a & 0xf0) > 0x90) {
            a += 0x60;
            flag_c = 1;
        }
    }

	checknz(a);
}

static void op_ror()
{
    uint8_t tmp8;

	memread(ea);
    clocktick();
    tmp8 = flag_c;
    flag_c = tmpreg & 1;
    tmpreg = (tmpreg >> 1) | (tmp8 << 7);
    memwrite(ea, tmpreg);
    checknz(tmpreg);
}

static void op_rol()
{
    uint8_t tmp8;

    memread(ea);
    clocktick();
    tmp8 = flag_c;
    flag_c = (tmpreg >> 7) & 1;
    tmpreg = (tmpreg << 1) | tmp8;
    memwrite(ea, tmpreg);
    checknz(tmpreg);
}

static void op_ror_acc()
{
    memread(pc);
    uint8_t tmp8 = flag_c;
    flag_c = a & 1;
    a = (a >> 1) | (tmp8 << 7);
    checknz(a);
}

static void op_rol_acc()
{
    memread(pc);
    uint8_t tmp8 = flag_c;
    flag_c = (a >> 7) & 1;
    a = (a << 1) | tmp8;
    checknz(a);
}

static void op_asl_acc()
{
	memread(pc);
	flag_c = (a >> 7) & 1;
	a <<= 1;
	checknz(a);
}

static void op_inc_acc()
{
	memread(pc);
	a++;
	checknz(a);
}

static void op_dec_acc()
{
	memread(pc);
	a--;
	checknz(a);
}

static void op_lsr_acc()
{
	memread(pc);
	a >>= 1;
	flag_c = 0;
	checknz(a);
}
