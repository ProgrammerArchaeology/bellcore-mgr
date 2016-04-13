#define DATA unsigned char

#include <mgr/bitblit.h>

extern DATA *graph_mem;

#define LOGBITS 3
#define BITS (~(~(unsigned)0<<LOGBITS))

#define bit_linesize(wide,depth) ((((depth)*(wide)+BITS)&~BITS)>>3)

#define BIT_SIZE(m) BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m))
#define BIT_Size(wide,high,depth) (((((depth)*(wide)+BITS)&~BITS)*(high))>>3)
#define BIT_LINE(x) ((((x)->primary->depth*(x)->primary->wide+BITS)&~BITS)>>LOGBITS)
/*{{{}}}*/


#include <sys/types.h>

extern void display_close(BITMAP *bitmap);

extern BITMAP *bit_expand(BITMAP *map,int fg,int bg);

extern void flip(DATA *s,int count);

extern int rop_invert(int op);

extern int mem_rop(BITMAP *dst_map,int x_dst,int y_dst,
				 int wide,int high,int op,
				 BITMAP *src_map,int x_src,int y_src);
