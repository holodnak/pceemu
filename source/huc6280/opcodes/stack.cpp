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

static void op_jmp()    {
    pc = ea;
}

static void op_jmp_abs()    {
    ea = memread(pc++);
    ea |= memread(pc++) << 8;

    pc = ea;
//    printf("mpr %d = %02X\n",pc >> 13, mpr[pc >> 13]);
}

static void op_jmp_abs_ind()    {
    ea = memread(pc++);
    ea |= memread(pc++) << 8;
//    clocktick();
    pc = memread(ea);
    ea = (ea & 0xFF00) | ((ea + 1) & 0xFF);
    pc |= memread(ea) << 8;
//    clocktick();
}

static void op_jmp_abx_ind()    {
    ea = memread(pc++);
    ea |= memread(pc++) << 8;
    ea += x;
    clocktick();
    pc = memread(ea++);
    pc |= memread(ea) << 8;
    clocktick();
}

static void op_jsr_abs()    {
    ea = memread(pc++);
    clocktick();
    push(pc >> 8);
    push(pc >> 0);
    ea |= memread(pc) << 8;
//    clocktick();
    pc = ea;
}

static void op_bsr_rel() {
    memread(pc);
    clocktick();
    push(pc >> 8);
    push(pc >> 0);
    clocktick();
    pc++;
    clocktick();
    pc += (uint16_t)((int16_t)((int8_t)tmpreg));
}

//rts/rti
static __inline void op_rts()
{
    clocktick();
    pc = pop();
    pc |= pop() << 8;
    clocktick();
    clocktick();
    pc++;
}

static __inline void op_rti()
{
    p = pop();
    expand_flags();
    pc = pop();
    pc |= pop() << 8;
    clocktick();
    clocktick();
}

static __inline void op_php()
{
    memread(ea);
    flag_t = 1;
    compact_flags();
    push(p | 0x10);
}

static __inline void op_pha()   {   memread(ea); push(a);    }
static __inline void op_phx()   {   memread(ea); push(x);    }
static __inline void op_phy()   {   memread(ea); push(y);    }

static __inline void op_plp() {
    memread(ea);
    clocktick();
    p = pop();
    expand_flags();
    flag_b = 0;
}

static __inline void op_pla() {
    memread(ea);
    clocktick();
    a = pop();
    checknz(a);
}

static __inline void op_plx() {
    memread(ea);
    clocktick();
    x = pop();
    checknz(x);
}

static __inline void op_ply() {
    memread(ea);
    clocktick();
    y = pop();
    checknz(y);
}
