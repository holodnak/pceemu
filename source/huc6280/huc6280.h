//
// Created by james on 3/2/2017.
//

#ifndef PCEEMU_HUC6280_H
#define PCEEMU_HUC6280_H

#define HUC6280_NMI		1
#define HUC6280_TIRQ	2
#define HUC6280_IRQ1	4
#define HUC6280_IRQ2	8

typedef uint8_t (*readfunc_t)(void*,uint32_t);
typedef void (*writefunc_t)(void*,uint32_t, uint8_t);

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

#endif //PCEEMU_HUC6280_H
