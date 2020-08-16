/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/
/* $Id: tf.h,v 35004.59 2007/01/13 23:12:39 kkeys Exp $ */

#ifndef TF_H
#define TF_H

#include <stdint.h>

#ifndef NCOLORS
# define NCOLORS 16
#endif

typedef uint64_t attr_t;
typedef uint64_t cattr_t;

/* headers needed everywhere */
#include <time.h>	/* may conflict with <sys/time.h> on some systems? */
#include <sys/time.h>	/* for struct timeval */
#include "malloc.h"
#include "dstring.h"
#include "globals.h"

#if SOCKS
# if (SOCKS == 4)
#  define connect Rconnect
#  define select Rselect
# else /* (SOCKS >= 5) */
#  include <socks.h>
# endif
#endif


/*
 * TinyFugue global types and variables.
 */

typedef struct Program Program;
typedef struct Screen Screen;
typedef struct Pattern Pattern;
typedef struct Macro Macro;
typedef struct World World;

#define RESTRICT_SHELL  1
#define RESTRICT_FILE   2
#define RESTRICT_WORLD  3

typedef char smallstr[65];     /* Short buffer */

enum enum_attr {
    /* inside the 16 low bits */
    F_UNDERLINE   = 0x0001,
    F_REVERSE     = 0x0002,
    F_FLASH       = 0x0000,   /* zero - not implemented */
    F_DIM         = 0x0000,   /* zero - not implemented */
    F_BOLD        = 0x0004,
    F_ITALIC      = 0x0008,
    F_HILITE      = 0x0010,
    F_NONE        = 0x0020,
    F_EXCLUSIVE   = 0x0040,

#if NCOLORS == 256 /* XXX ??? */
# define FGCOLORSHIFT 8
    F_FGCOLORMASK = 0x0000ff00,   /* 8 bits, interpreted as an integer */
    F_FGCOLOR     = 0x00000080,   /* flag */
# define BGCOLORSHIFT 16
    F_BGCOLORMASK = 0x00ff0000,   /* 8 bits, interpreted as an integer */
    F_BGCOLOR     = 0x00000100,   /* flag */
#else
    /* inside the 16 low bits */
# define FGCOLORSHIFT 8
    F_FGCOLORMASK = 0x0f00,   /* 4 bits, interpreted as an integer */
    F_FGCOLOR     = 0x0080,   /* flag */
# define BGCOLORSHIFT 12
    F_BGCOLORMASK = 0x7000,   /* 3 bits, interpreted as an integer */
    F_BGCOLOR     = 0x0100,   /* flag */
#endif

    /* outside the 16 low bits */
    F_NOACTIVITY  = 0x01000000,	    /* does not count as activity */
    F_NOLOG       = 0x02000000,
    F_BELL        = 0x04000000,
    F_GAG         = 0x08000000,
    F_NOHISTORY   = 0x10000000,

    F_TFPROMPT    = 0x20000000,	    /* is a prompt generated by tf */
    F_SERVPROMPT  = 0x40000000,	    /* is a prompt from server */

    F_FGCOLORS    = (F_FGCOLOR | F_FGCOLORMASK),
    F_BGCOLORS    = (F_BGCOLOR | F_BGCOLORMASK),
    F_COLORS      = (F_FGCOLORS | F_BGCOLORS),
    F_SIMPLE      = (F_UNDERLINE | F_REVERSE | F_FLASH | F_DIM | F_BOLD | F_ITALIC),
    F_HWRITE      = (F_SIMPLE | F_HILITE | F_COLORS),
    F_ENCODE      = (F_SIMPLE | F_HILITE | F_FGCOLOR | F_BGCOLOR),
    F_ATTR        = (F_HWRITE | F_GAG | F_NOHISTORY | F_NOACTIVITY | F_NONE |
		    F_EXCLUSIVE)
};

#define attr2fgcolor(attr)    (((attr) & F_FGCOLORMASK) >> FGCOLORSHIFT)
#define attr2bgcolor(attr)    (((attr) & F_BGCOLORMASK) >> BGCOLORSHIFT)
#define fgcolor2attr(color)   (F_FGCOLOR | ((color) << FGCOLORSHIFT))
#define bgcolor2attr(color)   (F_BGCOLOR | ((color) << BGCOLORSHIFT))

# define attr2cattr(attr)    ((cattr_t)(attr & F_HWRITE))

extern attr_t adj_attr(attr_t base, attr_t adj);


/* Macros for defining and manipulating bit vectors of arbitrary length.
 * We use an array of long because select() does, and these macros will be
 * used with select() on systems without the FD_* macros.
 */

#ifndef NBBY
# define NBBY 8                                   /* bits per byte */
#endif
#ifndef LONGBITS
# define LONGBITS  (sizeof(long) * NBBY)          /* bits per long */
#endif

#define VEC_TYPEDEF(type, size) \
    typedef struct { \
        unsigned long bits[(((size) + LONGBITS - 1) / LONGBITS)]; \
    } (type)

#define VEC_SET(n,p)   ((p)->bits[(n)/LONGBITS] |= (1L << ((n) % LONGBITS)))
#define VEC_CLR(n,p)   ((p)->bits[(n)/LONGBITS] &= ~(1L << ((n) % LONGBITS)))
#define VEC_ISSET(n,p) ((p)->bits[(n)/LONGBITS] & (1L << ((n) % LONGBITS)))
#if HAVE_MEMCPY   /* assume memcpy implies memset and bcopy implies bzero. */
# define VEC_ZERO(p)   memset((char *)(p)->bits, '\0', sizeof(*(p)))
#else
# define VEC_ZERO(p)   bzero((char *)(p)->bits, sizeof(*(p)))
#endif


/* Define enumerated constants */
#define ENUMEXTERN extern
#define bicode(a, b)  a 
#include "enumlist.h"

extern conString enum_off[];
extern conString enum_flag[];
extern conString enum_sub[];
extern conString enum_color[];

/* hook definitions */

extern int do_hook(int indx, const char *fmt, const char *argfmt, ...)
    format_printf(2, 4);

enum Hooks {
#define gencode(id, type)  H_##id 
#include "hooklist.h"
#undef gencode
    NUM_HOOKS
};

VEC_TYPEDEF(hookvec_t, NUM_HOOKS);


/* externs */
extern const char version[], sysname[], copyright[], contrib[], mods[];
extern int restriction, debug;
extern void internal_error(const char *file, int line,
    const char *fmt, ...) format_printf(3, 4);
extern void internal_error2(const char *file, int line, const char *file2,
    int line2, const char *fmt, ...) format_printf(5, 6);


#endif /* TF_H */
