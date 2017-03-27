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

int disasm = 0;

int op_count[256];

static FILE *fp = 0;

#include "../huc6270/huc6270.h"

int huc6280_step()
{
	int ret = 0;
	uint8_t tmp;
	static int need_irq1 = 0;
	static uint32_t prevpc;

	if(0)
	{
		static char nextline[1024];

		if (fp == 0) {
			fp = fopen("bonkrev-cpulog.txt", "rt");
			if (disasm == 0)
				disasm = 1;
			fgets(nextline, 1024, fp);
		}

		if (feof(fp)) {
			exit(0);
		}

		char line[1024];
		int n_PC, n_A, n_X, n_Y, n_S, n_P, n_clock, n_mpr[8];
		int next_PC, next_A, next_X, next_Y, next_S, next_P, next_clock, next_mpr[8];

		strcpy(line, nextline);
		fgets(nextline, 1024, fp);

		sscanf(line, "PC:%04X A:%02X X:%02X Y:%02X S:%02X P:%02X C:%d MPR:%d %d %d %d %d %d %d %d\n", &n_PC, &n_A, &n_X, &n_Y, &n_S, &n_P, &n_clock, &n_mpr[0], &n_mpr[1], &n_mpr[2], &n_mpr[3], &n_mpr[4], &n_mpr[5], &n_mpr[6], &n_mpr[7]);
		sscanf(nextline, "PC:%04X A:%02X X:%02X Y:%02X S:%02X P:%02X C:%d MPR:%d %d %d %d %d %d %d %d\n", &next_PC, &next_A, &next_X, &next_Y, &next_S, &next_P, &next_clock, &next_mpr[0], &next_mpr[1], &next_mpr[2], &next_mpr[3], &next_mpr[4], &next_mpr[5], &next_mpr[6], &next_mpr[7]);

		if (next_PC == READ_MEM16(IRQ_IRQ1)) {
			need_irq1 = 1;
		}
		//	printf("PC:%04X A:%02X X:%02X Y:%02X S:%02X P:%02X C:%d\n", n_PC, n_A, n_X, n_Y, n_S, n_P, n_clock);

		int error = 0;

		compact_flags();

		if (pc != n_PC) { printf("pc doesnt match: (me= %04X != %04X)\n", pc, n_PC); error++; }
		if (a != n_A) { printf("a doesnt match: (me= %02X != %02X)\n", a, n_A); error++; }
		if (x != n_X) { printf("x doesnt match: (me= %02X != %02X)\n", x, n_X); error++; }
		if (y != n_Y) { printf("y doesnt match: (me= %02X != %02X)\n", y, n_Y); error++; }
		if (s != n_S) { printf("sp doesnt match: (me= %02X != %02X)\n", s, n_S); error++; }
		if (p != n_P) { printf("flags doesnt match: (me= %02X != %02X)\n", p, n_P); error++; }

		for (int i = 0; i < 8; i++) {
			if (mpr[i] != n_mpr[i]) { printf("mpr[%d] doesnt match: (me= %02X != %02X)\n", i, mpr[i], n_mpr[i]); error++; }
		}

		//disasm = 0;

		if (error) {
//			printf("PC = %04X\nVDP: cycle: %d, scanline: %d, frame: %d\n", pc, vdp.cycle, vdp.scanline, vdp.frame);
			huc6280_disasm(prevpc);
			system("pause");
			disasm = 1;
		}
	}
	
	if (disasm) {
		huc6280_disasm(pc);  printf(" OP:%02X CY:%d\n", opcode, clocks);
	}

	prevpc = pc;

    opcode = memread(pc++);
	op_count[opcode]++;
    switch(opcode) {
        case 0x44:      op_bsr_rel();       break;
        case 0x20:      op_jsr_abs();       break;
        case 0x4C:      op_jmp_abs();       break;
        case 0x6C:      op_jmp_abs_ind();   break;
        case 0x7C:      op_jmp_abx_ind();   break;

			//        case 0x4C: am_abs();    op_jmp();   break;
			//        case 0x6C: am_ind();    op_jmp();   break;

		case 0x83:	am_imm(); tmp = memread(ea); am_zpg(); op_tst(tmp); break;
		case 0x93:	am_imm(); tmp = memread(ea); am_abs(); op_tst(tmp); break;
		case 0xA3:	am_imm(); tmp = memread(ea); am_zpx(); op_tst(tmp); break;
		case 0xB3:	am_imm(); tmp = memread(ea); am_abx(); op_tst(tmp); break;

		case 0x04:	am_zpg();	op_tsb();	break;
		case 0x0C:	am_abs();	op_tsb();	break;
		case 0x14:	am_zpg();	op_trb();	break;
		case 0x1C:	am_abs();	op_trb();	break;

		case 0x0A:      op_asl_acc();       break;
        case 0x4A:      op_lsr_acc();       break;
        case 0x1A:      op_inc_acc();       break;
        case 0x3A:      op_dec_acc();       break;
        case 0x2A:      op_rol_acc();       break;
        case 0x6A:      op_ror_acc();       break;

		case 0x40: am_imp();    op_rti();   break;
		case 0x60: am_imp();    op_rts();   break;

        case 0xEA: am_imp();    op_nop();   break;

		case 0x0F: am_zpg();    op_bbr(0);	break;
		case 0x1F: am_zpg();    op_bbr(1);	break;
		case 0x2F: am_zpg();    op_bbr(2);	break;
		case 0x3F: am_zpg();    op_bbr(3);	break;
		case 0x4F: am_zpg();    op_bbr(4);	break;
		case 0x5F: am_zpg();    op_bbr(5);	break;
		case 0x6F: am_zpg();    op_bbr(6);	break;
		case 0x7F: am_zpg();    op_bbr(7);	break;
		case 0x8F: am_zpg();    op_bbs(0);	break;
		case 0x9F: am_zpg();    op_bbs(1);	break;
		case 0xAF: am_zpg();    op_bbs(2);	break;
		case 0xBF: am_zpg();    op_bbs(3);	break;
		case 0xCF: am_zpg();    op_bbs(4);	break;
		case 0xDF: am_zpg();    op_bbs(5);	break;
		case 0xEF: am_zpg();    op_bbs(6);	break;
		case 0xFF: am_zpg();    op_bbs(7);	break;

		case 0x46: am_zpg();    op_lsr();   break;
        case 0x56: am_zpx();    op_lsr();   break;
        case 0x4E: am_abs();    op_lsr();   break;
        case 0x5E: am_abx();    op_lsr();   break;
        case 0x06: am_zpg();    op_asl();   break;
        case 0x16: am_zpx();    op_asl();   break;
        case 0x0E: am_abs();    op_asl();   break;
        case 0x1E: am_abx();    op_asl();   break;

        case 0x26: am_zpg();    op_rol();   break;
        case 0x36: am_zpx();    op_rol();   break;
        case 0x2E: am_abs();    op_rol();   break;
        case 0x3E: am_abx();    op_rol();   break;
        case 0x66: am_zpg();    op_ror();   break;
        case 0x76: am_zpx();    op_ror();   break;
        case 0x6E: am_abs();    op_ror();   break;
        case 0x7E: am_abx();    op_ror();   break;

        case 0xC6: am_zpg();    op_dec();   break;
        case 0xD6: am_zpx();    op_dec();   break;
        case 0xCE: am_abs();    op_dec();   break;
        case 0xDE: am_abx();    op_dec();   break;
        case 0xE6: am_zpg();    op_inc();   break;
        case 0xF6: am_zpx();    op_inc();   break;
        case 0xEE: am_abs();    op_inc();   break;
        case 0xFE: am_abx();    op_inc();   break;

        case 0x88: am_imp();    op_dey();   break;
        case 0xCA: am_imp();    op_dex();   break;
        case 0xC8: am_imp();    op_iny();   break;
        case 0xE8: am_imp();    op_inx();   break;

        case 0x58: am_imp();    op_cli();   break;
        case 0xD8: am_imp();    op_cld();   break;
        case 0xB8: am_imp();    op_clv();   break;
        case 0x18: am_imp();    op_clc();   break;
        case 0x78: am_imp();    op_sei();   break;
        case 0xF4: am_imp();    op_set();   break;
        case 0xF8: am_imp();    op_sed();   break;
        case 0x38: am_imp();    op_sec();   break;

        case 0xAA: am_imp();    op_tax();   break;
        case 0x8A: am_imp();    op_txa();   break;
        case 0xA8: am_imp();    op_tay();   break;
        case 0x98: am_imp();    op_tya();   break;
        case 0xBA: am_imp();    op_tsx();   break;
        case 0x9A: am_imp();    op_txs();   break;

        case 0x09: am_imm();    op_ora();   break;
        case 0x05: am_zpg();    op_ora();   break;
        case 0x15: am_zpx();    op_ora();   break;
        case 0x12: am_ind();    op_ora();   break;
        case 0x01: am_inx();    op_ora();   break;
        case 0x11: am_iny();    op_ora();   break;
        case 0x0D: am_abs();    op_ora();   break;
        case 0x1D: am_abx();    op_ora();   break;
        case 0x19: am_aby();    op_ora();   break;

        case 0x49: am_imm();    op_eor();   break;
        case 0x45: am_zpg();    op_eor();   break;
        case 0x55: am_zpx();    op_eor();   break;
        case 0x52: am_ind();    op_eor();   break;
        case 0x41: am_inx();    op_eor();   break;
        case 0x51: am_iny();    op_eor();   break;
        case 0x4D: am_abs();    op_eor();   break;
        case 0x5D: am_abx();    op_eor();   break;
        case 0x59: am_aby();    op_eor();   break;

        case 0x29: am_imm();    op_and();   break;
        case 0x25: am_zpg();    op_and();   break;
        case 0x35: am_zpx();    op_and();   break;
        case 0x32: am_ind();    op_and();   break;
        case 0x21: am_inx();    op_and();   break;
        case 0x31: am_iny();    op_and();   break;
        case 0x2D: am_abs();    op_and();   break;
        case 0x3D: am_abx();    op_and();   break;
        case 0x39: am_aby();    op_and();   break;

        case 0x89: am_imm();    op_bit();   break;
        case 0x24: am_zpg();    op_bit();   break;
        case 0x34: am_zpx();    op_bit();   break;
        case 0x2C: am_abs();    op_bit();   break;
        case 0x3C: am_abx();    op_bit();   break;

        case 0xE0: am_imm();    op_cpx();   break;
        case 0xE4: am_zpg();    op_cpx();   break;
        case 0xEC: am_abs();    op_cpx();   break;

        case 0xC0: am_imm();    op_cpy();   break;
        case 0xC4: am_zpg();    op_cpy();   break;
        case 0xCC: am_abs();    op_cpy();   break;

        case 0xA9: am_imm();    op_lda();   break;
        case 0xA5: am_zpg();    op_lda();   break;
        case 0xB5: am_zpx();    op_lda();   break;
        case 0xB2: am_ind();    op_lda();   break;
        case 0xA1: am_inx();    op_lda();   break;
        case 0xB1: am_iny();    op_lda();   break;
        case 0xAD: am_abs();    op_lda();   break;
        case 0xBD: am_abx();    op_lda();   break;
        case 0xB9: am_aby();    op_lda();   break;

        case 0xA0: am_imm();    op_ldy();   break;
        case 0xA4: am_zpg();    op_ldy();   break;
        case 0xB4: am_zpx();    op_ldy();   break;
        case 0xAC: am_abs();    op_ldy();   break;
        case 0xBC: am_abx();    op_ldy();   break;

        case 0xA2: am_imm();    op_ldx();   break;
        case 0xA6: am_zpg();    op_ldx();   break;
        case 0xB6: am_zpy();    op_ldx();   break;
        case 0xAE: am_abs();    op_ldx();   break;
        case 0xBE: am_aby();    op_ldx();   break;

        case 0x85: am_zpg();    op_sta();   break;
        case 0x95: am_zpx();    op_sta();   break;
        case 0x92: am_ind();    op_sta();   break;
        case 0x81: am_inx();    op_sta();   break;
        case 0x91: am_iny();    op_sta();   break;
        case 0x8D: am_abs();    op_sta();   break;
        case 0x9D: am_abx();    op_sta();   break;
        case 0x99: am_aby();    op_sta();   break;

        case 0x86: am_zpg();    op_stx();   break;
        case 0x96: am_zpy();    op_stx();   break;
        case 0x8E: am_abs();    op_stx();   break;

        case 0x84: am_zpg();    op_sty();   break;
        case 0x94: am_zpx();    op_sty();   break;
        case 0x8C: am_abs();    op_sty();   break;

        case 0x10: am_rel();    op_bpl();   break;
        case 0x30: am_rel();    op_bmi();   break;
        case 0x50: am_rel();    op_bvc();   break;
        case 0x70: am_rel();    op_bvs();   break;
        case 0x80: am_rel();    op_bra();   break;
        case 0x90: am_rel();    op_bcc();   break;
        case 0xB0: am_rel();    op_bcs();   break;
        case 0xD0: am_rel();    op_bne();   break;
        case 0xF0: am_rel();    op_beq();   break;

        case 0x08: am_imp();    op_php();   break;
        case 0x48: am_imp();    op_pha();   break;
        case 0xDA: am_imp();    op_phx();   break;
        case 0x5A: am_imp();    op_phy();   break;

        case 0x28: am_imp();    op_plp();   break;
        case 0x68: am_imp();    op_pla();   break;
        case 0xFA: am_imp();    op_plx();   break;
        case 0x7A: am_imp();    op_ply();   break;

        case 0xC9: am_imm();    op_cmp();   break;
        case 0xC5: am_zpg();    op_cmp();   break;
        case 0xD5: am_zpx();    op_cmp();   break;
        case 0xD2: am_ind();    op_cmp();   break;
        case 0xC1: am_inx();    op_cmp();   break;
        case 0xD1: am_iny();    op_cmp();   break;
        case 0xCD: am_abs();    op_cmp();   break;
        case 0xDD: am_abx();    op_cmp();   break;
        case 0xD9: am_aby();    op_cmp();   break;

        case 0x69: am_imm();    op_adc();   break;
        case 0x65: am_zpg();    op_adc();   break;
        case 0x75: am_zpx();    op_adc();   break;
        case 0x72: am_ind();    op_adc();   break;
        case 0x61: am_inx();    op_adc();   break;
        case 0x71: am_iny();    op_adc();   break;
        case 0x6D: am_abs();    op_adc();   break;
        case 0x7D: am_abx();    op_adc();   break;
        case 0x79: am_aby();    op_adc();   break;
        case 0xE9: am_imm();    op_sbc();   break;
        case 0xE5: am_zpg();    op_sbc();   break;
        case 0xF5: am_zpx();    op_sbc();   break;
        case 0xF2: am_ind();    op_sbc();   break;
        case 0xE1: am_inx();    op_sbc();   break;
        case 0xF1: am_iny();    op_sbc();   break;
        case 0xED: am_abs();    op_sbc();   break;
        case 0xFD: am_abx();    op_sbc();   break;
        case 0xF9: am_aby();    op_sbc();   break;

		case 0x07: am_zpg();    op_rmb(0);  break;
		case 0x17: am_zpg();    op_rmb(1);  break;
		case 0x27: am_zpg();    op_rmb(2);  break;
		case 0x37: am_zpg();    op_rmb(3);  break;
		case 0x47: am_zpg();    op_rmb(4);  break;
		case 0x57: am_zpg();    op_rmb(5);  break;
		case 0x67: am_zpg();    op_rmb(6);  break;
		case 0x77: am_zpg();    op_rmb(7);  break;

		case 0x87: am_zpg();    op_smb(0);  break;
		case 0x97: am_zpg();    op_smb(1);  break;
		case 0xA7: am_zpg();    op_smb(2);  break;
		case 0xB7: am_zpg();    op_smb(3);  break;
		case 0xC7: am_zpg();    op_smb(4);  break;
		case 0xD7: am_zpg();    op_smb(5);  break;
		case 0xE7: am_zpg();    op_smb(6);  break;
		case 0xF7: am_zpg();    op_smb(7);  break;

		case 0x64: am_zpg();    op_stz();   break;
		case 0x74: am_zpx();    op_stz();   break;
        case 0x9C: am_abs();    op_stz();   break;
        case 0x9E: am_abx();    op_stz();   break;

		case 0x03: am_imm();    op_st0();   break;
		case 0x13: am_imm();    op_st1();   break;
        case 0x23: am_imm();    op_st2();   break;

		case 0x02: am_imp();    op_sxy();   break;
		case 0x22: am_imp();    op_sax();   break;
		case 0x42: am_imp();    op_say();   break;

		case 0x43: am_imm();    op_tma();   break;
		case 0x53: am_imm();    op_tam();   break;
		case 0x73: am_imp();    op_tii();   break;
		case 0xD3: am_imp();    op_tin();   break;
		case 0xE3: am_imp();    op_tia();   break;
		case 0xF3: am_imp();    op_tai();   break;

        case 0x54: am_imp();    op_csl();   break;
        case 0xD4: am_imp();    op_csh();   break;

		case 0x62: am_imp();    op_cla();   break;
		case 0x82: am_imp();    op_clx();   break;
		case 0xC2: am_imp();    op_cly();   break;


        default:
            printf("invalid opcode: %02X\n", opcode);
			system("pause");
			am_imp();
			op_nop();
//			ret = -1;
			break;
    }

	if (need_irq1) {
		prev_irqstate |= HUC6280_IRQ1;
		need_irq1 = 0;
	}

	if (prev_irqstate) {
		if (prev_irqstate & HUC6280_NMI) {
			//		NMISTATE = 0;
			irqstate &= ~HUC6280_NMI;
			execute_irq(IRQ_NMI);
		}
		else if (prev_irqstate & HUC6280_TIRQ) {	execute_irq(IRQ_TIRQ);	}
		else if (prev_irqstate & HUC6280_IRQ1) {	execute_irq(IRQ_IRQ1);	}
		else if (prev_irqstate & HUC6280_IRQ2) {	execute_irq(IRQ_IRQ2);	}
	}

	return(ret);
}

uint32_t huc6280_execute(uint32_t c)
{
    uint32_t start = cycles;
    uint32_t stop = cycles + c;

    while (cycles < stop) {// && BADOPCODE == 0) {
        huc6280_step();
    }
    return((uint32_t)(cycles - start));
}
