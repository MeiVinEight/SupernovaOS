/**
 * sprintf.c  —  self-contained sprintf / snprintf implementation
 *
 * No libc dependency.  Supports:
 *
 *   %%              literal '%'
 *   %c              char
 *   %s              null-terminated string
 *   %d / %i         signed decimal   (int / long / long long)
 *   %u              unsigned decimal (unsigned int / long / long long)
 *   %o              unsigned octal
 *   %x / %X         unsigned hex (lower / upper)
 *   %p              pointer  (0x-prefixed, sizeof(void*)-wide hex)
 *   %f              floating-point decimal  (no %e/%g; precision 0..9)
 *
 * Flags   : '-'  '+'  ' '  '0'  '#'
 * Width   : decimal integer or '*'
 * Precision: '.' followed by decimal integer or '*'
 * Length  : hh  h  (none)  l  ll  z  t
 *
 * API:
 *   int my_sprintf (char *buf,                    const char *fmt, ...);
 *   int my_snprintf(char *buf, size_t n,          const char *fmt, ...);
 *   int my_vsprintf (char *buf,                   const char *fmt, va_list ap);
 *   int my_vsnprintf(char *buf, size_t n,         const char *fmt, va_list ap);
 *
 * Return value follows C99: number of characters that *would* have been
 * written (excluding NUL), regardless of buffer size n.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <std/string.h>

/* =========================================================================
 * Internal write context — keeps track of the output buffer.
 * Using a context struct lets all helpers share one clipping boundary
 * without threading `buf` and `n` through every call.
 * ========================================================================= */

typedef struct
{
	char* buf; /* start of caller's buffer (may be NULL for counting)  */
	size_t cap; /* total capacity including the NUL byte                */
	size_t pos; /* next write position (logical; may exceed cap)        */
} ctx_t;

/* Write one character to the context, advancing pos regardless of clipping. */
static void ctx_putc(ctx_t* c, char ch)
{
	if (c->buf && c->pos + 1 < c->cap) /* +1: leave room for NUL        */
		c->buf[c->pos] = ch;
	c->pos++;
}

/* Write a run of `n` copies of character `ch`. */
static void ctx_fill(ctx_t* c, char ch, size_t n)
{
	for (size_t i = 0; i < n; i++)
		ctx_putc(c, ch);
}

/* Write a string of known length. */
static void ctx_write(ctx_t* c, const char* s, size_t len)
{
	for (size_t i = 0; i < len; i++)
		ctx_putc(c, s[i]);
}

/* =========================================================================
 * Flags / state for one conversion specifier
 * ========================================================================= */

#define FL_LEFT     (1u << 0)   /* '-'  left-align                          */
#define FL_PLUS     (1u << 1)   /* '+'  force sign                          */
#define FL_SPACE    (1u << 2)   /* ' '  space before positive               */
#define FL_ZERO     (1u << 3)   /* '0'  zero-pad                            */
#define FL_HASH     (1u << 4)   /* '#'  alternate form (0x, 0, …)           */
#define FL_UPPER    (1u << 5)   /* uppercase hex / pointer                  */
#define FL_NEG      (1u << 6)   /* value is negative                        */
#define FL_PREC     (1u << 7)   /* explicit precision given                 */

/* Length modifier */
typedef enum { LEN_NONE, LEN_HH, LEN_H, LEN_L, LEN_LL, LEN_Z, LEN_T } len_t;

/* =========================================================================
 * Integer → string conversion (no division loops if power-of-2 base)
 *
 * Writes into a temporary buffer from right to left; returns pointer to
 * the first digit and sets *out_len.
 * ========================================================================= */

#define ITOA_BUF 22     /* enough for 64-bit octal + sign                   */

static const char DIGITS_LO[] = "0123456789abcdef";
static const char DIGITS_HI[] = "0123456789ABCDEF";

static char* u64_to_str(uint64_t val, unsigned base, bool upper,
                        char* buf_end, size_t* out_len)
{
	const char* digits = upper ? DIGITS_HI : DIGITS_LO;
	char* p = buf_end; /* write backwards                              */
	*--p = '\0';

	if (val == 0)
	{
		*--p = '0';
	}
	else
	{
		while (val)
		{
			*--p = digits[val % base];
			val /= base;
		}
	}

	*out_len = (size_t)(buf_end - p - 1);
	return p;
}

/* =========================================================================
 * Emit a fully-formatted integer / pointer field
 * ========================================================================= */

static void emit_int(ctx_t* c,
                     uint64_t uval,
                     unsigned base,
                     unsigned flags,
                     int width,
                     int prec) /* -1 = no precision given */
{
	char tmp[ITOA_BUF];
	size_t ndigits;
	char* digits = u64_to_str(uval, base,
	                          !!(flags & FL_UPPER),
	                          tmp + ITOA_BUF, &ndigits);

	/* Prefix: sign or '#' alternate form */
	char prefix[3];
	size_t prefix_len = 0;

	if (base == 10)
	{
		if (flags & FL_NEG) prefix[prefix_len++] = '-';
		else if (flags & FL_PLUS) prefix[prefix_len++] = '+';
		else if (flags & FL_SPACE) prefix[prefix_len++] = ' ';
	}
	else if (flags & FL_HASH)
	{
		if (base == 16 && uval)
		{
			prefix[prefix_len++] = '0';
			prefix[prefix_len++] = (flags & FL_UPPER) ? 'X' : 'x';
		}
		else if (base == 8 && (ndigits == 0 || digits[0] != '0'))
		{
			prefix[prefix_len++] = '0';
		}
	}

	/* Precision: minimum number of digits (overrides zero-flag for integers) */
	size_t min_digits = (prec >= 0) ? (size_t)prec : 0;
	size_t zpad = (min_digits > ndigits) ? min_digits - ndigits : 0;

	/* Total "payload" width (prefix + zero-pad + digits) */
	size_t payload = prefix_len + zpad + ndigits;

	/* Field width padding */
	size_t pad = ((int)payload < width) ? (size_t)(width - (int)payload) : 0;

	/* Zero-flag only applies when no explicit precision */
	bool zero_pad = (flags & FL_ZERO) && !(flags & FL_PREC) && !(flags & FL_LEFT);

	if (!(flags & FL_LEFT) && !zero_pad)
		ctx_fill(c, ' ', pad); /* right-align: space pad on the left   */

	ctx_write(c, prefix, prefix_len);

	if (zero_pad)
		ctx_fill(c, '0', pad + zpad); /* zero pad replaces both space+zpad */
	else
		ctx_fill(c, '0', zpad);

	ctx_write(c, digits, ndigits);

	if (flags & FL_LEFT)
		ctx_fill(c, ' ', pad); /* left-align: space pad on the right   */
}

/* =========================================================================
 * Floating-point: %f only.
 *
 * Strategy: extract integer and fractional parts as integers, format each
 * separately. No sprintf recursion, no libm dependency.
 * Range: approximately ±1e18 (fits in uint64_t integer part).
 * ========================================================================= */

/* Powers of 10 for precision 0..9 */
static const uint64_t POW10[10] = {
	1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL,
	100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL
};

static void emit_float(ctx_t* c, double val,
                       unsigned flags, int width, int prec)
{
	if (prec < 0) prec = 6; /* default precision for %f */
	if (prec > 9) prec = 9; /* clamp to our table size  */

	/* Handle sign */
	if (val < 0.0)
	{
		flags |= FL_NEG;
		val = -val;
	}

	/* Round: add 0.5 * 10^-prec so truncation is equivalent to rounding */
	double round_add = 0.5;
	for (int i = 0; i < prec; i++) round_add /= 10.0;
	val += round_add;

	uint64_t int_part = (uint64_t)val;
	uint64_t frac_part = 0;
	if (prec > 0)
	{
		double frac = val - (double)int_part;
		frac_part = (uint64_t)(frac * (double)POW10[prec]);
	}

	/* Convert integer part */
	char ibuf[ITOA_BUF];
	size_t ilen;
	char* istr = u64_to_str(int_part, 10, false, ibuf + ITOA_BUF, &ilen);

	/* Convert fractional part, zero-padded to `prec` digits */
	char fbuf[ITOA_BUF];
	size_t flen = 0;
	char* fstr = NULL;
	if (prec > 0)
	{
		fstr = u64_to_str(frac_part, 10, false, fbuf + ITOA_BUF, &flen);
		/* Left-pad with zeros if frac_part has fewer digits than prec */
	}

	/* sign + digits + '.' + frac digits */
	size_t sign_len = (flags & (FL_NEG | FL_PLUS | FL_SPACE)) ? 1 : 0;
	size_t dot_len = (prec > 0 || (flags & FL_HASH)) ? 1 : 0;
	size_t fzpad = (fstr && (size_t)prec > flen) ? (size_t)prec - flen : 0;
	size_t payload = sign_len + ilen + dot_len + fzpad + flen;
	size_t pad = ((int)payload < width) ? (size_t)(width - (int)payload) : 0;

	bool zero_pad = (flags & FL_ZERO) && !(flags & FL_LEFT);

	if (!(flags & FL_LEFT) && !zero_pad) ctx_fill(c, ' ', pad);

	/* Sign character */
	if (flags & FL_NEG) ctx_putc(c, '-');
	else if (flags & FL_PLUS) ctx_putc(c, '+');
	else if (flags & FL_SPACE) ctx_putc(c, ' ');

	if (zero_pad) ctx_fill(c, '0', pad);

	ctx_write(c, istr, ilen);

	if (dot_len)
	{
		ctx_putc(c, '.');
		ctx_fill(c, '0', fzpad);
		if (fstr) ctx_write(c, fstr, flen);
	}

	if (flags & FL_LEFT) ctx_fill(c, ' ', pad);
}

/* =========================================================================
 * String / char field
 * ========================================================================= */

static void emit_str(ctx_t* c, const char* s, unsigned flags,
                     int width, int prec)
{
	if (!s) s = "(null)";

	/* Compute length, clamped by precision */
	size_t len = 0;
	while (s[len]) len++;
	if ((flags & FL_PREC) && prec >= 0 && (size_t)prec < len)
		len = (size_t)prec;

	size_t pad = ((int)len < width) ? (size_t)(width - (int)len) : 0;

	if (!(flags & FL_LEFT)) ctx_fill(c, ' ', pad);
	ctx_write(c, s, len);
	if (flags & FL_LEFT) ctx_fill(c, ' ', pad);
}

static void emit_char(ctx_t* c, char ch, unsigned flags, int width)
{
	size_t pad = (width > 1) ? (size_t)(width - 1) : 0;
	if (!(flags & FL_LEFT)) ctx_fill(c, ' ', pad);
	ctx_putc(c, ch);
	if (flags & FL_LEFT) ctx_fill(c, ' ', pad);
}

/* =========================================================================
 * Core formatter
 * ========================================================================= */

static int core_vsnprintf(ctx_t* c, const char* fmt, va_list ap)
{
	for (const char* p = fmt; *p; p++)
	{
		/* Literal character */
		if (*p != '%')
		{
			ctx_putc(c, *p);
			continue;
		}
		p++; /* skip '%' */

		/* ------- Parse flags ------- */
		unsigned flags = 0;
		bool parsing_flags = true;
		while (parsing_flags)
		{
			switch (*p)
			{
				case '-': flags |= FL_LEFT;
					p++;
					break;
				case '+': flags |= FL_PLUS;
					p++;
					break;
				case ' ': flags |= FL_SPACE;
					p++;
					break;
				case '0': flags |= FL_ZERO;
					p++;
					break;
				case '#': flags |= FL_HASH;
					p++;
					break;
				default: parsing_flags = false;
					break;
			}
		}

		/* ------- Width ------- */
		int width = 0;
		if (*p == '*')
		{
			width = va_arg(ap, int);
			if (width < 0)
			{
				flags |= FL_LEFT;
				width = -width;
			}
			p++;
		}
		else
		{
			while (*p >= '0' && *p <= '9')
				width = width * 10 + (*p++ - '0');
		}

		/* ------- Precision ------- */
		int prec = -1;
		if (*p == '.')
		{
			p++;
			flags |= FL_PREC;
			prec = 0;
			if (*p == '*')
			{
				prec = va_arg(ap, int);
				if (prec < 0)
				{
					prec = -1;
					flags &= ~FL_PREC;
				}
				p++;
			}
			else
			{
				while (*p >= '0' && *p <= '9')
					prec = prec * 10 + (*p++ - '0');
			}
		}

		/* ------- Length modifier ------- */
		len_t len = LEN_NONE;
		switch (*p)
		{
			case 'h': p++;
				if (*p == 'h')
				{
					len = LEN_HH;
					p++;
				}
				else { len = LEN_H; }
				break;
			case 'l': p++;
				if (*p == 'l')
				{
					len = LEN_LL;
					p++;
				}
				else { len = LEN_L; }
				break;
			case 'z': len = LEN_Z;
				p++;
				break;
			case 't': len = LEN_T;
				p++;
				break;
		}

		/* ------- Conversion specifier ------- */
		char spec = *p;
		if (!spec) break; /* malformed format — stop */

		switch (spec)
		{
			/* ---- %% ---- */
			case '%':
				ctx_putc(c, '%');
				break;

			/* ---- %c ---- */
			case 'c':
			{
				char ch = (char)va_arg(ap, int); /* char promoted to int */
				emit_char(c, ch, flags, width);
				break;
			}

			/* ---- %s ---- */
			case 's':
			{
				const char* s = va_arg(ap, const char *);
				emit_str(c, s, flags, width, prec);
				break;
			}

			/* ---- %d / %i (signed decimal) ---- */
			case 'd':
			case 'i':
			{
				int64_t sval;
				uint64_t uval;

				switch (len)
				{
					case LEN_LL: sval = (int64_t)va_arg(ap, long long);
						break;
					case LEN_L: sval = (int64_t)va_arg(ap, long);
						break;
					case LEN_H: sval = (int16_t)va_arg(ap, int);
						break;
					case LEN_HH: sval = (int8_t)va_arg(ap, int);
						break;
					case LEN_Z: sval = (int64_t)va_arg(ap, size_t);
						break;
					case LEN_T: sval = (int64_t)va_arg(ap, ptrdiff_t);
						break;
					default: sval = (int64_t)va_arg(ap, int);
						break;
				}

				if (sval < 0)
				{
					flags |= FL_NEG;
					uval = (uint64_t)(-sval);
				}
				else { uval = (uint64_t)sval; }

				emit_int(c, uval, 10, flags, width, prec);
				break;
			}

			/* ---- %u ---- */
			case 'u':
			{
				uint64_t uval;
				switch (len)
				{
					case LEN_LL: uval = (uint64_t)va_arg(ap, unsigned long long);
						break;
					case LEN_L: uval = (uint64_t)va_arg(ap, unsigned long);
						break;
					case LEN_H: uval = (uint16_t)va_arg(ap, unsigned int);
						break;
					case LEN_HH: uval = (uint8_t)va_arg(ap, unsigned int);
						break;
					case LEN_Z: uval = (uint64_t)va_arg(ap, size_t);
						break;
					case LEN_T: uval = (uint64_t)va_arg(ap, ptrdiff_t);
						break;
					default: uval = (uint64_t)va_arg(ap, unsigned int);
						break;
				}
				emit_int(c, uval, 10, flags, width, prec);
				break;
			}

			/* ---- %o ---- */
			case 'o':
			{
				uint64_t uval;
				switch (len)
				{
					case LEN_LL: uval = (uint64_t)va_arg(ap, unsigned long long);
						break;
					case LEN_L: uval = (uint64_t)va_arg(ap, unsigned long);
						break;
					default: uval = (uint64_t)va_arg(ap, unsigned int);
						break;
				}
				emit_int(c, uval, 8, flags, width, prec);
				break;
			}

			/* ---- %x / %X ---- */
			case 'x':
			case 'X':
			{
				uint64_t uval;
				switch (len)
				{
					case LEN_LL: uval = (uint64_t)va_arg(ap, unsigned long long);
						break;
					case LEN_L: uval = (uint64_t)va_arg(ap, unsigned long);
						break;
					case LEN_H: uval = (uint16_t)va_arg(ap, unsigned int);
						break;
					case LEN_HH: uval = (uint8_t)va_arg(ap, unsigned int);
						break;
					case LEN_Z: uval = (uint64_t)va_arg(ap, size_t);
						break;
					default: uval = (uint64_t)va_arg(ap, unsigned int);
						break;
				}
				if (spec == 'X') flags |= FL_UPPER;
				emit_int(c, uval, 16, flags, width, prec);
				break;
			}

			/* ---- %p ---- */
			case 'p':
			{
				uintptr_t ptr = (uintptr_t)va_arg(ap, void *);
				/* Always: "0x" prefix, lowercase hex, width = 2*pointer size */
				flags |= FL_HASH;
				int ptr_width = (width > 0) ? width : (int)(sizeof(void*) * 2 + 2);
				emit_int(c, (uint64_t)ptr, 16, flags, ptr_width, prec);
				break;
			}

			/* ---- %f ---- */
			case 'f':
			case 'F':
			{
				double val = va_arg(ap, double);
				emit_float(c, val, flags, width, prec);
				break;
			}

			/* ---- %n (write count back) ---- */
			case 'n':
			{
				/*
				 * %n is a potential security vulnerability (format-string attacks).
				 * Implement only if you are certain the call sites are trusted.
				 * We support it for completeness; production code should omit it.
				 */
				int* np = va_arg(ap, int *);
				if (np) *np = (int)c->pos;
				break;
			}

			default:
				/* Unknown specifier — emit literally */
				ctx_putc(c, '%');
				ctx_putc(c, spec);
				break;
		}
	}

	/* NUL-terminate (does NOT count towards return value) */
	if (c->buf && c->cap > 0)
	{
		size_t nul_pos = (c->pos < c->cap) ? c->pos : c->cap - 1;
		c->buf[nul_pos] = '\0';
	}

	return (int)c->pos;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap)
{
	ctx_t c = {buf, n, 0};
	return core_vsnprintf(&c, fmt, ap);
}

int vsprintf(char* buf, const char* fmt, va_list ap)
{
	/* Unbounded: cap = SIZE_MAX so clipping never fires */
	ctx_t c = {buf, (size_t)-1, 0};
	return core_vsnprintf(&c, fmt, ap);
}

int __cdecl snprintf(char* buf, size_t n, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vsnprintf(buf, n, fmt, ap);
	va_end(ap);
	return r;
}

int __cdecl sprintf(char* buf, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vsprintf(buf, fmt, ap);
	va_end(ap);
	return r;
}

/* =========================================================================
 * Optional: drop-in aliases (uncomment if you want printf/sprintf redirected)
 * =========================================================================
 *
 * #define sprintf  my_sprintf
 * #define snprintf my_snprintf
 * #define vsprintf  my_vsprintf
 * #define vsnprintf my_vsnprintf
 */
