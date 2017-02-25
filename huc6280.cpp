#include <stdio.h>
#include <stdint.h>

static uint32_t clocklen;
static uint32_t cycles;

static uint16_t pc, ea;
static uint8_t a, x, y, s, p;
static uint8_t mpr[8], mpr_buf;

static uint8_t flag_n;
static uint8_t flag_z;
static uint8_t flag_v;
static uint8_t flag_c;
static uint8_t flag_i;
static uint8_t flag_t;
static uint8_t flag_d;

static uint8_t tmpreg;
static uint8_t opcode;

uint8_t read_mem(uint32_t addr);
uint8_t read_io(uint32_t addr);
void write_mem(uint32_t addr, uint8_t data);
void write_io(uint32_t addr, uint8_t data);

static void clocktick()
{
    cycles += clocklen;
}

static uint8_t memread(uint32_t addr)
{
//    printf("reading mem %04X\n", (mpr[addr >> 13] << 13) |  (addr & 0x1FFF));

    clocktick();
    tmpreg = read_mem((mpr[addr >> 13] << 13) |  (addr & 0x1FFF));

    return(tmpreg);
}

static uint8_t memreadzp(uint32_t addr)
{
//    printf("reading mem %04X\n", (mpr[addr >> 13] << 13) |  (addr & 0x1FFF));

    clocktick();
    tmpreg = read_mem((mpr[addr >> 13] << 13) |  (addr & 0x1FFF));

    return(tmpreg);
}

static void memwrite(uint32_t addr, uint8_t data)
{
    printf("writing mem %04X=%02X\n", (mpr[addr >> 13] << 13) |  (addr & 0x1FFF), data);

    clocktick();
    write_mem((mpr[addr >> 13] << 13) |  (addr & 0x1FFF), data);
}


static void expand_flags()
{
    flag_c = (p & 0x01) >> 0;
    flag_z = (p & 0x02) >> 1;
    flag_i = (p & 0x04) >> 2;
    flag_d = (p & 0x08) >> 3;
    flag_v = (p & 0x40) >> 6;
    flag_n = (p & 0x80) >> 7;
}

//check value for n/z and set flags
static void checknz(uint8_t n)
{
    flag_n = (n >> 7) & 1;
    flag_z = (n == 0) ? 1 : 0;
}

//implied addressing
static void am_imp()
{
    ea = pc;

    memread(pc);
}

static void am_imm()
{
    ea = pc++;

    memread(ea);
}

static void am_abs()
{
    ea = memread(pc++);
    ea |= memread(pc++) << 8;

    memread(ea);
}

static void op_csl()    {   clocklen = 12;  memread(pc);    }
static void op_csh()    {   clocklen = 3;   memread(pc);    }

static void op_set()    {   flag_t = 1;                     }
static void op_sec()    {   flag_t = 0;     flag_c = 1;     }
static void op_sed()    {   flag_t = 0;     flag_d = 1;     }
static void op_sei()    {   flag_t = 0;     flag_i = 1;     }
static void op_cli()    {   flag_t = 0;     flag_i = 0;     }

static void op_lda()    {   a = tmpreg;    }
static void op_ldx()    {   x = tmpreg;    }
static void op_ldy()    {   y = tmpreg;    }

static void op_and() {
    if(flag_t) {
        uint8_t t = memread()
        //TODO
    }
    else {
        a &= tmpreg;
        checknz(a);
    }
}

static void op_tam() {
    int i;

    clocktick();
    clocktick();
    clocktick();
    if(tmpreg) {
        mpr_buf = a;
        for(i=0;i<8;i++) {
            if(tmpreg & (1 << i))
                mpr[i] = a;
        }
    }
}

void huc6280_disasm()
{

}

void huc6280_reset()
{
    cycles = 0;
    clocklen = 3;
    pc = 0;

    clocktick();
    clocktick();
    clocktick(); s--;
    clocktick(); s--;
    clocktick(); s--;
    clocktick(); pc = memread(0xFFFE);
    clocktick(); pc |= memread(0xFFFF) << 8;

    clocktick();

    flag_i = 1;

    flag_n = 0;
    flag_z = 0;
    flag_v = 0;
    flag_c = 0;

    mpr[7] = 0;
}

void huc6280_step()
{
    uint32_t ncycles = cycles;

    opcode = memread(pc++);
    switch(opcode) {
        case 0x29: am_imm();    op_and();    break;
        case 0x38: am_imp();    op_sec();    break;
        case 0x53: am_imm();    op_tam();    break;
        case 0x54: am_imp();    op_csl();    break;
        case 0x58: am_imp();    op_cli();    break;
        case 0x78: am_imp();    op_sei();    break;
        case 0xA9: am_imm();    op_lda();    break;
        case 0xAD: am_abs();    op_lda();    break;
        case 0xD4: am_imp();    op_csh();    break;
        case 0xF4: am_imp();    op_set();    break;
        case 0xF8: am_imp();    op_sed();    break;
        default:
            printf("invalid opcode: %02X\n", opcode);
    }
    printf("opcode %02X: %d cycles\n", opcode, cycles - ncycles);
}
