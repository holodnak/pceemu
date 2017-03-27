static uint32_t hsw_maxcycle;
static uint32_t hds_maxcycle;
static uint32_t hdw_maxcycle;
static uint32_t hde_maxcycle;

static uint32_t bgstart, renderstart, renderstop, pixel;
static uint32_t visiblestart, visiblestop, vblankstart;

/*

The VDC has more CPU vram access slots during active display, then what the cpu can saturate. But if there's a slight out of
alignment, the cpu will be stalled for a 'master clock' time slice. Of course, that's on the latch (MSB) and there's ~supposed~
to be a 1 word buffer as well. So to the programmer, this appears as unlimited access. The only real thing that can stall the
CPU, is during hblank. The VDC can use all access slots to fetch sprite pixel data (IIRC it lasts about 16+ vdc pixels; it's
relative to amount of sprites it's fetching for that scanline). If you touch 'vram' during this phase, the cpu will be
stalled/halted. This does not apply to VDC registers; you have unlimited access to those without stalling.

Burst mode is where the VDC is not fetching sprite data or bg/map data. You cannot forcibly put the VDC in this mode by
disabling sprites/bg layers; that disables the output but not the internal fetching process. And, IIRC, burst mode is the 
only area in the display(frame) where you can request vram-vram DMA (and satb-dma).

*/
static void update_display_registers()
{
	//update the horizontal and veritcal sync registers from the register bank
	hds = ((HSR >> 8) & 0x7F) + 1;
	hsw = ((HSR >> 0) & 0x1F) + 1;
	hde = ((HDR >> 8) & 0x7F) + 1;
	hdw = ((HDR >> 0) & 0x7F) + 1;

	vds = ((VPR >> 8) & 0xFF) + 2;
	vsw = ((VPR >> 0) & 0x1F) + 1;
	vdw = (VDW & 0x1FF) + 1;
	vcr = VCR & 0xFF;

	bxr = BXR;
	byr = BYR;

	//compute lengths of each 'state'
	bgstart = (hsw + 1) * 8;
	renderstart = (hsw + hds) * 8;
	renderstop = (hsw + hds + hdw) * 8;

	visiblestart = vds;
	visiblestop = vds + vdw;
	vblankstart = vds + vdw + vcr;

	//internal counters
	pixel = 0;

}

//increment the cycle counter
static void increment_cycle()
{
	cycle++;
	if (cycle >= CYCLES_PER_LINE) {
		cycle = 0;

//		printf("scanline incrementing: %d (%d cpl)\n", scanline, CYCLES_PER_LINE);
		scanline++;
		if (scanline >= LINES_PER_FRAME) {
			scanline = 0;
			frame++;
		}
	}
}
