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

//implied
static void am_imp()
{
    ea = pc;
}

//immediate
static void am_imm()
{
    ea = pc++;
}

//relative
static void am_rel()
{
    ea = pc++;
}

//absolute
static void am_abs()
{
    ea = memread(pc++);
    ea |= memread(pc++) << 8;
}

//absolute x
static void am_abx()
{
    ea = memread(pc++);
    ea |= memread(pc++) << 8;
    ea += x;
}

//absolute y
static void am_aby()
{
    ea = memread(pc++);
    ea |= memread(pc++) << 8;
    ea += y;
}

static void am_ind()
{
    uint8_t t;

    t = memread(pc++);
    clocktick();
    ea = memread(t++ | ZEROPAGE);
    ea |= memread(t | ZEROPAGE) << 8;
}

static void am_inx()
{
    uint8_t t;

    t = memread(pc++);
    clocktick();
    t += x;
    ea = memread(t++ | ZEROPAGE);
    ea |= memread(t | ZEROPAGE) << 8;
}

static void am_iny()
{
    uint8_t t;

    t = memread(pc++);
    clocktick();
    ea = memread(t++ | ZEROPAGE);
    ea |= memread(t | ZEROPAGE) << 8;
    ea += y;
    clocktick();
    clocks -= 2;
}

static void am_zpg()
{
    uint8_t t;

    ea = memread(pc++) | ZEROPAGE;
}

static void am_zpx()
{
    uint8_t t;

    ea = ((memread(pc++) + x) & 0xFF) | ZEROPAGE;
    clocktick();
}

static void am_zpy()
{
    uint8_t t;

    ea = ((memread(pc++) + y) & 0xFF) | ZEROPAGE;
    clocktick();
}
