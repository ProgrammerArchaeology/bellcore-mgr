#define DATA unsigned char

#include <mgr/bitblit.h>

#define LOGBITS 3
#define BITS (~(~(unsigned)0 << LOGBITS))

#define bit_linesize(wide, depth) ((((depth) * (wide) + BITS) & ~BITS) >> 3)

#define BIT_LINE(x) ((((x)->primary->depth * (x)->primary->wide + BITS) & ~BITS) >> LOGBITS)

#define BIT_Size(wide,high,depth) (((((depth)*(wide)+BITS)&~BITS)*(high))>>3)
