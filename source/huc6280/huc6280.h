//
// Created by james on 3/2/2017.
//

#ifndef PCEEMU_HUC6280_H
#define PCEEMU_HUC6280_H

#define HUC6280_NMI		1
#define HUC6280_TIRQ	2
#define HUC6280_IRQ1	4
#define HUC6280_IRQ2	8

typedef uint8_t(*readfunc_t)(void*, uint32_t);
typedef void(*writefunc_t)(void*, uint32_t, uint8_t);

/*
void huc6280_init();
void huc6280_reset();
int huc6280_step();
void huc6280_set_irq(int line);
void huc6280_clear_irq(int line);
void huc6280_setuser(void *user);
void huc6280_setreadfunc(int page, readfunc_t rd);
void huc6280_setwritefunc(int page, writefunc_t wr);
void huc6280_setreadmem(int page, uint8_t *p);
void huc6280_setwritemem(int page, uint8_t *p);
void huc6280_setmem(int page, uint8_t *p);
*/
#include "../PceDevice.h"
#include "../Pce.h"
#include <stdint.h>
#include <stdio.h>

extern CPce *pce;

#define MEM_READ	0x1
#define MEM_WRITE	0x2

class CPce;
class CHuc6280 : public CPceDevice
{
private:
	int cycles;
	uint32_t frameLines, pixelClock;
	uint32_t stripBurst;
	uint16_t colortable[512];
	uint32_t colortable_cache[512];
	uint16_t cta;

protected:

public:
	CHuc6280(CPce *p);
	~CHuc6280();

	void Reset();
	void Tick(int clocks);

	void SetIrq(uint32_t line);
	void ClearIrq(uint32_t line);

	void SetUserData(void *user);
	void SetReadFunc(int page, readfunc_t rd);
	void SetWriteFunc(int page, writefunc_t rd);
	void SetMem(int page, uint8_t *p, int flags = MEM_READ | MEM_WRITE);
};

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;

typedef uint8(*readfunc_t)(void*, uint32);
typedef void(*writefunc_t)(void*, uint32, uint8);

class CPicHuc6280 : public CPceDevice
{
private:
	uint64_t cycles;
	int32 clock;
	uint8 cycle_length;
	void *userdata;

	readfunc_t read_mem_funcs[256];
	writefunc_t write_mem_funcs[256];

	uint8 *read_mem_pages[256];
	uint8 *write_mem_pages[256];

	uint8 block_transfer;

	/* registers */
	uint8 A;       // accumulator
	uint8 X;       // index register X
	uint8 Y;       // index register Y
	uint8 S;       // stack pointer
	uint8 P;       // status register
	uint16 PC;     // program counter
	uint8 MPR[8];  // memory mapping registers
	uint16 SRC;    // source register
	uint16 DST;    // destination register
	uint16 LEN;    // length register
	uint8 MPRs_buffer;   // MPRs buffer
	uint8 read_buffer;   // read buffer
	uint8 IO_buffer;     // I/O buffer
	uint8 IRQ_lines;     // ...
	uint8 old_P;         // ...
	uint8 old_irq_mask;  // ...

#define irq_mask pce->IntCtrl()->Disable; //IntCtrl::disable

								/* P flag masks */
	const uint8 N_FLAG = 0x80;  // negative
	const uint8 V_FLAG = 0x40;  // overflow
	const uint8 T_FLAG = 0x20;  // X modified operation mode
	const uint8 B_FLAG = 0x10;  // break
	const uint8 D_FLAG = 0x08;  // decimal mode
	const uint8 I_FLAG = 0x04;  // interrupt disable
	const uint8 Z_FLAG = 0x02;  // zero
	const uint8 C_FLAG = 0x01;  // carry

								/* interrupt vector addresses */
	const uint16 RESET_VECTOR = 0xFFFE;
	const uint16 NMI_VECTOR = 0xFFFC;
	const uint16 TIRQ_VECTOR = 0xFFFA;
	const uint16 IRQ1_VECTOR = 0xFFF8;
	const uint16 IRQ2_VECTOR = 0xFFF6;

	/* interrupt line masks */
	const uint8 IRQ2_LINE = 1;
	const uint8 IRQ1_LINE = 2;
	const uint8 TIRQ_LINE = 4;


protected:

public:
	int disasm = 0;

public:
	CPicHuc6280(CPce *p);
	~CPicHuc6280();

	void Reset();
	void Tick(int clocks);
	int Execute(int clocks);

	void SetIrq(uint32_t line);
	void ClearIrq(uint32_t line);

	void SetUserData(void *user);
	void SetReadFunc(int page, readfunc_t rd);
	void SetWriteFunc(int page, writefunc_t rd);
	void SetMem(int page, uint8_t *p, int flags = MEM_READ | MEM_WRITE);

	void SetReadMem(int page, uint8_t *p) { SetMem(page, p, MEM_READ); }
	void SetWriteMem(int page, uint8_t *p) { SetMem(page, p, MEM_WRITE); }

	static uint8_t ReadMemPage(void *user, uint32_t addr) {
		CPce *pce = (CPce*)user;
		int page = (addr >> 13) & 0xFF;
		uint8_t *ptr = pce->huc6280->read_mem_pages[page];

		return(ptr ? ptr[addr & 0x1FFF] : 0xFF);
	}

	static void WriteMemPage(void *user, uint32_t addr, uint8_t data) {
		CPce *pce = (CPce*)user;
		int page = (addr >> 13) & 0xFF;
		uint8_t *ptr = pce->huc6280->write_mem_pages[page];

		if(ptr)
			ptr[addr & 0x1FFF] = data;
	}


	void adv_clk()
	{
		clock -= cycle_length;
		pce->Tick(cycle_length);
	}

#define ADV_CLK() \
	clock -= cycle_length; \
	pce->Tick(cycle_length);

	/* ---------- */

	uint8 read_abs(uint16 addr16)
	{
		uint8 page = MPR[addr16 >> 13];
		uint32 addr = ((uint32)page << 13) | (addr16 & 0x1FFF);
		uint8 ret;

	/*	if (read_mem_pages[page])
			ret = read_mem_pages[page][addr & 0x1FFF];
		else*/
			ret = (read_mem_funcs[page])(userdata, addr);
	//	printf("reading address %04X (page %02X)= %02X\n", addr16, page, ret);
		read_buffer = ret;
		return ret;
	}

	inline uint8 read_zp(uint8 addr8)
	{
		uint8 page = MPR[1];
		uint32 addr = ((uint32)page << 13) | (addr8);
		uint8 ret = (read_mem_funcs[page])(userdata, addr);

	//	printf("reading ZP addr %06X (page %02X)= %02X\n", addr, page, ret);
		read_buffer = ret;
		return ret;
	}

	inline uint8 read_stack()
	{
		uint8 page = MPR[1];
		uint32 addr = ((uint32)page << 13) | (0x100 | S);
		uint8 ret = (read_mem_funcs[page])(userdata, addr);

	//	printf("reading SP addr %06X (page %02X)= %02X\n", addr, page, ret);
		read_buffer = ret;
		return ret;
	}

	inline void write_abs(uint16 addr16, uint8 data)
	{
		uint8 page = MPR[addr16 >> 13];
		uint32 addr32 = ((uint32)page << 13) | (addr16 & 0x1FFF);
		(write_mem_funcs[page])(userdata, addr32, data);

	//	printf("writing address %04X (page %02X)= %02X\n", addr16, page, data);
	}

	inline void write_zp(uint8 addr8, uint8 data)
	{
		uint8 page = MPR[1];
		uint32 addr32 = ((uint32)page << 13) | (addr8);
		(write_mem_funcs[page])(userdata, addr32, data);

	//	printf("writing ZP addr %06X (page %02X)= %02X\n", addr32, page, data);
	}

	inline void write_stack(uint8 data)
	{
		uint8 page = MPR[1];
		uint32 addr32 = ((uint32)page << 13) | (0x100 | S);
		(write_mem_funcs[page])(userdata, addr32, data);

	//	printf("writing SP addr %06X (page %02X)= %02X\n", addr32, page, data);
	}


	/* ---------- */

#define FETCH_OPCODE() \
  ADV_CLK(); read_abs(PC); PC++;

#define FETCH_IMPL() \
  ADV_CLK(); read_abs(PC);

#define FETCH_IMM() \
  ADV_CLK(); uint8 M = read_abs(PC); PC++;

#define FETCH_ZP() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK();

#define FETCH_ZPX() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); addr8 += X;

#define FETCH_ZPY() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); addr8 += Y;

#define FETCH_ABS() \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; PC++; \
  ADV_CLK();

#define FETCH_ABSX() \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); addr16 += X;

#define FETCH_ABSY() \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); addr16 += Y;

#define FETCH_ZPIND() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); \
  ADV_CLK(); uint16 addr16 = read_zp(addr8); addr8++; \
  ADV_CLK(); addr16 |= read_zp(addr8) << 8; \
  ADV_CLK();

#define FETCH_ZPXIND() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); addr8 += X; \
  ADV_CLK(); uint16 addr16 = read_zp(addr8); addr8++; \
  ADV_CLK(); addr16 |= read_zp(addr8) << 8; \
  ADV_CLK();

#define FETCH_ZPINDY() \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); \
  ADV_CLK(); uint16 addr16 = read_zp(addr8); addr8++; \
  ADV_CLK(); addr16 |= read_zp(addr8) << 8; \
  ADV_CLK(); addr16 += Y;

#define LD_IMM() \
  FETCH_IMM();

#define LD_ZP() \
  FETCH_ZP(); \
  ADV_CLK(); uint8 M = read_zp(addr8);

#define LD_ZPX() \
  FETCH_ZPX(); \
  ADV_CLK(); uint8 M = read_zp(addr8);

#define LD_ZPY() \
  FETCH_ZPY(); \
  ADV_CLK(); uint8 M = read_zp(addr8);

#define LD_ABS() \
  FETCH_ABS(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_ABSX() \
  FETCH_ABSX(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_ABSY() \
  FETCH_ABSY(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_ZPIND() \
  FETCH_ZPIND(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_ZPXIND() \
  FETCH_ZPXIND(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_ZPINDY() \
  FETCH_ZPINDY(); \
  ADV_CLK(); uint8 M = read_abs(addr16);

#define LD_IMM_ZP() \
  ADV_CLK(); uint8 Mi = read_abs(PC); PC++; \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); \
  ADV_CLK(); \
  ADV_CLK(); uint8 M = read_zp(addr8); \
  ADV_CLK();

#define LD_IMM_ZPX() \
  ADV_CLK(); uint8 Mi = read_abs(PC); PC++; \
  ADV_CLK(); uint8 addr8 = read_abs(PC); PC++; \
  ADV_CLK(); addr8 += X; \
  ADV_CLK(); \
  ADV_CLK(); uint8 M = read_zp(addr8); \
  ADV_CLK();

#define LD_IMM_ABS() \
  ADV_CLK(); uint8 Mi = read_abs(PC); PC++; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); \
  ADV_CLK(); \
  ADV_CLK(); uint8 M = read_abs(addr16); \
  ADV_CLK();

#define LD_IMM_ABSX() \
  ADV_CLK(); uint8 Mi = read_abs(PC); PC++; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); addr16 += X; \
  ADV_CLK(); \
  ADV_CLK(); uint8 M = read_abs(addr16); \
  ADV_CLK();

#define ST_ZP(reg) \
  P&=~T_FLAG; \
  FETCH_ZP(); \
  ADV_CLK(); write_zp(addr8, reg);

#define ST_ZPX(reg) \
  P&=~T_FLAG; \
  FETCH_ZPX(); \
  ADV_CLK(); write_zp(addr8, reg);

#define ST_ZPY(reg) \
  P&=~T_FLAG; \
  FETCH_ZPY(); \
  ADV_CLK(); write_zp(addr8, reg);

#define ST_ABS(reg) \
  P&=~T_FLAG; \
  FETCH_ABS(); \
  ADV_CLK(); write_abs(addr16, reg);

#define ST_ABSX(reg) \
  P&=~T_FLAG; \
  FETCH_ABSX(); \
  ADV_CLK(); write_abs(addr16, reg);

#define ST_ABSY(reg) \
  P&=~T_FLAG; \
  FETCH_ABSY(); \
  ADV_CLK(); write_abs(addr16, reg);

#define ST_ZPIND(reg) \
  P&=~T_FLAG; \
  FETCH_ZPIND(); \
  ADV_CLK(); write_abs(addr16, reg);

#define ST_ZPXIND(reg) \
  P&=~T_FLAG; \
  FETCH_ZPXIND(); \
  ADV_CLK(); write_abs(addr16, reg);

#define ST_ZPINDY(reg) \
  P&=~T_FLAG; \
  FETCH_ZPINDY(); \
  ADV_CLK(); write_abs(addr16, reg);

#define WR_ZP() \
  ADV_CLK(); \
  ADV_CLK(); write_zp(addr8, M);

#define WR_ABS() \
  ADV_CLK(); \
  ADV_CLK(); write_abs(addr16, M);

#define PCL \
  (uint8)PC

#define PCH \
  (uint8)(PC >> 8)

#define UPD_NZ(reg) \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG); \
  P |= reg & N_FLAG; \
  if (reg == 0) P |= Z_FLAG;

	void start() // 8 o 7 cicli?
	{
		ADV_CLK(); // NOT TESTED
		ADV_CLK(); // 'PC++;'? NOT TESTED
		ADV_CLK(); S--;
		ADV_CLK(); S--;
		ADV_CLK(); S--;
		ADV_CLK(); PC = read_abs(RESET_VECTOR);
		ADV_CLK(); PC |= read_abs(RESET_VECTOR + 1) << 8;
		ADV_CLK(); P &= ~(T_FLAG | D_FLAG); P |= I_FLAG;  // necessario 'old_P = P;'?
	}

#define INTERRUPT(vector) \
 { \
  FETCH_IMPL(); \
  FETCH_IMPL(); \
  ADV_CLK(); write_stack(PCH); S--; \
  ADV_CLK(); write_stack(PCL); S--; \
  ADV_CLK(); write_stack(P & ~B_FLAG); S--; \
  ADV_CLK(); PC = read_abs(vector); \
  ADV_CLK(); PC |= read_abs(vector+1) << 8; \
  ADV_CLK(); P &= ~(T_FLAG|D_FLAG); P |= I_FLAG; old_P = P; \
 }

#define ADC() \
  if ((P & T_FLAG) == 0) \
  { \
   uint8 C = P & C_FLAG; \
   uint32 sum = A + M + C; \
   if ((P & D_FLAG) == 0) \
   { \
    P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
    P |= (uint8)(sum >> 8); \
    P |= (uint8)(((~(A ^ M) & (A ^ sum)) >> 1) & V_FLAG); \
   } \
   else \
   { \
    ADV_CLK(); \
    P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
    if (((A & 0x0F) + (M & 0x0F) + C) > 0x09) sum += 0x06; \
    if (sum > 0x9F) { sum += 0x60; P |= C_FLAG; } \
   } \
   A = (uint8)sum; \
   P |= A & N_FLAG; \
   if (A == 0) P |= Z_FLAG; \
  } \
  else \
  { \
   ADV_CLK(); uint8 Mx = read_zp(X); \
   ADV_CLK(); \
   uint8 C = P & C_FLAG; \
   uint32 sum = Mx + M + C; \
   if ((P & D_FLAG) == 0) \
   { \
    P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
    P |= (uint8)(sum >> 8); \
    P |= (uint8)(((~(Mx ^ M) & (Mx ^ sum)) >> 1) & V_FLAG); \
   } \
   else \
   { \
    ADV_CLK(); \
    P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
    if (((Mx & 0x0F) + (M & 0x0F) + C) > 0x09) sum += 0x06; \
    if (sum > 0x9F) { sum += 0x60; P |= C_FLAG; } \
   } \
   Mx = (uint8)sum; \
   P |= Mx & N_FLAG; \
   if (Mx == 0) P |= Z_FLAG; \
   ADV_CLK(); write_zp(X, Mx); \
  }

#define SBC() \
  uint8 C = ~P & C_FLAG; \
  uint32 dif = A - M - C; \
  P |= C_FLAG; \
  if ((P & D_FLAG) == 0) \
  { \
   P ^= (uint8)(((dif >> 8) & C_FLAG)); \
   P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG); \
   P |= (uint8)((((A ^ M) & (A ^ dif)) >> 1) & V_FLAG); \
   A = (uint8)dif; \
   P |= A & N_FLAG; \
   if (A == 0) P |= Z_FLAG; \
  } \
  else \
  { \
   ADV_CLK(); \
   if ((int32)dif < 0x00) { dif -= 0x60; P &= ~C_FLAG; } \
   if ((int8)((A & 0x0F) - (M & 0x0F) - C) < 0x00) dif -= 0x06; \
   A = (uint8)dif; \
   UPD_NZ(A); \
  }

#define AND() \
  if ((P & T_FLAG) == 0) \
  { \
   A &= M; UPD_NZ(A); \
  } \
  else \
  { \
   ADV_CLK(); uint8 Mx = read_zp(X); \
   ADV_CLK(); Mx &= M; UPD_NZ(Mx); \
   ADV_CLK(); write_zp(X, Mx); \
  }

#define EOR() \
  if ((P & T_FLAG) == 0) \
  { \
   A ^= M; UPD_NZ(A); \
  } \
  else \
  { \
   ADV_CLK(); uint8 Mx = read_zp(X); \
   ADV_CLK(); Mx ^= M; UPD_NZ(Mx); \
   ADV_CLK(); write_zp(X, Mx); \
  }

#define ORA() \
  if ((P & T_FLAG) == 0) \
  { \
   A |= M; UPD_NZ(A); \
  } \
  else \
  { \
   ADV_CLK(); uint8 Mx = read_zp(X); \
   ADV_CLK(); Mx |= M; UPD_NZ(Mx); \
   ADV_CLK(); write_zp(X, Mx); \
  }

#define BIT() \
  P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG); \
  P |= M & (N_FLAG|V_FLAG); \
  if ((M & A) == 0) P |= Z_FLAG;

#define CMP() \
  uint32 dif = A - M; \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= (uint8)dif & N_FLAG; \
  if ((uint8)dif == 0) P |= Z_FLAG; \
  P |= (uint8)((~dif >> 8) & C_FLAG);

#define CPX() \
  uint32 dif = X - M; \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= (uint8)dif & N_FLAG; \
  if ((uint8)dif == 0) P |= Z_FLAG; \
  P |= (uint8)((~dif >> 8) & C_FLAG);

#define CPY() \
  uint32 dif = Y - M; \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= (uint8)dif & N_FLAG; \
  if ((uint8)dif == 0) P |= Z_FLAG; \
  P |= (uint8)((~dif >> 8) & C_FLAG);

#define LDA() \
  A = M; UPD_NZ(A);

#define LDX() \
  X = M; UPD_NZ(X);

#define LDY() \
  Y = M; UPD_NZ(Y);

#define ASL() \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= M >> 7; \
  M <<= 1; \
  if (M == 0) P |= Z_FLAG; \
  P |= M & N_FLAG;

#define ASL_A() \
  FETCH_IMPL(); \
  uint8 M = A; ASL(); A = M;

#define LSR() \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= M & C_FLAG; \
  M >>= 1; \
  if (M == 0) P |= Z_FLAG;

#define LSR_A() \
  FETCH_IMPL(); \
  uint8 M = A; LSR(); A = M;

#define BBR(bit) \
  P &= ~T_FLAG; \
  LD_ZP(); \
  ADV_CLK(); \
  ADV_CLK(); int8 offset = (int8)read_abs(PC); PC++; \
  if ((M & (1 << bit)) == 0) \
  { \
   ADV_CLK(); \
   ADV_CLK(); PC += (uint16)((int16)offset); \
  }

#define BBS(bit) \
  P &= ~T_FLAG; \
  LD_ZP(); \
  ADV_CLK(); \
  ADV_CLK(); int8 offset = (int8)read_abs(PC); PC++; \
  if ((M & (1 << bit)) != 0) \
  { \
   ADV_CLK(); \
   ADV_CLK(); PC += (uint16)((int16)offset); \
  }

#define ROL() \
  uint8 C = P & C_FLAG; \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= M >> 7; \
  M <<= 1; M |= C; \
  if (M == 0) P |= Z_FLAG; \
  P |= M & N_FLAG;

#define ROL_A() \
  FETCH_IMPL(); \
  uint8 M = A; ROL(); A = M;

#define ROR() \
  uint8 C = P << 7; \
  P &= ~(N_FLAG|T_FLAG|Z_FLAG|C_FLAG); \
  P |= M & C_FLAG; \
  M >>= 1; M |= C; \
  if (M == 0) P |= Z_FLAG; \
  P |= M & N_FLAG;

#define ROR_A() \
  FETCH_IMPL(); \
  uint8 M = A; ROR(); A = M;

#define RMB(bit) \
  P &= ~T_FLAG; \
  LD_ZP(); \
  ADV_CLK(); \
  ADV_CLK(); M &= ~(1 << bit); \
  ADV_CLK(); write_zp(addr8, M);

#define SMB(bit) \
  P &= ~T_FLAG; \
  LD_ZP(); \
  ADV_CLK(); \
  ADV_CLK(); M |= (1 << bit); \
  ADV_CLK(); write_zp(addr8, M);

#define ST_VDC(reg) \
  P &= ~T_FLAG; \
  LD_IMM(); \
  ADV_CLK(); \
  ADV_CLK();  \
  write_mem_funcs[0xFF](userdata, reg, M);
//VDC::write(reg, M);

#define TRB() \
  P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG); \
  P |= M & (N_FLAG|V_FLAG); \
  M &= ~A; \
  if (M == 0) P |= Z_FLAG;

#define TSB() \
  P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG); \
  P |= M & (N_FLAG|V_FLAG); \
  M |= A; \
  if (M == 0) P |= Z_FLAG;

#define TST() \
  P &= ~(N_FLAG|V_FLAG|T_FLAG|Z_FLAG); \
  P |= M & (N_FLAG|V_FLAG); \
  if ((M & Mi) == 0) P |= Z_FLAG;

#define NOP() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); 

#define ILL() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); 

#define BSR() \
  P &= ~T_FLAG; \
  ADV_CLK(); int8 offset = (int8)read_abs(PC); \
  ADV_CLK(); \
  ADV_CLK(); write_stack(PCH); S--; \
  ADV_CLK(); write_stack(PCL); S--; \
  ADV_CLK(); PC++; \
  ADV_CLK(); \
  ADV_CLK(); PC += (uint16)((int16)offset);

#define JSR() \
  P &= ~T_FLAG; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); \
  ADV_CLK(); write_stack(PCH); S--; \
  ADV_CLK(); write_stack(PCL); S--; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; \
  ADV_CLK(); PC = addr16;

#define BRK() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); PC++; \
  ADV_CLK(); write_stack(PCH); S--; \
  ADV_CLK(); write_stack(PCL); S--; \
  ADV_CLK(); write_stack(P); S--; \
  ADV_CLK(); PC = read_abs(IRQ2_VECTOR); \
  ADV_CLK(); PC |= read_abs(IRQ2_VECTOR+1) << 8; \
  ADV_CLK(); P &= ~D_FLAG; P |= I_FLAG; old_P = P;

#define JMP_ABS() \
  P &= ~T_FLAG; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; \
  ADV_CLK(); PC = addr16;

#define JMP_ABSIND() \
  P &= ~T_FLAG; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; \
  ADV_CLK(); \
  ADV_CLK(); PC = read_abs(addr16); addr16++; \
  ADV_CLK(); PC |= read_abs(addr16) << 8; \
  ADV_CLK();

#define JMP_ABSXIND() \
  P &= ~T_FLAG; \
  ADV_CLK(); uint16 addr16 = read_abs(PC); PC++; \
  ADV_CLK(); addr16 |= read_abs(PC) << 8; \
  ADV_CLK(); addr16 += X; \
  ADV_CLK(); PC = read_abs(addr16); addr16++; \
  ADV_CLK(); PC |= read_abs(addr16) << 8; \
  ADV_CLK();

#define RTI() \
  FETCH_IMPL(); \
  ADV_CLK(); S++; \
  ADV_CLK(); P = read_stack(); P |= B_FLAG; old_P = P; S++; \
  ADV_CLK(); PC = read_stack(); S++; \
  ADV_CLK(); PC |= read_stack() << 8; \
  ADV_CLK();

#define RTS() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  ADV_CLK(); S++; \
  ADV_CLK(); PC = read_stack(); S++; \
  ADV_CLK(); PC |= read_stack() << 8; \
  ADV_CLK(); \
  ADV_CLK(); PC++;

#define TAM() \
  P &= ~T_FLAG; \
  LD_IMM(); \
  ADV_CLK(); \
  ADV_CLK(); \
  ADV_CLK(); \
  if (M != 0x00) \
  { \
   MPRs_buffer = A; \
   if ((M & 0x01) != 0) MPR[0] = A; \
   if ((M & 0x02) != 0) MPR[1] = A; \
   if ((M & 0x04) != 0) MPR[2] = A; \
   if ((M & 0x08) != 0) MPR[3] = A; \
   if ((M & 0x10) != 0) MPR[4] = A; \
   if ((M & 0x20) != 0) MPR[5] = A; \
   if ((M & 0x40) != 0) MPR[6] = A; \
   if ((M & 0x80) != 0) MPR[7] = A; \
  }

#define TMA() \
  P &= ~T_FLAG; \
  LD_IMM(); \
  ADV_CLK(); \
  ADV_CLK(); \
  switch (M) \
  { \
   case 0x00: A = MPRs_buffer; break; \
   case 0x01: A = MPRs_buffer = MPR[0]; break; \
   case 0x02: A = MPRs_buffer = MPR[1]; break; \
   case 0x04: A = MPRs_buffer = MPR[2]; break; \
   case 0x08: A = MPRs_buffer = MPR[3]; break; \
   case 0x10: A = MPRs_buffer = MPR[4]; break; \
   case 0x20: A = MPRs_buffer = MPR[5]; break; \
   case 0x40: A = MPRs_buffer = MPR[6]; break; \
   case 0x80: A = MPRs_buffer = MPR[7]; break; \
   default: \
   { \
    uint8 ones[8] = {0}; \
    uint8 delta = 1; \
    for (uint32 i = 0; i < 8; i++) \
    { \
     if ((M & (0x01 << i)) != 0) \
     { \
      delta++; \
      if ((MPR[i] & 0x01) != 0) ones[0]++; \
      if ((MPR[i] & 0x02) != 0) ones[1]++; \
      if ((MPR[i] & 0x04) != 0) ones[2]++; \
      if ((MPR[i] & 0x08) != 0) ones[3]++; \
      if ((MPR[i] & 0x10) != 0) ones[4]++; \
      if ((MPR[i] & 0x20) != 0) ones[5]++; \
      if ((MPR[i] & 0x40) != 0) ones[6]++; \
      if ((MPR[i] & 0x80) != 0) ones[7]++; \
     } \
    } \
    delta >>= 1; \
    MPRs_buffer = 0; \
    if (ones[0] >= delta) MPRs_buffer |= 0x01; \
    if (ones[1] >= delta) MPRs_buffer |= 0x02; \
    if (ones[2] >= delta) MPRs_buffer |= 0x04; \
    if (ones[3] >= delta) MPRs_buffer |= 0x08; \
    if (ones[4] >= delta) MPRs_buffer |= 0x10; \
    if (ones[5] >= delta) MPRs_buffer |= 0x20; \
    if (ones[6] >= delta) MPRs_buffer |= 0x40; \
    if (ones[7] >= delta) MPRs_buffer |= 0x80; \
    A = MPRs_buffer; \
   } \
  }

#define CSH() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  cycle_length = 1; \
  ADV_CLK();

#define CSL() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  cycle_length = 4; \
  ADV_CLK();

#define BRA() \
  P &= ~T_FLAG; \
  ADV_CLK(); int8 offset = (int8)read_abs(PC); PC++; \
  ADV_CLK(); \
  ADV_CLK(); PC += (uint16)((int16)offset);

#define DEC() \
  M--; UPD_NZ(M);

#define DEC_A() \
  FETCH_IMPL(); \
  A--; UPD_NZ(A);

#define DEX() \
  FETCH_IMPL(); \
  X--; UPD_NZ(X);

#define DEY() \
  FETCH_IMPL(); \
  Y--; UPD_NZ(Y);

#define INC() \
  M++; UPD_NZ(M);

#define INC_A() \
  FETCH_IMPL(); \
  A++; UPD_NZ(A);

#define INX() \
  FETCH_IMPL(); \
  X++; UPD_NZ(X);

#define INY() \
  FETCH_IMPL(); \
  Y++; UPD_NZ(Y);

#define PLP() \
  FETCH_IMPL(); \
  ADV_CLK(); S++; \
  ADV_CLK(); P = read_stack(); P |= B_FLAG;

#define PH(reg) \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  ADV_CLK(); write_stack(reg); S--;

#define PL(reg) \
  FETCH_IMPL(); \
  ADV_CLK(); S++; \
  ADV_CLK(); reg = read_stack(); \
  UPD_NZ(reg);

#define TXS() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); S = X;

#define BR(cond) \
  P &= ~T_FLAG; \
  ADV_CLK(); int8 offset = (int8)read_abs(PC); PC++; \
  if (cond) \
  { \
   FETCH_IMPL(); \
   ADV_CLK(); PC += (uint16)((int16)offset); \
  }

#define SWP(reg1,reg2) \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  ADV_CLK(); \
  uint8 temp = reg1; \
  reg1 = reg2; \
  reg2 = temp;

#define TRN(reg1,reg2) \
  FETCH_IMPL(); \
  reg2 = reg1; \
  UPD_NZ(reg2);

#define CL_REG(reg) \
  P &= ~T_FLAG; \
  FETCH_IMPL(); reg = 0;

#define CL_FLAG(flag) \
  P &= ~(T_FLAG|flag); \
  FETCH_IMPL();

#define SEC() \
  P &= ~T_FLAG; P |= C_FLAG; \
  FETCH_IMPL();

#define SED() \
  P &= ~T_FLAG; P |= D_FLAG; \
  FETCH_IMPL();

#define SEI() \
  P &= ~T_FLAG; P |= I_FLAG; \
  FETCH_IMPL();

#define SET() \
  P |= T_FLAG; \
  FETCH_IMPL();

#define CLI() \
  P &= ~(T_FLAG|I_FLAG); \
  FETCH_IMPL();

#define BT_BEGIN() \
  P &= ~T_FLAG; \
  FETCH_IMPL(); \
  ADV_CLK(); \
  ADV_CLK(); write_stack(Y); S--; \
  ADV_CLK(); write_stack(A); S--; \
  ADV_CLK(); write_stack(X); \
  ADV_CLK(); SRC = read_abs(PC); PC++; \
  ADV_CLK(); SRC |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); DST = read_abs(PC); PC++; \
  ADV_CLK(); DST |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); LEN = read_abs(PC); PC++; \
  ADV_CLK(); LEN |= read_abs(PC) << 8; PC++; \
  ADV_CLK(); \
  ADV_CLK();

#define BT_END() \
  ADV_CLK(); X = read_stack(); S++; \
  ADV_CLK(); A = read_stack(); S++; \
  ADV_CLK(); Y = read_stack(); \
  old_irq_mask = irq_mask;

#define TAI() \
  BT_BEGIN(); \
  block_transfer = BT_TAI; \
  continue_TAI: \
  do \
  { \
   if (clock <= 0) return(1); \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC++; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST++; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   if (LEN == 0) break; \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC--; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST++; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   VDC::resync(); \
  } \
  while (LEN != 0); \
  block_transfer = 0; \
  BT_END();

#define TDD() \
  BT_BEGIN(); \
  block_transfer = BT_TDD; \
  continue_TDD: \
  do \
  { \
   if (clock <= 0) return(1); \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC--; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST--; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   VDC::resync(); \
  } \
  while (LEN != 0); \
  block_transfer = 0; \
  BT_END();

#define TIA() \
  BT_BEGIN(); \
  block_transfer = BT_TIA; \
  continue_TIA: \
  do \
  { \
   if (clock <= 0) return(1); \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC++; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST++; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   if (LEN == 0) break; \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC++; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST--; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   VDC::resync(); \
  } \
  while (LEN != 0); \
  block_transfer = 0; \
  BT_END();

#define TII() \
  BT_BEGIN(); \
  block_transfer = BT_TII; \
  continue_TII: \
  do \
  { \
   if (clock <= 0) return(1); \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC++; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); DST++; \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   VDC::resync(); \
  } \
  while (LEN != 0); \
  block_transfer = 0; \
  BT_END();

#define TIN() \
  BT_BEGIN(); \
  block_transfer = BT_TIN; \
  continue_TIN: \
  do \
  { \
   if (clock <= 0) return(1); \
   ADV_CLK(); read_abs(SRC); \
   ADV_CLK(); SRC++; \
   ADV_CLK(); write_abs(DST, read_buffer); \
   ADV_CLK(); \
   ADV_CLK(); \
   ADV_CLK(); LEN--; \
   VDC::resync(); \
  } \
  while (LEN != 0); \
  block_transfer = 0; \
  BT_END();

	/*------------------------------------------------*/

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
	er = 0, no = a_imp, ab = a_abs, ax = a_abx, ay = a_aby, ac = a_acc, im = a_imm, ix = a_inx, iy = a_iny, in = a_ind, re = a_rel, zp = a_zpg, zx = a_zpx, zy = a_zpy, bt = a_tra, zr = a_zpr, bb = a_bbr, ta = a_tab
};

char optable[256][4] = {
	"BRK","ORA","???","ST0","???","ORA","ASL","???","PHP","ORA","ASL","???","???","ORA","ASL","BBR", /*00-0F*/
	"BPL","ORA","???","ST1","???","ORA","ASL","???","CLC","ORA","INC","???","???","ORA","ASL","BBR", /*10-1F*/
	"JSR","AND","???","ST2","BIT","AND","ROL","???","PLP","AND","ROL","???","BIT","AND","ROL","BBR", /*20-2F*/
	"BMI","AND","???","???","???","AND","ROL","???","SEC","AND","DEC","???","???","AND","ROL","BBR", /*30-3F*/
	"RTI","EOR","???","???","BSR","EOR","LSR","???","PHA","EOR","LSR","???","JMP","EOR","LSR","BBR", /*40-4F*/
	"BVC","EOR","???","TAM","CSL","EOR","LSR","???","CLI","EOR","???","???","???","EOR","LSR","BBR", /*50-5F*/
	"RTS","ADC","CLA","???","STZ","ADC","ROR","???","PLA","ADC","ROR","???","JMP","ADC","ROR","BBR", /*60-6F*/
	"BVS","ADC","???","TII","STZ","ADC","ROR","???","SEI","ADC","???","???","???","ADC","ROR","BBR", /*70-7F*/
	"BRA","STA","CLX","TST","STY","STA","STX","???","DEY","???","TXA","???","STY","STA","STX","BBS", /*80-8F*/
	"BCC","STA","???","TST","STY","STA","STX","???","TYA","STA","TXS","???","STZ","STA","STZ","BBS", /*90-9F*/
	"LDY","LDA","LDX","TST","LDY","LDA","LDX","???","TAY","LDA","TAX","???","LDY","LDA","LDX","BBS", /*A0-AF*/
	"BCS","LDA","???","TST","LDY","LDA","LDX","???","CLV","LDA","TSX","???","LDY","LDA","LDX","BBS", /*B0-BF*/
	"CPY","CMP","CLY","TDD","CPY","CMP","DEC","???","INY","CMP","DEX","???","CPY","CMP","DEC","BBS", /*C0-CF*/
	"BNE","CMP","???","TIN","CSH","CMP","DEC","???","CLD","CMP","???","???","???","CMP","DEC","BBS", /*D0-DF*/
	"CPX","SBC","???","TIA","CPX","SBC","INC","???","INX","SBC","NOP","???","CPX","SBC","INC","BBS", /*E0-EF*/
	"BEQ","SBC","???","TAI","???","SBC","INC","???","SED","SBC","???","???","???","SBC","INC","BBS"  /*F0-FF*/
};
uint8_t addrtable[256] = {
 /* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF         */
	im,ix,er,im,er,zp,zp,er,no,im,ac,er,er,ab,ab,bb, /*00-0f*/
	re,iy,er,im,er,zx,zx,er,no,ay,ac,er,er,ax,ax,bb, /*10-1f*/
	ab,ix,er,im,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,bb, /*20-2f*/
	re,iy,er,er,er,zx,zx,er,no,ay,ac,er,er,ax,ax,bb, /*30-3f*/
	no,ix,er,er,re,zp,zp,er,no,im,ac,er,ab,ab,ab,bb, /*40-4f*/
	re,iy,er,im,no,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*50-5f*/
	no,ix,no,er,er,zp,zp,er,no,im,ac,er,in,ab,ab,bb, /*60-6f*/
	re,iy,er,bt,er,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*70-7f*/
	re,ix,no,zp,zp,zp,zp,er,no,er,no,er,ab,ab,ab,bb, /*80-8f*/
	re,iy,er,ta,zx,zx,zy,er,no,ay,no,er,ab,ax,ax,bb, /*90-9f*/
	im,ix,im,zx,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*a0-af*/
	re,iy,er,ax,zx,zx,zy,er,no,ay,no,er,ax,ax,ay,bb, /*b0-bf*/
	im,ix,no,bt,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*c0-cf*/
	re,iy,er,bt,no,zx,zx,er,no,ay,er,er,er,ax,ax,bb, /*d0-df*/
	im,ix,er,bt,zp,zp,zp,er,no,im,no,er,ab,ab,ab,bb, /*e0-ef*/
	re,iy,er,bt,zx,zx,zx,er,no,ay,er,er,ax,ax,ax,bb  /*f0-ff*/
};

opcode_t curop;

opcode_t *findop(uint8_t op)
{
	curop.am = addrtable[op];
	curop.name = optable[op];
	return(&curop);
}

char *get_flags(char *buf)
{
	sprintf(buf, "----:----");

	if (P & C_FLAG) buf[8] = 'C';
	if (P & Z_FLAG) buf[7] = 'Z';
	if (P & I_FLAG) buf[6] = 'I';
	if (P & D_FLAG) buf[5] = 'D';
	if (P & B_FLAG) buf[3] = 'B';
	if (P & T_FLAG) buf[2] = 'T';
	if (P & V_FLAG) buf[1] = 'V';
	if (P & N_FLAG) buf[0] = 'N';
	return(buf);
}

uint32_t xlat(uint32_t n)
{
	uint32_t ret;

	ret = n & 0x1FFF;
	ret |= MPR[n >> 13] << 13;
	return(ret);
}

uint32_t Disasm(uint32_t dpc, char *outbuf, bool show_regs = false);

#define REG_A	0x01
#define REG_X	0x02
#define REG_Y	0x03
#define REG_S	0x04
#define REG_P	0x05
#define REG_PC	0x07
#define REG_MPR	0x08

uint32_t GetPC() { return(PC); }
uint32_t GetReg(int r) { 

	if ((r & ~7) == REG_MPR) {
		return(MPR[r & 7]);
	}

	switch (r) {
	case REG_A: return(A);
	case REG_X: return(X);
	case REG_Y: return(Y);
	case REG_S: return(S);
	case REG_P: return(P);
	case REG_PC:return(PC);
	}

	return(0xdeadbeef);
}

void Step();

uint64_t Cycle() {
	return(cycles);
}

};

#endif //PCEEMU_HUC6280_H
