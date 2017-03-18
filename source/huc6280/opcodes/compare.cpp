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

static __inline void op_bit()
{
    memread(ea);
    flag_v = (tmpreg >> 6) & 1;
    flag_n = (tmpreg >> 7) & 1;
    flag_z = (a & tmpreg) ? 0 : 1;
}

static void op_cmp()
{
    int tmpi = a - memread(ea);
    flag_c = (tmpi >= 0) ? 1 : 0;
    checknz((uint8_t)tmpi);
}

static void op_cpx()
{
    int tmpi = x - memread(ea);
    flag_c = (tmpi >= 0) ? 1 : 0;
    checknz((uint8_t)tmpi);
}

static void op_cpy()
{
    int tmpi = y - memread(ea);
    flag_c = (tmpi >= 0) ? 1 : 0;
    checknz((uint8_t)tmpi);
}

static void op_tst(uint8_t imm)
{
	memread(ea);
	flag_t = 0;
	flag_v = (tmpreg & 0x40) ? 1 : 0;
	flag_n = (tmpreg & 0x80) ? 1 : 0;
	tmpreg &= imm;
	flag_z = (tmpreg == 0) ? 1 : 0;
}

OPDECL(tsb) {
	memread(ea);
	flag_t = 0;
	flag_v = (tmpreg & 0x40) ? 1 : 0;
	flag_n = (tmpreg & 0x80) ? 1 : 0;
	tmpreg |= a;
	memwrite(ea, tmpreg);
	flag_z = (tmpreg == 0) ? 1 : 0;
}

OPDECL(trb) {
	memread(ea);
	flag_t = 0;
	flag_v = (tmpreg & 0x40) ? 1 : 0;
	flag_n = (tmpreg & 0x80) ? 1 : 0;
	tmpreg &= ~a;
	memwrite(ea, tmpreg);
	flag_z = (tmpreg == 0) ? 1 : 0;
}
