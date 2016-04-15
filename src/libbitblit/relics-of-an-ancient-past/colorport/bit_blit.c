/*{{{}}}*/
/*{{{  Notes*/
/*  8-bit color version of portable bit-blt code.  (S. A. Uhler)
 */
/*}}}  */
/*{{{  #includes*/
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "screen.h"
/*}}}  */
/*{{{  #defines*/
#define dprintf	if(bit_debug)fprintf
/*}}}  */

/*{{{  variables*/
int bit_debug;
/*}}}  */

BITMAP *bit_expand(BITMAP *map,int fg,int bg);


/*{{{  bit_blit -- map bit_blits into mem_rops, caching 8 bit images as needed*/
void
bit_blit(dst_map,x_dst,y_dst,wide,high,op,src_map,x_src,y_src)
BITMAP *dst_map;				/* source bitmap */
BITMAP *src_map;				/* destination bitmap */
int x_dst,y_dst;				/* destination coords */
int x_src,y_src;				/* source coords */
int wide,high;					/* bitmap size */
int op;							/* bitmap function */
	{
	if (!src_map || src_map->depth == dst_map->depth) {	/* normal bit-blit */
		mem_rop(dst_map,x_dst,y_dst,wide,high,op,src_map,x_src,y_src);
		}
	else if (src_map->depth==1) {			/* expand src to 8 bits */
		int fg =  GETFCOLOR(op);
		int bg =  GETBCOLOR(op);
		BITMAP *temp;

		/* try to protect the users from themselves */
		/* if (fg == bg) bg = 0xFF^fg; */

		/* cached image is invalid, kill it */

		if (BIT_CACHE(src_map) && 0xffff&op>>4 != BIT_CHCLR(src_map)) {
			dprintf(stderr,"kill cache %d x %d\r\n",
					src_map->primary->wide, src_map->primary->high);
			bit_destroy(BIT_CACHE(src_map));
			BIT_CACHE(src_map) = NULL;
			}

		/* make an 8 bit image cache */

		if (!BIT_CACHE(src_map)) {
			dprintf(stderr," C(%d x %d)",
					src_map->primary->wide, src_map->primary->high);
			if (bit_debug) fflush(stderr);
			temp = bit_expand(src_map->primary,fg,bg);
			dprintf(stderr," done\r\n");
			BIT_CACHE(src_map) = temp;
			BIT_CHCLR(src_map) = op>>4 & 0xffff;
			}
		mem_rop(dst_map,x_dst,y_dst,wide,high,op,BIT_CACHE(src_map),
				BIT_X(src_map)+x_src,BIT_Y(src_map)+y_src);
		}

	/* shrink a bitmap, then do the blit.  NEVER USED! (well almost never) */

	else {	
		BITMAP *small;
		small = bit_shrink(src_map,GETBCOLOR(op));
		/* fprintf(stderr,"shrinking %d x %d\n",dst_map->wide, dst_map->high); */
		mem_rop(dst_map,x_dst,y_dst,wide,high,op,small,
				BIT_X(src_map)+x_src,BIT_Y(src_map)+y_src);
		bit_destroy(small);
		}
	if( BIT_CACHE( dst_map))
		SET_DIRTY( dst_map);	/* need this for bit_point, etc, too */
      }
/*}}}  */
/*{{{  flip -- flip the bit order on count elements of s*/
static unsigned char flp[256] = {
	0x00,	0x80,	0x40,	0xc0,	0x20,	0xa0,	0x60,	0xe0,
	0x10,	0x90,	0x50,	0xd0,	0x30,	0xb0,	0x70,	0xf0,
	0x08,	0x88,	0x48,	0xc8,	0x28,	0xa8,	0x68,	0xe8,
	0x18,	0x98,	0x58,	0xd8,	0x38,	0xb8,	0x78,	0xf8,
	0x04,	0x84,	0x44,	0xc4,	0x24,	0xa4,	0x64,	0xe4,
	0x14,	0x94,	0x54,	0xd4,	0x34,	0xb4,	0x74,	0xf4,
	0x0c,	0x8c,	0x4c,	0xcc,	0x2c,	0xac,	0x6c,	0xec,
	0x1c,	0x9c,	0x5c,	0xdc,	0x3c,	0xbc,	0x7c,	0xfc,
	0x02,	0x82,	0x42,	0xc2,	0x22,	0xa2,	0x62,	0xe2,
	0x12,	0x92,	0x52,	0xd2,	0x32,	0xb2,	0x72,	0xf2,
	0x0a,	0x8a,	0x4a,	0xca,	0x2a,	0xaa,	0x6a,	0xea,
	0x1a,	0x9a,	0x5a,	0xda,	0x3a,	0xba,	0x7a,	0xfa,
	0x06,	0x86,	0x46,	0xc6,	0x26,	0xa6,	0x66,	0xe6,
	0x16,	0x96,	0x56,	0xd6,	0x36,	0xb6,	0x76,	0xf6,
	0x0e,	0x8e,	0x4e,	0xce,	0x2e,	0xae,	0x6e,	0xee,
	0x1e,	0x9e,	0x5e,	0xde,	0x3e,	0xbe,	0x7e,	0xfe,
	0x01,	0x81,	0x41,	0xc1,	0x21,	0xa1,	0x61,	0xe1,
	0x11,	0x91,	0x51,	0xd1,	0x31,	0xb1,	0x71,	0xf1,
	0x09,	0x89,	0x49,	0xc9,	0x29,	0xa9,	0x69,	0xe9,
	0x19,	0x99,	0x59,	0xd9,	0x39,	0xb9,	0x79,	0xf9,
	0x05,	0x85,	0x45,	0xc5,	0x25,	0xa5,	0x65,	0xe5,
	0x15,	0x95,	0x55,	0xd5,	0x35,	0xb5,	0x75,	0xf5,
	0x0d,	0x8d,	0x4d,	0xcd,	0x2d,	0xad,	0x6d,	0xed,
	0x1d,	0x9d,	0x5d,	0xdd,	0x3d,	0xbd,	0x7d,	0xfd,
	0x03,	0x83,	0x43,	0xc3,	0x23,	0xa3,	0x63,	0xe3,
	0x13,	0x93,	0x53,	0xd3,	0x33,	0xb3,	0x73,	0xf3,
	0x0b,	0x8b,	0x4b,	0xcb,	0x2b,	0xab,	0x6b,	0xeb,
	0x1b,	0x9b,	0x5b,	0xdb,	0x3b,	0xbb,	0x7b,	0xfb,
	0x07,	0x87,	0x47,	0xc7,	0x27,	0xa7,	0x67,	0xe7,
	0x17,	0x97,	0x57,	0xd7,	0x37,	0xb7,	0x77,	0xf7,
	0x0f,	0x8f,	0x4f,	0xcf,	0x2f,	0xaf,	0x6f,	0xef,
	0x1f,	0x9f,	0x5f,	0xdf,	0x3f,	0xbf,	0x7f,	0xff,
	};

/*
 * Convert from external to internal bitmap format.  Internal format
 * is SUN-3 bit order, 1=black, 0=white.  This sample flip routine is
 * for the DEC3100, where 1=white, 0=black, and the bits in every byte
 * are reversed
 */

void
flip(s,count)
register DATA *s;
register int count;
	{
	while (count-- > 0) 
		*s++ = ~((flp[*s&0xff]) | (flp[*s>>8&0xff]<<8) |
				 (flp[*s>>16&0xff]<<16) | (flp[*s>>24&0xff]<<24));
	}
/*}}}  */
/*{{{  bit_expand -- expand a 1 bit per pixel primary bitmap to 8 bits per pixel*/
/* implementation very suboptimal for case of 8-bit DATA */
static int masks[] = {
	0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff, 
	0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff, 
	0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff, 
	0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff, 
	};

BITMAP *
bit_expand(map,fg,bg)
BITMAP *map;		/* bitmap to expand */
int fg,bg;			/* foreground and background colors */
	{
	register DATA fg_mask = fg;
	register DATA bg_mask = bg;
	register BITMAP *new;
	register DATA *src;
	register DATA *dst;
	register DATA s;
	DATA maps[16];
	register int i;
	register int h;
	register int count;

	if (!map) {
		fprintf(stderr,"map failed in expand\r\n");
		exit(1);
		}
	new = bit_alloc(BIT_WIDE(map),BIT_HIGH(map),NULL,8);

	if (!new) {
		fprintf(stderr,"bit_alloc failed in expand\r\n");
		exit(1);
		}

	/* initialize the map table */

#if LOGBITS > 3
	fg_mask |= fg_mask<<8;
	bg_mask |= bg_mask<<8;
#endif
#if LOGBITS > 4
	fg_mask |= fg_mask<<16;
	bg_mask |= bg_mask<<16;
#endif
	for(i=0;i<16;i++) {
		maps[i] = (fg_mask&masks[i]) | (bg_mask&~masks[i]);
		}

	/* expand the data (stupid, but it works) */

	src = BIT_DATA(map);	/* pointer to 1 bit per pixel data */
	dst = BIT_DATA(new);	/* pointer to 8 bit per pixel data */
	h = BIT_HIGH(map);	/* number of lines */
	count = BIT_LINE(new);	/* # of DATA's per line */

#define MM ((1<<(1<<(LOGBITS-3)))-1)
#define SD (sizeof(DATA)/sizeof(char))

	while(h-- > 0) {
		for(i=0;i<count;i++) {
			switch(i&7) {
				case 0: s = *src++;
					*dst++ = maps[GETLSB(s,7*SD)&MM]; break;
				case 1: *dst++ = maps[GETLSB(s,6*SD)&MM]; break;
				case 2: *dst++ = maps[GETLSB(s,5*SD)&MM]; break;
				case 3: *dst++ = maps[GETLSB(s,4*SD)&MM]; break;
				case 4: *dst++ = maps[GETLSB(s,3*SD)&MM]; break;
				case 5: *dst++ = maps[GETLSB(s,2*SD)&MM]; break;
				case 6: *dst++ = maps[GETLSB(s,1*SD)&MM]; break;
				case 7: *dst++ = maps[GETLSB(s,0*SD)&MM]; break;
				}
			}
		}
		
/*	too bad this one won't work 
	for(;s = *src, src++ < end; dst += 8)
		for(i=7; i>=0; i--, s>>=4)
			dst[i] = maps[s&0xf];
*/
	
	return(new);	
	}
/*}}}  */
/*{{{  bit_shrink -- shrink an 8-bit bitmap into a 1 bit bitmap*/
/* shrink an 8-bit bitmap into a 1 bit bitmap */
/* only works for primary bitmaps for now */
/* assumes 32 bit data, 8 bits per pixel */

BITMAP *
bit_shrink(src_map,bg_color)
BITMAP *src_map;	/* bitmap to shrink  - must be a primary bitmap */
int bg_color;		/* color to use as background - all else is on! */
	{
	BITMAP *map = src_map-> primary;
	BITMAP *result = bit_alloc(BIT_WIDE(map),BIT_HIGH(map),NULL,1);
	DATA *src = BIT_DATA(map);
	DATA *dst = BIT_DATA(result);
	DATA *end = src + BIT_LINE(map)*BIT_HIGH(map);
#ifdef BANKED
	DATA *bsrc = src;	/* bsrc is banked addr, src the flat address */
	void *lastinbank = end;	/* no interruption if not screen bitmap */
#else
	DATA initsbits = ~GETLSB((DATA)~0,8);	/* most sig pixel  (8 bits) */
	DATA sbits = initsbits;
#endif
	DATA initdbits = ~GETLSB((DATA)~0,1);	/* most sig pixel (1 bit) */
	DATA dbits = initdbits;
	DATA color = bg_color & 0xff;

#if LOGBITS > 3
	color |= (color<<8);
#endif
#if LOGBITS > 4
	color |= (color<<16);
#endif

#ifdef BANKED
	if (IS_SCREEN(map))
		bsrc = get_addr_fb( src, BIT_DATA(map), &lastinbank);
#endif
	
	memset(dst,0,BIT_SIZE(result));
	while(src<end) {
#ifdef BANKED
		if (*bsrc != color)          *dst |= dbits;
#else
		if ((*src ^ color) & sbits)  *dst |= dbits;
#endif
		if (!(dbits=GETLSB(dbits,1))) {
			dbits = initdbits;
			dst++;
			}
#ifdef BANKED
		if (++src > (DATA *)lastinbank)
			bsrc = get_addr_fb( src, BIT_DATA(map), &lastinbank);
		else
			bsrc += 1;
#else
		if (!(sbits=GETLSB(sbits,8))) {
			sbits = initsbits;
			src++;
			}
#endif
		}
	return(result);
	}
/*}}}  */
/*{{{  rop_invert -- invert a raster op, flipping the sense of white and black*/
int
rop_invert(op)
register int op;
	{
	register int result;
	
	switch (OPCODE(op)) {

		/* these cases are unaffected by SRC */

		case OPCODE(~0):
		case OPCODE(0):
		case OPCODE(~DST):
		case OPCODE(DST):
			result = OPCODE(op);
			break;

		case OPCODE(SRC):
			result = OPCODE(~SRC);
			break;
		case OPCODE(~SRC):
			result = OPCODE(SRC);
			break;

		case OPCODE(~SRC | DST):
			result = OPCODE(SRC | DST);
			break;
		case OPCODE(SRC | DST):
			result = OPCODE(~SRC | DST);
			break;

		case OPCODE(SRC | ~DST):
			result = OPCODE(~SRC | ~DST);
			break;
		case OPCODE(~SRC | ~DST):
			result = OPCODE(SRC | ~DST);
			break;

		case OPCODE(DST & SRC):
			result = OPCODE(DST & ~SRC);
			break;
		case OPCODE(DST & ~SRC):
			result = OPCODE(DST & SRC);
			break;

		case OPCODE(~DST & SRC):
			result = OPCODE(~DST & ~SRC);
			break;
		case OPCODE(~DST & ~SRC):
			result = OPCODE(~DST & SRC);
			break;

		case OPCODE(SRC ^ DST):
			result = OPCODE(~SRC ^ DST);
			break;
		case OPCODE(~SRC ^ DST):
			result = OPCODE(SRC ^ DST);
			break;
		default:
			result = 0;		/* impossible */
		}
	return (result | (op&~0xf));
	}
/*}}}  */
