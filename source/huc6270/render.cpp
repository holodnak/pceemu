static void decode_tile_line(uint8_t *buf, uint16_t b, uint16_t c01, uint16_t c23)
{
	uint8_t p;
	int i;

	for (i = 0; i < 8; i++) {
		p = (b >> 8) & 0xF0;
		if (c01 & (1 << (7 - i)))	p |= 1;
		if (c01 & (1 << (15 - i)))	p |= 2;
		if (c23 & (1 << (7 - i)))	p |= 4;
		if (c23 & (1 << (15 - i)))  p |= 8;
		buf[i] = p;
	}
}

void CHuc6270::RenderBackgroundLine()
{
	int vscanline = scanline - visiblestart;
	uint8_t *lineptr = bgLineBuffer;
	uint16_t bat, c01, c23;

	int t, xoff, yoff, offs;

	//	printf("line %d, drawing %d tiles\n", vscanline, hdw);

	xoff = (bxr >> 3) & (screenw - 1);
	yoff = ((byr + vscanline) >> 3) & (screenh - 1);

	for (t = 0; t < (hdw + 1); t++) {

		bat = vram[xoff + (yoff * screenw)];

		offs = (bat & 0xFFF) * 16;
		offs += (byr + vscanline) & 7;

		c01 = vram[offs + 0];
		c23 = vram[offs + 8];

		decode_tile_line(lineptr, bat, c01, c23);

		lineptr += 8;
		xoff = (xoff + 1) & (screenw - 1);
	}
}

void CHuc6270::RenderSpriteLine()
{
	int i, n, x, y, pataddr, attrib;
	uint16_t *satbufptr = satbuf + (15 * 4);
	int vscanline = scanline - visiblestart;
	uint16_t spr_ch0, spr_ch1, spr_ch2, spr_ch3, p;
	int sprwidth, sprheight;

	if ((CR & 0x40) == 0) //sprites disabled
		return;

	if (vscanline < 0)
		return;

	memset(sprLineBuffer, 0, 1024);

	for (n = 15; n >= 0; n--) {
		int tbl_width[2] = { 16, 32 };
		int tbl_height[4] = { 16, 32, 16, 64 };

		int hoffs;

		y = (satbufptr[0] & 0x3FF) - 64;
		x = (satbufptr[1] & 0x3FF) - 32;
		pataddr = satbufptr[2] & 0x7FF;
		attrib = satbufptr[3];
		sprwidth = tbl_width[(attrib >> 8) & 1];
		sprheight = tbl_height[(attrib >> 12) & 3];
		hoffs = vscanline - y;

		if (attrib & 0x8000) {
			if (sprheight == 16)
				hoffs = 15 - hoffs;
			else if (sprheight == 32)
				hoffs = 31 - hoffs;
			else if (sprheight == 64)
				hoffs = 63 - hoffs;
		}

		pataddr >>= 1;
		if (sprwidth == 32) {
			pataddr &= ~1;
		}

		if (sprheight == 16) {
			if (attrib & 0x8000)
				hoffs = 15 - hoffs;
		}

		else if (sprheight == 32) {
			if (attrib & 0x8000)
				hoffs = 31 - hoffs;
			pataddr &= ~2;
			if (hoffs >= 16)
				pataddr |= 2;
		}

		else if (sprheight == 64) {
			if (attrib & 0x8000)
				hoffs = 63 - hoffs;
			pataddr &= ~6;
			if (hoffs >= 48)
				pataddr |= 6;
			else if (hoffs >= 32)
				pataddr |= 4;
			else if (hoffs >= 16)
				pataddr |= 2;
		}

		pataddr = (pataddr << 6) + (hoffs & 0xF);

		if (y < 0 /*|| x < 0*/ || x >= (hdw * 8)) {
			satbufptr -= 4;
			continue;
		}

		if (attrib & 0x0800 && sprwidth == 32) {
			pataddr += 64;
		}

		//	printf("drawing sprite %d at (%d, %d) (curline: %d)\n", n, x, y, vscanline);

		//draw tile line to linebuffer
		for (i = 0; i < sprwidth; i++) {
			if ((i & 0xF) == 0) {
				if (attrib & 0x0800) {
					spr_ch0 = vram[pataddr];
					spr_ch1 = vram[pataddr + 16];
					spr_ch2 = vram[pataddr + 32];
					spr_ch3 = vram[pataddr + 48];
					pataddr -= 64;
				}
				else {
					spr_ch0 = vram[pataddr];
					spr_ch1 = vram[pataddr + 16];
					spr_ch2 = vram[pataddr + 32];
					spr_ch3 = vram[pataddr + 48];
					pataddr += 64;
				}
			}
			p = 0;
			if (attrib & 0x0800) {
				if (spr_ch0 & (1 << ((i & 0xF))))	p |= 1;
				if (spr_ch1 & (1 << ((i & 0xF))))	p |= 2;
				if (spr_ch2 & (1 << ((i & 0xF))))	p |= 4;
				if (spr_ch3 & (1 << ((i & 0xF))))	p |= 8;
			}
			else {
				if (spr_ch0 & (1 << (15 - (i & 0xF))))	p |= 1;
				if (spr_ch1 & (1 << (15 - (i & 0xF))))	p |= 2;
				if (spr_ch2 & (1 << (15 - (i & 0xF))))	p |= 4;
				if (spr_ch3 & (1 << (15 - (i & 0xF))))	p |= 8;
			}
			if (p) {
				int offs = x + i;

				if (offs >= 0 && offs < 256) {

					//check for sprite0
					if (attrib & 0x10) {

						//check for pixel already here
//						if (sprLineBuffer[offs]) {
						{
							if (CR & 1) {
								pce->IntCtrl()->SetIrq(INT_IRQ1);
								printf("sp0 hit irq\n");
							}

							status |= 1;
						}
					}

					sprLineBuffer[offs] = (uint8_t)(p | ((attrib & 0xF) << 4));
					//					linebuffer[offs] = sprlinebuffer[offs];
				}
			}
		}

		satbufptr -= 4;
	}

}

