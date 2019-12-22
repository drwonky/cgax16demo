/*
 * cga16dmo.cpp
 *
 *  Created on: Dec 19, 2019
 *      Author: Perry Harrington
 *		License: MIT (do whatever you want with it)
 *
 *	This code is written to compile on Borland C++ 3.1 or later
 */

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include "xpm.h"

#define CRTCa 0x3d8	/* 6845 mode control register */
#define CRTCb 0x3d4 /* 6845 index register */
#define TEXT 0x03   /* DOS video mode for 80x25 text */

/* 
Simple helper routing to write to the index register, then the data register
The 6845 index and data registers are paired, the index register is the base
port address and the data register is one address higher, when writing data
to the 6845 you write the address of the register you want to change to the
index register, then you write the new data to the data register
*/
void write_crtc(unsigned int port, unsigned char reg, unsigned char val)
{
	outportb(port, reg);
	outportb(port+1,val);
}

/*
This routine automatically detects CGA and VGA cards, it sets the attributes
appropriately to enable 160x100 16 color text mode
*/
void x16mode(void)
{
	unsigned char status;

	_AL=(unsigned char) TEXT;
	_AH=0;
	geninterrupt(0x10);

	/* Read the input status register to reset the VGA attribute controller index/data state */
	status=inportb(0x3da);

	/* VGA attribute controller index register, mode register select */
	outportb(0x3c0,0x30);

	/* VGA attribute controller data register, read mode register */
	status=inportb(0x3c1);

	/* Check if VGA attribute controller register present */
	if (status != 0xFF) {  /* VGA */

		status&=0xf7;	/* turn off bit 3, blink */
		outportb(0x3c0,status);	/* write to VGA attribute controller data register */

		/* 
		   Write maximum scan line address and set to 4 lines
		   VGA has 8x16 character cells and pixels are square
		   To get 160x100 square pixels we need 4 lines by 8 pixels wide
		   VGA is 720x400 in text mode.
		*/
		write_crtc(CRTCb,0x09,0x03);

	} else { /* CGA does not have an attribute controller register, only mode controller */

		/* set mode control register for 80x25 text mode and disable video output */
		outportb(CRTCa, 1);

		/*
			These settings put the 6845 into "graphics" mode without actually
			switching the CGA controller into graphics mode.  The register
			values are directly copied from CGA graphics mode register
			settings.  The 6845 does not directly display graphics, the
			6845 only generates addresses and sync signals, the CGA
			attribute controller either displays character ROM data or color
			pixel data, this is external to the 6845 and keeps the CGA card
			in text mode.
			ref: HELPPC
		*/

		/* set vert total lines to 127 */
		write_crtc(CRTCb,0x04,0x7f);
		/* set vert displayed char rows to 100 */
		write_crtc(CRTCb,0x06,0x64);
		/* set vert sync position to 112 */
		write_crtc(CRTCb,0x07,0x70);
		/* set char scan line count to 1 */
		write_crtc(CRTCb,0x09,0x01);

		/* re-enable the video output in 80x25 text mode */
		outportb(CRTCa, 9);
	}
}

/*
simple helper to put video card back in standard 80x25 text mode
*/
void textmode(void)
{
	_AL=(unsigned char) TEXT;
	_AH=0;
	geninterrupt(0x10);
}

int main(void)
{
	unsigned char far *buffer;
	int i;
	unsigned char lfsr,bit;

	buffer = (unsigned char far *)MK_FP(0xb800,0);

	/* enable 160x100 16 color mode */
	x16mode();

	for (i=0;i<16000;i+=2) {
		/* Set the video memory to ▐ */
		buffer[i]=0xde;
		/* assign random color attribute to each cell */
		buffer[i+1]=(unsigned char)1+(rand()%254);
	}

	while(!kbhit()) {
		for (i=1;i<16000;i+=2) {
			/* 
				this lfsr implementation is much faster than calling rand() for each cell
				the downside of this implementation is that black cells remain black and
				the pattern repeats shortly.
			
				taps: 8 6 5 4; feedback polynomial: x^8 + x^6 + x^5 + x^4 + 1

				it's faster to read the far pointer, manipulate it locally, then write
			*/
			lfsr=buffer[i];

			if (lfsr == xpm[i/2]) continue;
			bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) /* & 1 */;
			lfsr = (lfsr >> 1) | (bit << 7);

			buffer[i]=lfsr;
		}
	}

	textmode();

	return 0;

}
