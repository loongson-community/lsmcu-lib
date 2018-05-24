
#ifndef _STDIO_
#define _STDIO_

#include <stdarg.h>

#if __mips >= 3
#define	HAVE_QUAD	/* QUAD data type native */
#endif


#ifndef NULL
#define NULL    0
#endif

#define EOF  (-1)


int vsprintf (char *d, const char *s, va_list ap);
int printf (const char *fmt, ...);





#endif /* _STDIO_ */
