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

static void op_lda()    {   memread(ea);    a = tmpreg; checknz(a); }
static void op_ldx()    {   memread(ea);    x = tmpreg; checknz(x); }
static void op_ldy()    {   memread(ea);    y = tmpreg; checknz(y); }

static void op_sta()    {   flag_t = 0;     memwrite(ea, a);    }
static void op_stx()    {   flag_t = 0;     memwrite(ea, x);    }
static void op_sty()    {   flag_t = 0;     memwrite(ea, y);    }
static void op_stz()    {   flag_t = 0;     memwrite(ea, 0);    }

static void op_st0()	{   flag_t = 0;		memread(ea);    clocktick();    memwrite_real(0x1FE000, tmpreg);	}
static void op_st1()    {   flag_t = 0;     memread(ea);    clocktick();    memwrite_real(0x1FE002, tmpreg);    }
static void op_st2()    {   flag_t = 0;     memread(ea);    clocktick();    memwrite_real(0x1FE003, tmpreg);    }

OPDECL(cla) { clocktick();   flag_t = 0;    a = 0; }
OPDECL(clx) { clocktick();   flag_t = 0;    x = 0; }
OPDECL(cly) { clocktick();   flag_t = 0;    y = 0; }

static void op_smb(int bit) {
	flag_t = 0;
	memread(ea);
	tmpreg |= 1 << bit;
	memwrite(ea, tmpreg);
}

static void op_rmb(int bit) {
	flag_t = 0;
	memread(ea);
	tmpreg &= ~(1 << bit);
	memwrite(ea, tmpreg);
}
