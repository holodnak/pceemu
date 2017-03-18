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

static void op_csl()    {   clocklen = 12;  clocktick();    }
static void op_csh()    {   clocklen = 3;   clocktick();    }

static void op_nop()    {   clocktick();    }

OPDECL(sax) {
	uint8_t tmp = a;

	a = x;
	x = tmp;
}

OPDECL(say) {
	uint8_t tmp = a;

	a = y;
	y = tmp;
}

OPDECL(sxy) {
	uint8_t tmp = y;

	y = x;
	x = tmp;
}
