#include <stdio.h>
#include <stdint.h>

#define MASTER_CLOCK    21477270

void huc6280_reset();
void huc6280_step();

uint8_t rom[1024 * 1024];
uint8_t wram[8192];
uint32_t rommask;
size_t sz;

uint8_t read_mem(uint32_t addr)
{
    //rom reads
    if(addr < 0x100000) {
        addr &= rommask;
        return (rom[addr]);
    }

    if(addr >= 0x1F0000 && addr <= 0x1F7FFF) {
        return(wram[addr & 0x1FFF]);
    }

    printf("reading %08X (%X)\n",addr, addr & 0x1FFC00);
    switch(addr & 0x1FFC00) {
        case 0x1FF400:
            printf("int controller read\n");
            return(0);
        case 0x1FF000:
            printf("8-bit I/O port read\n");
            return(0);
        case 0x1FE800:
            printf("psg read\n");
            return(0);
        case 0x1FE400:
            printf("vce read\n");
            return(0);
        case 0x1FE000:
            printf("vdc read\n");
            return(0);
    }
}

uint8_t read_io(uint32_t addr)
{

}

void write_mem(uint32_t addr, uint8_t data)
{

}

void write_io(uint32_t addr, uint8_t data)
{

}

int main(int argc, char* argv[]) {
    char fn[] = "rom.pce";
    FILE *fp = fopen(fn,"rb");
    int i;

    fseek(fp,0,SEEK_END);
    sz = ftell(fp);
    fseek(fp,0,SEEK_SET);
    fread(rom,1,sz,fp);
    fclose(fp);
    rommask = sz - 1;
    printf("loaded rom, %d kb (size = %X, mask = %X)\n",sz / 1024, sz, rommask);

    huc6280_reset();

    for(i=0;i<10;i++)
        huc6280_step();

    return 0;
}
