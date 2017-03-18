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

typedef struct opcode_s {
    uint8_t op;
    uint8_t am;
    char *name;
} opcode_t;

enum {
    a_end = 0,
    a_acc,
    a_imp,
    a_imm,
    a_abs,
    a_abx,
    a_aby,
    a_zpg,
    a_zpx,
    a_zpy,
    a_ind,
    a_inx,
    a_iny,
    a_rel,
    a_tra,
	a_zpr,
	a_bbr,
	a_tab
};

enum addrmodes {
    er=0,no=a_imp,ab=a_abs,ax=a_abx,ay=a_aby,ac=a_acc,im=a_imm,ix=a_inx,iy=a_iny,in=a_ind,re=a_rel,zp=a_zpg,zx=a_zpx,zy=a_zpy,bt=a_tra,zr=a_zpr,bb=a_bbr,ta=a_tab
};

static char optable[256][4] = {
        "BRK","ORA","???","ST0","???","ORA","ASL","???","PHP","ORA","ASL","???","???","ORA","ASL","BBR", /*00-0F*/
        "BPL","ORA","???","ST1","???","ORA","ASL","???","CLC","ORA","INC","???","???","ORA","ASL","BBR", /*10-1F*/
        "JSR","AND","???","ST2","BIT","AND","ROL","???","PLP","AND","ROL","???","BIT","AND","ROL","BBR", /*20-2F*/
        "BMI","AND","???","???","???","AND","ROL","???","SEC","AND","???","???","???","AND","ROL","BBR", /*30-3F*/
        "RTI","EOR","???","???","BSR","EOR","LSR","???","PHA","EOR","LSR","???","JMP","EOR","LSR","BBR", /*40-4F*/
        "BVC","EOR","???","TAM","CSL","EOR","LSR","???","CLI","EOR","???","???","???","EOR","LSR","BBR", /*50-5F*/
        "RTS","ADC","???","???","???","ADC","ROR","???","PLA","ADC","ROR","???","JMP","ADC","ROR","BBR", /*60-6F*/
        "BVS","ADC","???","???","???","ADC","ROR","???","SEI","ADC","???","???","???","ADC","ROR","BBR", /*70-7F*/
        "BRA","STA","???","TST","STY","STA","STX","???","DEY","???","TXA","???","STY","STA","STX","BBS", /*80-8F*/
        "BCC","STA","???","TST","STY","STA","STX","???","TYA","STA","TXS","???","STZ","STA","STZ","BBS", /*90-9F*/
        "LDY","LDA","LDX","TST","LDY","LDA","LDX","???","TAY","LDA","TAX","???","LDY","LDA","LDX","BBS", /*A0-AF*/
        "BCS","LDA","???","TST","LDY","LDA","LDX","???","CLV","LDA","TSX","???","LDY","LDA","LDX","BBS", /*B0-BF*/
        "CPY","CMP","CLY","???","CPY","CMP","DEC","???","INY","CMP","DEX","???","CPY","CMP","DEC","BBS", /*C0-CF*/
        "BNE","CMP","???","???","CSH","CMP","DEC","???","CLD","CMP","???","???","???","CMP","DEC","BBS", /*D0-DF*/
        "CPX","SBC","???","???","CPX","SBC","INC","???","INX","SBC","NOP","???","CPX","SBC","INC","BBS", /*E0-EF*/
        "BEQ","SBC","???","TAI","???","SBC","INC","???","SED","SBC","???","???","???","SBC","INC","BBS"  /*F0-FF*/
};
static uint8_t addrtable[256] = {
     /* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF         */
        im,ix,er,im,er,zp,zp,er,no,im,ac,er,er,ab,ab,bb, /*00-0f*/
        re,iy,er,im,er,zx,zx,er,no,ay,ac,er,er,ax,ax,bb, /*10-1f*/
        ab,ix,er,im,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,bb, /*20-2f*/
        re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*30-3f*/
        no,ix,er,er,re,zp,zp,er,no,im,ac,er,ab,ab,ab,bb, /*40-4f*/
        re,iy,er,im,no,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*50-5f*/
        no,ix,er,er,er,zp,zp,er,no,im,ac,er,in,ab,ab,bb, /*60-6f*/
        re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*70-7f*/
        re,ix,er,zp,zp,zp,zp,er,no,er,no,er,ab,ab,ab,bb, /*80-8f*/
        re,iy,er,ta,zx,zx,zy,er,no,ay,no,er,ab,ax,ax,bb, /*90-9f*/
        im,ix,im,zx,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*a0-af*/
        re,iy,er,ax,zx,zx,zy,er,no,ay,no,er,ax,ax,ay,bb, /*b0-bf*/
        im,ix,no,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*c0-cf*/
        re,iy,er,er,no,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*d0-df*/
        im,ix,er,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*e0-ef*/
        re,iy,er,bt,zx,zx,zx,er,no,ay,er,er,ax,ax,ax,bb  /*f0-ff*/
};

/*opcode_t opcodes[] = {
        {0x0A, a_acc, "asl"},
        {0x1A, a_acc, "inc"},

        {0x12, a_ind, "ora"},

        {0xD8, a_imp, "cld"},
        {0x18, a_imp, "clc"},
        {0x58, a_imp, "cli"},
        {0xB8, a_imp, "clv"},
        {0x78, a_imp, "sei"},
        {0xEA, a_imp, "nop"},

        {0x53, a_imm, "tam"},

        {0x54, a_imp, "csl"},
        {0xD4, a_imp, "csh"},

        {0x29, a_imm, "and"},
        {0x69, a_imm, "adc"},
        {0x89, a_imm, "bit"},
        {0xA9, a_imm, "lda"},

        {0x4C, a_abs, "jmp"},
        {0xAD, a_abs, "lda"},

        {0xF0, a_rel, "beq"},
        {0x80, a_rel, "bra"},

        {0xF3, a_tra, "tai"},

        {0x00, a_end, ""}
};

opcode_t *findop(uint8_t op)
{
    opcode_t *o = opcodes;

    while(o->am != a_end) {
        if(o->op == op)
            return(o);
        o++;
    }
    return(0);
}*/

static opcode_t curop;

opcode_t *findop(uint8_t op)
{
    curop.am = addrtable[op];
    curop.name = optable[op];
    return(&curop);
}

static char *get_flags(char *buf)
{
    sprintf(buf, "----:----");

    if(flag_c) buf[8] = 'C';
    if(flag_z) buf[7] = 'Z';
    if(flag_i) buf[6] = 'I';
    if(flag_d) buf[5] = 'D';
    if(flag_b) buf[3] = 'B';
    if(flag_t) buf[2] = 'T';
    if(flag_v) buf[1] = 'V';
    if(flag_n) buf[0] = 'N';
    return(buf);
}

uint32_t xlat(uint32_t n)
{
    uint32_t ret;

    ret = n & 0x1FFF;
    ret |= mpr[n >> 13] << 13;
    return(ret);
}

uint32_t huc6280_disasm(uint32_t dpc)
{
	uint32_t start = dpc;
	uint8_t opcode = READ_MEM(dpc);
	opcode_t *o = findop(opcode);
	char opbytes[64];
	char opdis[64];
	char buf[64];
	int oplen = 0;
	int i, j, offs;
	int maxlen = 7;
	int tmpea;
	static int counter = -1;

	if (disasm == 100) {
		disasm = 1;

		printf("opcode counts:\n\n");
		for (i = 0, j = 0; i < 256; i++) {
			printf("%02X:\t%s \t%d", i, optable[i], op_count[i]);
			j++;
			if (j == 4) {
				j = 0;
				printf("\n");
			}
			else {
				printf("\t || \t");
			}
		}
		counter = cycles + 200;
	}
	if (counter != -1 && cycles >= counter) {
		printf("counter expired, exiting.\n");
		system("pause");
		exit(0);
	}
//    printf("disasm from %04X (opcode = %02X)\n",dpc, opcode);
    if(o == 0) {
        printf("not found in oplist(opcode = %02X)\n", opcode);
        return ((uint32_t) -1);
    }

    memset(opbytes, ' ', 32);
    sprintf(opbytes, "%02X:%04X:  %02X %02X %02X %02X %02X %02X %02X %02X", mpr[dpc >> 13], dpc & 0x1FFF, READ_MEM(dpc), READ_MEM(dpc + 1), READ_MEM(dpc + 2), READ_MEM(dpc + 3), READ_MEM(dpc + 4), READ_MEM(dpc + 5), READ_MEM(dpc + 6), READ_MEM(dpc + 7));

    switch(o->am) {
        case a_imp:
            oplen = 1;
            sprintf(opdis, "%s  ", o->name);
            break;

        case a_acc:
            oplen = 1;
            sprintf(opdis, "%s  a", o->name);
            break;

        case a_rel:
            oplen = 2;
			offs = (int32_t)(dpc + 2);
			offs += (int16_t)(int8_t)READ_MEM(dpc + 1);
			sprintf(opdis, "%s  $%04X", o->name, offs);
            break;

        case a_imm:
            oplen = 2;
            sprintf(opdis, "%s  #$%02X", o->name, READ_MEM(dpc + 1));
            break;

        case a_zpg:
            oplen = 2;
            sprintf(opdis, "%s  $%02X", o->name, READ_MEM(dpc + 1));
            break;

		case a_zpx:
			oplen = 2;
			sprintf(opdis, "%s  $%02X, x", o->name, READ_MEM(dpc + 1));
			break;

		case a_zpy:
			oplen = 2;
			sprintf(opdis, "%s  $%02X, y", o->name, READ_MEM(dpc + 1));
			break;

		case a_abs:
			oplen = 3;
			sprintf(opdis, "%s  $%04X", o->name, READ_MEM16(dpc + 1));
			break;

		case a_tab:
			oplen = 4;
			sprintf(opdis, "%s  #$%02X, $%04X", o->name, READ_MEM(dpc + 1), READ_MEM16(dpc + 2));
			break;

		case a_abx:
			oplen = 3;
			sprintf(opdis, "%s  $%04X, x", o->name, READ_MEM16(dpc + 1));
			break;

		case a_aby:
			oplen = 3;
			sprintf(opdis, "%s  $%04X, y", o->name, READ_MEM16(dpc + 1));
			break;

		case a_ind:
			oplen = 2;
			sprintf(opdis, "%s  ($%02X)", o->name, READ_MEM(dpc + 1));
			break;

		case a_inx:
			oplen = 2;
			sprintf(opdis, "%s  ($%02X), x", o->name, READ_MEM(dpc + 1));
			break;

		case a_iny:
			oplen = 2;
			sprintf(opdis, "%s  ($%02X, y)", o->name, READ_MEM(dpc + 1));
			break;

		case a_bbr:
			oplen = 3;
			offs = (int32_t)(dpc + 2);
			offs += (int16_t)(int8_t)READ_MEM(dpc + 2);
			sprintf(opdis, "%s%d #$%02X, $%04X", o->name, o->op & 7, (dpc + 1), offs);
			break;

		case a_tra:
			oplen = 7;
			sprintf(opdis, "%s  $%06X, $%06X, $%04X", o->name, xlat(READ_MEM16(dpc + 1)), xlat(READ_MEM16(dpc + 3)), READ_MEM16(dpc + 5));
			break;

		case a_zpr:
			oplen = 3;
			offs = (int32_t)(dpc + 3);
			offs += (int16_t)(int8_t)READ_MEM(dpc + 2);
			sprintf(opdis, "%s  ($%02X), $%04X", o->name, READ_MEM16(dpc + 1), offs);
			break;

		default:
            oplen = 1;
            sprintf(opdis, "%s", o->name);
            break;
    }

    dpc += oplen;

    offs = oplen * 3 + 10;
    for(i = offs; i < (10 + maxlen * 3); i++) {
        opbytes[i] = ' ';
    }
    opbytes[(10 + maxlen * 3)] = 0;

    for(i = strlen(opdis); i < 32; i++) {
        opdis[i] = ' ';
    }
    opdis[i] = 0;

    printf(opbytes);
    printf(opdis);

    //print registers too (currently debugging the core...)
    printf("A:%02X X:%02X Y:%02X S:%02X PC=%04X P:%s", a, x, y, s, pc, get_flags(buf));

	{
		static FILE *fp = 0;

		if (fp == 0)
			fp = fopen("dis.txt", "wt");

		fprintf(fp, opbytes);
		fprintf(fp, opdis);
		fprintf(fp, "A:%02X X:%02X Y:%02X S:%02X PC=%04X P:%s\n", a, x, y, s, pc, get_flags(buf));
		fflush(fp);
	}

//    printf("\n");
    return(dpc);
}
