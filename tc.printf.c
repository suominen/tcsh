/* $Header: /u/christos/src/tcsh-6.01/RCS/tc.printf.c,v 3.5 1992/03/21 02:46:07 christos Exp $ */
/*
 * tc.printf.c: A public-domain, minimal printf/sprintf routine that prints
 *	       through the putchar() routine.  Feel free to use for
 *	       anything...  -- 7/17/87 Paul Placeway
 */
/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "sh.h"

RCSID("$Id: tc.printf.c,v 3.5 1992/03/21 02:46:07 christos Exp $")

#ifdef lint
#undef va_arg
#define va_arg(a, b) (a ? (b) 0 : (b) 0)
#endif

#define INF	32766		/* should be bigger than any field to print */

static char buf[128];

static	void	xaddchar	__P((int));
static	void	doprnt		__P((void (*) __P((int)), char *, va_list));

static void
doprnt(addchar, sfmt, ap)
    void    (*addchar)();
    char   *sfmt;
    va_list ap;
{
    register char *f, *bp;
    register long l;
    register unsigned long u;
    register int i;
    register int fmt;
    register unsigned char pad = ' ';
    int     flush_left = 0, f_width = 0, prec = INF, hash = 0, do_long = 0;
    int     sign = 0;
    int     attributes = 0;


    f = sfmt;
    for (; *f; f++) {
	if (*f != '%') {	/* then just out the char */
	    (*addchar) ((int) (((unsigned char)*f) | attributes));
	}
	else {
	    f++;		/* skip the % */

	    if (*f == '-') {	/* minus: flush left */
		flush_left = 1;
		f++;
	    }

	    if (*f == '0' || *f == '.') {
		/* padding with 0 rather than blank */
		pad = '0';
		f++;
	    }
	    if (*f == '*') {	/* field width */
		f_width = va_arg(ap, int);
		f++;
	    }
	    else if (Isdigit((unsigned char) *f)) {
		f_width = atoi(f);
		while (Isdigit((unsigned char) *f))
		    f++;	/* skip the digits */
	    }

	    if (*f == '.') {	/* precision */
		f++;
		if (*f == '*') {
		    prec = va_arg(ap, int);
		    f++;
		}
		else if (Isdigit((unsigned char) *f)) {
		    prec = atoi((char *) f);
		    while (Isdigit((unsigned char) *f))
			f++;	/* skip the digits */
		}
	    }

	    if (*f == '#') {	/* alternate form */
		hash = 1;
		f++;
	    }

	    if (*f == 'l') {	/* long format */
		do_long = 1;
		f++;
	    }

	    fmt = (unsigned char) *f;
	    if (Isupper(fmt)) {
		do_long = 1;
		fmt = Tolower(fmt);
	    }
	    bp = buf;
	    switch (fmt) {	/* do the format */
	    case 'd':
		if (do_long)
		    l = va_arg(ap, long);
		else
		    l = (long) (va_arg(ap, int));
		if (l < 0) {
		    sign = 1;
		    l = -l;
		}
		do {
		    *bp++ = l % 10 + '0';
		} while ((l /= 10) > 0);
		if (sign)
		    *bp++ = '-';
		f_width = f_width - (bp - buf);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (bp--; bp >= buf; bp--)
		    (*addchar) ((int) (((unsigned char) *bp) | attributes));
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;

	    case 'o':
	    case 'x':
	    case 'u':
		if (do_long)
		    u = va_arg(ap, unsigned long);
		else
		    u = (unsigned long) (va_arg(ap, unsigned int));
		if (fmt == 'u') {	/* unsigned decimal */
		    do {
			*bp++ = u % 10 + '0';
		    } while ((u /= 10) > 0);
		}
		else if (fmt == 'o') {	/* octal */
		    do {
			*bp++ = u % 8 + '0';
		    } while ((u /= 8) > 0);
		    if (hash)
			*bp++ = '0';
		}
		else if (fmt == 'x') {	/* hex */
		    do {
			i = u % 16;
			if (i < 10)
			    *bp++ = i + '0';
			else
			    *bp++ = i - 10 + 'a';
		    } while ((u /= 16) > 0);
		    if (hash) {
			*bp++ = 'x';
			*bp++ = '0';
		    }
		}
		i = f_width - (bp - buf);
		if (!flush_left)
		    while (i-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (bp--; bp >= buf; bp--)
		    (*addchar) ((int) (((unsigned char) *bp) | attributes));
		if (flush_left)
		    while (i-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;


	    case 'c':
		i = va_arg(ap, int);
		(*addchar) ((int) (i | attributes));
		break;

	    case 's':
		bp = va_arg(ap, char *);
		if (!bp)
		    bp = "(nil)";
		f_width = f_width - strlen((char *) bp);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (i = 0; *bp && i < prec; i++) {
		    (*addchar) ((int) (((unsigned char) *bp) | attributes));
		    bp++;
		}
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (' ' | attributes));

		break;

	    case 'a':
		attributes = va_arg(ap, int);
		break;

	    case '%':
		(*addchar) ((int) ('%' | attributes));
		break;

	    default:
		break;
	    }
	    flush_left = 0, f_width = 0, prec = INF, hash = 0, do_long = 0;
	    sign = 0;
	    pad = ' ';
	}
    }
}


static char *xstring;
static void
xaddchar(c)
    int     c;
{
    *xstring++ = (char) c;
}


void
/*VARARGS*/
#if __STDC__
xsprintf(char *str, char *fmt, ...)
#else
xsprintf(va_alist)
    va_dcl
#endif
{
    va_list va;
#if __STDC__
    va_start(va, fmt);
#else
    char *str, *fmt;

    va_start(va);
    str = va_arg(va, char *);
    fmt = va_arg(va, char *);
#endif

    xstring = (char *) str;
    doprnt(xaddchar, fmt, va);
    va_end(va);
    *xstring++ = '\0';
}


void
/*VARARGS*/
#if __STDC__
xprintf(char *fmt, ...)
#else
xprintf(va_alist)
    va_dcl
#endif
{
    va_list va;
#if __STDC__
    va_start(va, fmt);
#else
    char   *fmt;

    va_start(va);
    fmt = va_arg(va, char *);
#endif
    doprnt(xputchar, fmt, va);
    va_end(va);
}


void
xvprintf(fmt, va)
    char   *fmt;
    va_list va;
{
    doprnt(xputchar, fmt, va);
}

void
xvsprintf(str, fmt, va)
    char   *str;
    char   *fmt;
    va_list va;
{
    xstring = (char *) str;
    doprnt(xaddchar, fmt, va);
    *xstring++ = '\0';
}
