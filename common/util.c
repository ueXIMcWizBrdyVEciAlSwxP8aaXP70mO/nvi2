/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1991, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 *
 * See the LICENSE file for redistribution information.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "$Id: util.c,v 10.23 2011/07/04 20:27:50 zy Exp $ (Berkeley) $Date: 2011/07/04 20:27:50 $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

/*
 * binc --
 *	Increase the size of a buffer.
 *
 * PUBLIC: void *binc __P((SCR *, void *, size_t *, size_t));
 */
void *
binc(
	SCR *sp,			/* sp MAY BE NULL!!! */
	void *bp,
	size_t *bsizep,
	size_t min)
{
	size_t csize;

	/* If already larger than the minimum, just return. */
	if (min && *bsizep >= min)
		return (bp);

	csize = *bsizep + MAX(min, 256);
	REALLOC(sp, bp, void *, csize);

	if (bp == NULL) {
		/*
		 * Theoretically, realloc is supposed to leave any already
		 * held memory alone if it can't get more.  Don't trust it.
		 */
		*bsizep = 0;
		return (NULL);
	}
	/*
	 * Memory is guaranteed to be zero-filled, various parts of
	 * nvi depend on this.
	 */
	memset((char *)bp + *bsizep, 0, csize - *bsizep);
	*bsizep = csize;
	return (bp);
}

/*
 * nonblank --
 *	Set the column number of the first non-blank character
 *	including or after the starting column.  On error, set
 *	the column to 0, it's safest.
 *
 * PUBLIC: int nonblank __P((SCR *, recno_t, size_t *));
 */
int
nonblank(
	SCR *sp,
	recno_t lno,
	size_t *cnop)
{
	CHAR_T *p;
	size_t cnt, len, off;
	int isempty;

	/* Default. */
	off = *cnop;
	*cnop = 0;

	/* Get the line, succeeding in an empty file. */
	if (db_eget(sp, lno, &p, &len, &isempty))
		return (!isempty);

	/* Set the offset. */
	if (len == 0 || off >= len)
		return (0);

	for (cnt = off, p = &p[off],
	    len -= off; len && isblank(*p); ++cnt, ++p, --len);

	/* Set the return. */
	*cnop = len ? cnt : cnt - 1;
	return (0);
}

/*
 * tail --
 *	Return tail of a path.
 *
 * PUBLIC: char *tail __P((char *));
 */
char *
tail(char *path)
{
	char *p;

	if ((p = strrchr(path, '/')) == NULL)
		return (path);
	return (p + 1);
}

/*
 * v_strdup --
 *	Strdup for 8-bit character strings with an associated length.
 *
 * PUBLIC: char *v_strdup __P((SCR *, const char *, size_t));
 */
char *
v_strdup(
	SCR *sp,
	const char *str,
	size_t len)
{
	char *copy;

	MALLOC(sp, copy, char *, len + 1);
	if (copy == NULL)
		return (NULL);
	memcpy(copy, str, len);
	copy[len] = '\0';
	return (copy);
}

/*
 * v_wstrdup --
 *	Strdup for wide character strings with an associated length.
 *
 * PUBLIC: CHAR_T *v_wstrdup __P((SCR *, const CHAR_T *, size_t));
 */
CHAR_T *
v_wstrdup(SCR *sp,
	const CHAR_T *str,
	size_t len)
{
	CHAR_T *copy;

	MALLOC(sp, copy, CHAR_T *, (len + 1) * sizeof(CHAR_T));
	if (copy == NULL)
		return (NULL);
	MEMCPYW(copy, str, len);
	copy[len] = '\0';
	return (copy);
}

/*
 * nget_uslong --
 *      Get an unsigned long, checking for overflow.
 *
 * PUBLIC: enum nresult nget_uslong __P((u_long *, const CHAR_T *, CHAR_T **, int));
 */
enum nresult
nget_uslong(
	u_long *valp,
	const CHAR_T *p,
	CHAR_T **endp,
	int base)
{
	errno = 0;
	*valp = STRTOUL(p, endp, base);
	if (errno == 0)
		return (NUM_OK);
	if (errno == ERANGE && *valp == ULONG_MAX)
		return (NUM_OVER);
	return (NUM_ERR);
}

/*
 * nget_slong --
 *      Convert a signed long, checking for overflow and underflow.
 *
 * PUBLIC: enum nresult nget_slong __P((long *, const CHAR_T *, CHAR_T **, int));
 */
enum nresult
nget_slong(
	long *valp,
	const CHAR_T *p,
	CHAR_T **endp,
	int base)
{
	errno = 0;
	*valp = STRTOL(p, endp, base);
	if (errno == 0)
		return (NUM_OK);
	if (errno == ERANGE) {
		if (*valp == LONG_MAX)
			return (NUM_OVER);
		if (*valp == LONG_MIN)
			return (NUM_UNDER);
	}
	return (NUM_ERR);
}

#ifdef DEBUG
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * TRACE --
 *	debugging trace routine.
 *
 * PUBLIC: void TRACE __P((SCR *, const char *, ...));
 */
void
#ifdef __STDC__
TRACE(SCR *sp, const char *fmt, ...)
#else
TRACE(sp, fmt, va_alist)
	SCR *sp;
	char *fmt;
	va_dcl
#endif
{
	FILE *tfp;
	va_list ap;

	if ((tfp = sp->gp->tracefp) == NULL)
		return;
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)vfprintf(tfp, fmt, ap);
	va_end(ap);

	(void)fflush(tfp);
}
#endif
