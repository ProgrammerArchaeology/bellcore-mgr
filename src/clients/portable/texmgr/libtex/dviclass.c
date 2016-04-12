/*
 * Copyright 1989 Chris Torek
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Chris Torek or M.I.T.
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  Chris
 * Torek and M.I.T. make no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * CHRIS TOREK AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS.  IN NO EVENT SHALL CHRIS TOREK OR M.I.T. BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * Original Author:
 * 	Chris Torek
 * 	Dept. of Computer Science
 * 	Univ. of Maryland
 * 	chris@cs.umd.edu
 */ 

#ifndef lint
static char rcsid[] = "$Header: /home/reed/grunwald/Projects/Iptex/lib/RCS/dviclass.c,v 1.3 89/02/13 14:30:57 grunwald Exp Locker: grunwald $";
#endif

/*
 * dviclass - DVI code classification tables.
 */

#include "dviclass.h"

/* shorthand---in lowercase for contrast (read on!) */
#define	four(x)		x, x, x, x
#define	six(x)		four(x), x, x
#define	sixteen(x)	four(x), four(x), four(x), four(x)
#define	sixty_four(x)	sixteen(x), sixteen(x), sixteen(x), sixteen(x)
#define	one_twenty_eight(x)	sixty_four(x), sixty_four(x)

/*
 * This table contains the byte length of the single operand, or DPL_NONE
 * if no operand, or if it cannot be decoded this way.
 *
 * The sequences UNS1, UNS2, UNS3, SGN4 (`SEQ_U') and SGN1, SGN2, SGN3,
 * SGN4 (`SEQ_S') are rather common, and so we define macros for these.
 */
#define	SEQ_U	DPL_UNS1, DPL_UNS2, DPL_UNS3, DPL_SGN4
#define	SEQ_S	DPL_SGN1, DPL_SGN2, DPL_SGN3, DPL_SGN4

char dvi_oplen[256] = {
	one_twenty_eight(DPL_NONE),
				/* characters 0 through 127 */
	SEQ_U,			/* DVI_SET1 through DVI_SET4 */
	DPL_NONE,		/* DVI_SETRULE */
	SEQ_U,			/* DVI_PUT1 through DVI_PUT4 */
	DPL_NONE,		/* DVI_PUTRULE */
	DPL_NONE,		/* DVI_NOP */
	DPL_NONE,		/* DVI_BOP */
	DPL_NONE,		/* DVI_EOP */
	DPL_NONE,		/* DVI_PUSH */
	DPL_NONE,		/* DVI_POP */
	SEQ_S,			/* DVI_RIGHT1 through DVI_RIGHT4 */
	DPL_NONE,		/* DVI_W0 */
	SEQ_S,			/* DVI_W1 through DVI_W4 */
	DPL_NONE,		/* DVI_X0 */
	SEQ_S,			/* DVI_X1 through DVI_X4 */
	SEQ_S,			/* DVI_DOWN1 through DVI_DOWN4 */
	DPL_NONE,		/* DVI_Y0 */
	SEQ_S,			/* DVI_Y1 through DVI_Y4 */
	DPL_NONE,		/* DVI_Z0 */
	SEQ_S,			/* DVI_Z1 through DVI_Z4 */
	sixty_four(DPL_NONE),	/* DVI_FNTNUM0 through DVI_FNTNUM63 */
	SEQ_U,			/* DVI_FNT1 through DVI_FNT4 */
	SEQ_U,			/* DVI_XXX1 through DVI_XXX4 */
	SEQ_U,			/* DVI_FNTDEF1 through DVI_FNTDEF4 */
	DPL_NONE,		/* DVI_PRE */
	DPL_NONE,		/* DVI_POST */
	DPL_NONE,		/* DVI_POSTPOST */
	six(DPL_NONE)		/* 250 through 255 */
};

char dvi_dt[256] = {
	one_twenty_eight(DT_CHAR),
				/* characters 0 through 127 */
	four(DT_SET),		/* DVI_SET1 through DVI_SET4 */
	DT_SETRULE,		/* DVI_SETRULE */
	four(DT_PUT),		/* DVI_PUT1 through DVI_PUT4 */
	DT_PUTRULE,		/* DVI_PUTRULE */
	DT_NOP,			/* DVI_NOP */
	DT_BOP,			/* DVI_BOP */
	DT_EOP,			/* DVI_EOP */
	DT_PUSH,		/* DVI_PUSH */
	DT_POP,			/* DVI_POP */
	four(DT_RIGHT),		/* DVI_RIGHT1 through DVI_RIGHT4 */
	DT_W0,			/* DVI_W0 */
	four(DT_W),		/* DVI_W1 through DVI_W4 */
	DT_X0,			/* DVI_X0 */
	four(DT_X),		/* DVI_X1 through DVI_X4 */
	four(DT_DOWN),		/* DVI_DOWN1 through DVI_DOWN4 */
	DT_Y0,			/* DVI_Y0 */
	four(DT_Y),		/* DVI_Y1 through DVI_Y4 */
	DT_Z0,			/* DVI_Z0 */
	four(DT_Z),		/* DVI_Z1 through DVI_Z4 */
	sixty_four(DT_FNTNUM),	/* DVI_FNTNUM0 through DVI_FNTNUM63 */
	four(DT_FNT),		/* DVI_FNT1 through DVI_FNT4 */
	four(DT_XXX),		/* DVI_XXX1 through DVI_XXX4 */
	four(DT_FNTDEF),	/* DVI_FNTDEF1 through DVI_FNTDEF4 */
	DT_PRE,			/* DVI_PRE */
	DT_POST,		/* DVI_POST */
	DT_POSTPOST,		/* DVI_POSTPOST */
	six(DT_UNDEF)		/* 250 through 255 */
};
