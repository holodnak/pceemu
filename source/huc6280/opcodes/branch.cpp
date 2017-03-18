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

OPDECL(bra) {
        memread(ea);
		clocktick();
		pc += (uint16_t)((int16_t)((int8_t)tmpreg));
}

OPDECL(beq) {   if(flag_z)  op_bra();   else    clocktick();    }
OPDECL(bne) {   if(!flag_z) op_bra();   else    clocktick();    }
OPDECL(bcs) {   if(flag_c)  op_bra();   else    clocktick();    }
OPDECL(bcc) {   if(!flag_c) op_bra();   else    clocktick();    }
OPDECL(bvs) {   if(flag_v)  op_bra();   else    clocktick();    }
OPDECL(bvc) {   if(!flag_v) op_bra();   else    clocktick();    }
OPDECL(bmi) {   if(flag_n)  op_bra();   else    clocktick();    }
OPDECL(bpl) {   if(!flag_n) op_bra();   else    clocktick();    }

static __inline void op_bbs(int bit)
{
	uint8_t mask = 1 << bit;
	uint8_t tmp = memread(ea);

	flag_t = 0;
	memread(pc++);
	if (tmp & mask) {
		clocktick();
		pc += (uint16_t)((int16_t)((int8_t)tmpreg));
	}
}

static __inline void op_bbr(int bit)
{
	uint8_t mask = 1 << bit;
	uint8_t tmp = memread(ea);

	flag_t = 0;
	memread(pc++);
	if ((tmp & mask) == 0) {
		clocktick();
		pc += (uint16_t)((int16_t)((int8_t)tmpreg));
	}
}
