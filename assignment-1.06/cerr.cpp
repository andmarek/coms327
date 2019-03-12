#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

/*
Becase err.h is platform dependent, here's a reproduced err and errx.
Almost identical to OpenBSD's err(3) and errx(3).
*/

static void
vcerr(int eval, char const *fmt, va_list ap)
{
	int sverrno;

	sverrno = errno;

	if (fmt != NULL) {
		(void)vfprintf(stderr, fmt, ap);
		(void)fprintf(stderr, ": ");
	}

	(void)fprintf(stderr, "%s\n", strerror(sverrno));

	exit(eval);
}

void
cerr(int eval, char const *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vcerr(eval, fmt, ap);
	va_end(ap);
}

static void
vcerrx(int eval, char const *fmt, va_list ap)
{
	if (fmt != NULL) {
		(void)vfprintf(stderr, fmt, ap);
	}

	(void)fprintf(stderr, "\n");

	exit(eval);
}

void
cerrx(int eval, char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vcerrx(eval, fmt, ap);
	va_end(ap);
}
