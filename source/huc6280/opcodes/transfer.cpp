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

OPDECL(tax) {   clocktick();    x = a;  checknz(x); }
OPDECL(tay) {   clocktick();    y = a;  checknz(y); }
OPDECL(txa) {   clocktick();    a = x;  checknz(a); }
OPDECL(tya) {   clocktick();    a = y;  checknz(a); }
OPDECL(tsx) {   clocktick();    x = s;  checknz(x); }
OPDECL(txs) {   clocktick();    s = x;              }

OPDECL(tam) {
	int i;

	flag_t = 0;
	memread(ea);
	clocktick();
	clocktick();
	clocktick();
	if (tmpreg) {
		mpr_buf = a;
		for (i = 0; i<8; i++) {
			if (tmpreg & (1 << i))
				mpr[i] = a;
		}
	}
}

OPDECL(tma) {
	int i;

	flag_t = 0;
	memread(ea);
	clocktick();
	clocktick();
	switch (tmpreg) {
	case 0x00: break;
	case 0x01: mpr_buf = mpr[0]; break;
	case 0x02: mpr_buf = mpr[1]; break;
	case 0x04: mpr_buf = mpr[2]; break;
	case 0x08: mpr_buf = mpr[3]; break;
	case 0x10: mpr_buf = mpr[4]; break;
	case 0x20: mpr_buf = mpr[5]; break;
	case 0x40: mpr_buf = mpr[6]; break;
	case 0x80: mpr_buf = mpr[7]; break;
	default:
		printf("invalid tma!\n");
		break;
	}
	a = mpr_buf;
	if (tmpreg) {
		mpr_buf = a;
		for (i = 0; i<8; i++) {
			if (tmpreg & (1 << i))
				mpr[i] = a;
		}
	}
}


static void blocktransfer_start() {
    clocktick(); push(y);
    clocktick(); push(a);
    clocktick(); push(x);
    clocktick(); src = memread(pc++);
    clocktick(); src |= memread(pc++) << 8;
    clocktick(); dst = memread(pc++);
    clocktick(); dst |= memread(pc++) << 8;
    clocktick(); len = memread(pc++);
    clocktick(); len |= memread(pc++) << 8;
}

static void blocktransfer_end() {
    clocktick(); x = pop();
    clocktick(); a = pop();
    clocktick(); y = pop();
}

OPDECL(tai){
        blocktransfer_start();
        do {

            memread(src);
            clocktick(); src++;
            memwrite(dst, tmpreg);
            clocktick(); dst++;
            clocktick();
            clocktick(); len--;

            if(len == 0)
                break;

            memread(src);
            clocktick(); src--;
            memwrite(dst, tmpreg);
            clocktick(); dst++;
            clocktick();
            clocktick(); len--;

        } while(len);

        blocktransfer_end();
}

OPDECL(tia){
        blocktransfer_start();
        do {

            memread(src);
            clocktick(); src++;
            memwrite(dst, tmpreg);
            clocktick(); dst++;
            clocktick();
            clocktick(); len--;

            if(len == 0)
                break;

            memread(src);
            clocktick(); src++;
            memwrite(dst, tmpreg);
            clocktick(); dst--;
            clocktick();
            clocktick(); len--;

        } while(len);

        blocktransfer_end();
}

OPDECL(tii) {
	blocktransfer_start();
	do {

		memread(src);
		clocktick(); src++;
		memwrite(dst, tmpreg);
		clocktick(); dst++;
		clocktick();
		clocktick(); len--;

	} while (len);

	blocktransfer_end();
}

OPDECL(tin) {
	blocktransfer_start();
	do {

		memread(src);
		clocktick(); src++;
		memwrite(dst, tmpreg);
		clocktick();
		clocktick();
		clocktick(); len--;

	} while (len);

	blocktransfer_end();
}
