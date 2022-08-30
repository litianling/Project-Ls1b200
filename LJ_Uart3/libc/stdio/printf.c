
//-------------------------------------------------------------------------------------------------
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and (v)snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources. These routines are thread
//        safe and reentrant!
//        Use this instead of the bloated standard/newlib printf cause these use
//        malloc for printf (and may not be thread safe).
//
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#include "printf.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * XXX 需要浮点数和64位整数打印支持吗? 调整下面的宏定义.
 *
 *     默认支持龙芯1B200和龙芯1C300, 不支持1C101和1J
 *
 */

//-------------------------------------------------------------------------------------------------
// define this globally (e.g. gcc -DPRINTF_INCLUDE_CONFIG_H ...) to include the
// printf_config.h header file
// default: undefined

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_FTOA_BUFFER_SIZE    32U
#endif

// support for the floating point type (%f)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_FLOAT
  #if defined(LS1B) || defined(LS1C)
	#define PRINTF_SUPPORT_FLOAT			/* XXX float 类型打印 */
  #endif
#endif

// support for exponential floating point notation (%e/%g)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
  #if defined(LS1B) || defined(LS1C)
	#define PRINTF_SUPPORT_EXPONENTIAL		/* XXX double 类型打印 */
  #endif
#endif

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U
#endif

// define the largest float suitable to print with %f
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#define PRINTF_MAX_FLOAT  1e9
#endif

// support for the long long types (%ll or %p)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
  #if defined(LS1B) || defined(LS1C)
	#define PRINTF_SUPPORT_LONG_LONG		/* XXX 64位整数类型打印  */
  #endif
#endif

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_SUPPORT_PTRDIFF_T
#endif

//-------------------------------------------------------------------------------------------------

// 内部标志定义  <<为左移
#define FLAGS_ZEROPAD       (1U <<  0U)  //1
#define FLAGS_LEFT          (1U <<  1U)  //2
#define FLAGS_PLUS          (1U <<  2U)  //4
#define FLAGS_SPACE         (1U <<  3U)  //8
#define FLAGS_HASH          (1U <<  4U)  //16
#define FLAGS_UPPERCASE     (1U <<  5U)  //32
#define FLAGS_CHAR          (1U <<  6U)  //64
#define FLAGS_SHORT         (1U <<  7U)  //128
#define FLAGS_LONG          (1U <<  8U)  //256
#define FLAGS_LONG_LONG     (1U <<  9U)  //512
#define FLAGS_PRECISION     (1U << 10U)  //1024
#define FLAGS_ADAPT_EXP     (1U << 11U)  //2048

// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#include <float.h>
#endif

// 输出函数类型
typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

// 输出函数类型的包装器(用作缓冲区)
typedef struct
{
    void  (*fct)(char character, void* arg);
    void* arg;
} out_fct_wrap_type;

//-------------------------------------------------------------------------------------------------

// 内部缓冲输出
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
    if (idx < maxlen)
    {
        ((char*)buffer)[idx] = character;
    }
}

// 内部零输出
static inline void _out_null(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)character; (void)buffer; (void)idx; (void)maxlen;
}

// 内部 _putchar 包装
static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)buffer; (void)idx; (void)maxlen;
    if (character)
    {
        _putchar(character);
    }
}

static inline void _out_char1(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)buffer; (void)idx; (void)maxlen;
    if (character)
    {
        _putchar1(character);
    }
}
/*
我们知道，在定义函数时，加在函数名前的“void”表示该函数没有返回值。但在调用时，在函数名前加“(void)”的作用又是什么呢？
　　最明显的一点就是表示程序并不关心调用该函数后的返回值是什么，比如函数strcpy，我们直接用“strcpy(des_str, src_str);”这样的形式来调用。
“(void)strcpy(des_str, src_str);”这样的形式还真不多见！
　　原因是这种写法不针对人，也不针对编译器，而是针对静态代码检测工具，它会把函数返回值作为一项检测标准。在某些大公司，比较重视代码规范，
若在代码静态检测时需要检测该项。此时就需要用在被调用的函数(名)前加上“(void)”这种形式来告诉静态代码检测工具程序并非没有处理该函数的返回值，
而是该处确实不需要处理它(该函数的返回值)，不需要再对该处代码作此项检测。其实这和我们在代码中使用“#pragma warning (disable: XXXX)”的道理是一样的。
*/



// 内部输出函数包装器
static inline void _out_fct(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)idx; (void)maxlen;
    if (character)
    {
        // Buffer是输出的FCT指针
        ((out_fct_wrap_type*)buffer)->fct(character, ((out_fct_wrap_type*)buffer)->arg);
    }
}

// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by 'maxsize'
static inline unsigned int _strnlen_s(const char* str, size_t maxsize)
{
    const char* s;
    for (s = str; *s && maxsize--; ++s);
    return (unsigned int)(s - str);
}

// internal test if char is a digit (0-9)
// \return true if char is a digit
static inline bool _is_digit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

// internal ASCII string to unsigned int conversion
static unsigned int _atoi(const char** str)
{
    unsigned int i = 0U;
    while (_is_digit(**str))
    {
        i = i * 10U + (unsigned int)(*((*str)++) - '0');
    }
    return i;
}

// output the specified string in reverse, taking care of any zero-padding
static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen,
                       const char* buf, size_t len, unsigned int width, unsigned int flags)
{
    size_t i;
    const size_t start_idx = idx;

    // pad spaces up to given width
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD))
    {
        for (i = len; i < width; i++)
        {
            out(' ', buffer, idx++, maxlen);
        }
    }

    // reverse string
    while (len)
    {
        out(buf[--len], buffer, idx++, maxlen);
    }

    // append pad spaces up to given width
    if (flags & FLAGS_LEFT)
    {
        while (idx - start_idx < width)
        {
            out(' ', buffer, idx++, maxlen);
        }
    }

    return idx;
}

// internal itoa format
static size_t _ntoa_format(out_fct_type out, char* buffer, size_t idx, size_t maxlen,
                           char* buf, size_t len, bool negative, unsigned int base,
                           unsigned int prec, unsigned int width, unsigned int flags)
{
    // pad leading zeros
    if (!(flags & FLAGS_LEFT))
    {
        if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
        {
            width--;
        }

        while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE))
        {
            buf[len++] = '0';
        }

        while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE))
        {
            buf[len++] = '0';
        }
    }

    // handle hash
    if (flags & FLAGS_HASH)
    {
        if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width)))
        {
            len--;
            if (len && (base == 16U))
            {
                len--;
            }
        }

        if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE))
        {
            buf[len++] = 'x';
        }
        else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE))
        {
            buf[len++] = 'X';
        }
        else if ((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE))
        {
            buf[len++] = 'b';
        }
        if (len < PRINTF_NTOA_BUFFER_SIZE)
        {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_NTOA_BUFFER_SIZE)
    {
        if (negative)
        {
            buf[len++] = '-';
        }
        else if (flags & FLAGS_PLUS)
        {
            buf[len++] = '+';  // ignore the space if the '+' exists
        }
        else if (flags & FLAGS_SPACE)
        {
            buf[len++] = ' ';
        }
    }

    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

// internal itoa for 'long' type
static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen,
                         unsigned long value, bool negative, unsigned long base,
                         unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value)
    {
        flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value)
    {
        do
        {
            const char digit = (char)(value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
     }

    return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}

// internal itoa for 'long long' type
#if defined(PRINTF_SUPPORT_LONG_LONG)

#if (__mips == 32)
static unsigned long long div_u64_u32(unsigned long long *n, unsigned int base)
{
    unsigned long long rem = *n;
    unsigned long long b = base;
    unsigned long long res, d = 1;
    unsigned int       high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base)
    {
		high /= base;
		res = (uint64_t) high << 32;
		rem -= (uint64_t) (high*base) << 32;
	}

    while ((int64_t)b > 0 && b < rem)
    {
		b = b+b;
		d = d+d;
	}

    do
    {
		if (rem >= b)
        {
			rem -= b;
			res += d;
		}

        b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}

#endif

static size_t _ntoa_long_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen,
                              unsigned long long value, bool negative, unsigned long long base,
                              unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value)
    {
        flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value)
    {
        do
        {
            //-----------------------------------------------------------------
            // 64位与32位的问题:
            //   undefined reference to `__umoddi3'
            //   undefined reference to `__udivdi3'
            //-----------------------------------------------------------------
            
    #if (__mips == 32)
            unsigned long long rem;
            char digit;
            
            rem = div_u64_u32(&value, base);
            digit = (char)rem;
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;

    #else
            const char digit = (char)(value % base);
            
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
            
    #endif
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}
#endif  // PRINTF_SUPPORT_LONG_LONG

#if defined(PRINTF_SUPPORT_FLOAT)

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values > PRINTF_MAX_FLOAT
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value,
                    unsigned int prec, unsigned int width, unsigned int flags);
#endif

// internal ftoa for fixed decimal floating point
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value,
                    unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_FTOA_BUFFER_SIZE];
    size_t len  = 0U;
    double diff = 0.0;

    // powers of 10
    static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    // test for special values
    if (value != value)
        return _out_rev(out, buffer, idx, maxlen, "nan", 3, width, flags);
    if (value < -DBL_MAX)
        return _out_rev(out, buffer, idx, maxlen, "fni-", 4, width, flags);
    if (value > DBL_MAX)
        return _out_rev(out, buffer, idx, maxlen, (flags & FLAGS_PLUS) ? "fni+" : "fni",
                        (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);

    // test for very large values
    // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters
    // overflowing your buffers == bad
    if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT))
    {
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
        return _etoa(out, buffer, idx, maxlen, value, prec, width, flags);
#else
        return 0U;
#endif
    }

    // test for negative
    bool negative = false;
    if (value < 0)
    {
        negative = true;
        value = 0 - value;
    }

    // set default precision, if not set explicitly
    if (!(flags & FLAGS_PRECISION))
    {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }

    // limit precision to 9, cause a prec >= 10 can lead to overflow errors
    while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U))
    {
        buf[len++] = '0';
        prec--;
    }

    int whole = (int)value;
    double tmp = (value - whole) * pow10[prec];
    unsigned long frac = (unsigned long)tmp;
    diff = tmp - frac;

    if (diff > 0.5)
    {
        ++frac;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (frac >= pow10[prec])
        {
            frac = 0;
            ++whole;
        }
    }
    else if (diff < 0.5)
    {
        //
    }
    else if ((frac == 0U) || (frac & 1U))
    {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

    if (prec == 0U)
    {
        diff = value - (double)whole;
        if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1))
        {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    }
    else
    {
        unsigned int count = prec;
        // now do fractional part, as an unsigned number
        while (len < PRINTF_FTOA_BUFFER_SIZE)
        {
            --count;
            buf[len++] = (char)(48U + (frac % 10U));
            if (!(frac /= 10U))
            {
                break;
            }
        }

        // add extra 0s
        while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U))
        {
            buf[len++] = '0';
        }

        if (len < PRINTF_FTOA_BUFFER_SIZE)
        {
            // add decimal
            buf[len++] = '.';
        }
    }

    // do whole part, number is reversed
    while (len < PRINTF_FTOA_BUFFER_SIZE)
    {
        buf[len++] = (char)(48 + (whole % 10));
        if (!(whole /= 10))
        {
            break;
        }
    }

    // pad leading zeros
    if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD))
    {
        if (width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
        {
            width--;
        }

        while ((len < width) && (len < PRINTF_FTOA_BUFFER_SIZE))
        {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_FTOA_BUFFER_SIZE)
    {
        if (negative)
        {
            buf[len++] = '-';
        }
        else if (flags & FLAGS_PLUS)
        {
            buf[len++] = '+';  // ignore the space if the '+' exists
        }
        else if (flags & FLAGS_SPACE)
        {
            buf[len++] = ' ';
        }
    }

    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value,
                    unsigned int prec, unsigned int width, unsigned int flags)
{
    // check for NaN and special values
    if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX))
    {
        return _ftoa(out, buffer, idx, maxlen, value, prec, width, flags);
    }

    // determine the sign
    const bool negative = value < 0;
    if (negative)
    {
        value = -value;
    }

    // default precision
    if (!(flags & FLAGS_PRECISION))
    {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }

    // determine the decimal exponent
    // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
    union
    {
        uint64_t U;
        double   F;
    } conv;

    conv.F = value;
    int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;           // effectively log2
    conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U);  // drop the exponent so conv.F is now in [1,2)
    // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
    int expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
    // now we want to compute 10^expval but we want to be sure it won't overflow
    exp2 = (int)(expval * 3.321928094887362 + 0.5);
    const double z  = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2 = z * z;
    conv.U = (uint64_t)(exp2 + 1023) << 52U;
    // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function
    // #Continued_fractions_for_ex
    conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    // correct for rounding errors
    if (value < conv.F)
    {
        expval--;
        conv.F /= 10;
    }

    // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
    unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

    // in "%g" mode, "prec" is the number of *significant figures* not decimals
    if (flags & FLAGS_ADAPT_EXP)
    {
        // do we want to fall-back to "%f" mode?
        if ((value >= 1e-4) && (value < 1e6))
        {
            if ((int)prec > expval)
            {
                prec = (unsigned)((int)prec - expval - 1);
            }
            else
            {
                prec = 0;
            }

            flags |= FLAGS_PRECISION;   // make sure _ftoa respects precision
            // no characters in exponent
            minwidth = 0U;
            expval   = 0;
        }
        else
        {
            // we use one sigfig for the whole part
            if ((prec > 0) && (flags & FLAGS_PRECISION))
            {
                --prec;
            }
        }
    }

    // will everything fit?
    unsigned int fwidth = width;

    if (width > minwidth)
    {
        // we didn't fall-back so subtract the characters required for the exponent
        fwidth -= minwidth;
    }
    else
    {
        // not enough characters, so go back to default sizing
        fwidth = 0U;
    }

    if ((flags & FLAGS_LEFT) && minwidth)
    {
        // if we're padding on the right, DON'T pad the floating part
        fwidth = 0U;
    }

    // rescale the float value
    if (expval)
    {
        value /= conv.F;
    }

    // output the floating part
    const size_t start_idx = idx;
    idx = _ftoa(out, buffer, idx, maxlen, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

    // output the exponent part
    if (minwidth)
    {
        // output the exponential symbol
        out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
        // output the exponent value
        idx = _ntoa_long(out, buffer, idx, maxlen, (expval < 0) ? -expval : expval,
                         expval < 0, 10, 0, minwidth-1, FLAGS_ZEROPAD | FLAGS_PLUS);
        // might need to right-pad spaces
        if (flags & FLAGS_LEFT)
        {
            while (idx - start_idx < width) out(' ', buffer, idx++, maxlen);
        }
    }

    return idx;
}
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT

// 内部函数 vsnprintf     输出函数、缓存区、最大长度、流动指针、（未知）
static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va)
{
    unsigned int flags, width, precision, n;
    size_t idx = 0U;        //U无符号整型、L长整型、F浮点数

    if (!buffer)    //输出字符为空
    {
        // 使用空输出函数
        out = _out_null;
    }

    while (*format)  //逐字符输出没有结束
    {
        // format specifier?  %[flags][width][.precision][length]
        // 格式说明？ %后边跟着 标志，宽度，精度，长度
        if (*format != '%')                         //print中%号有特殊意义%c %s %d
        {
            out(*format, buffer, idx++, maxlen);    //不是%调用输出函数输出
            format++;                               //指向下一个字符
            continue;                               //跳回while循环
        }
        else
        {
            format++;                               // 是%, 分析其数据类型
        }

        // 分析标志
        flags = 0U;
        do
        {
            switch (*format)
            {
                case '0': flags |= FLAGS_ZEROPAD; format++; n = 1U; break;
                case '-': flags |= FLAGS_LEFT;    format++; n = 1U; break;
                case '+': flags |= FLAGS_PLUS;    format++; n = 1U; break;
                case ' ': flags |= FLAGS_SPACE;   format++; n = 1U; break;
                case '#': flags |= FLAGS_HASH;    format++; n = 1U; break;
                default :                                   n = 0U; break;
            }
        } while (n);

        // 分析宽度
        width = 0U;
        if (_is_digit(*format))         //_is_digit函数为在'0'到'9'之间
        {
            width = _atoi(&format);     //_atoi实现内部ASCII字符串到unsigned int的转换
        }
        else if (*format == '*')
        {
            const int w = va_arg(va, int);
            if (w < 0)
            {
                flags |= FLAGS_LEFT;    // 反向填充 reverse padding
                width = (unsigned int)-w;
            }
            else
            {
                width = (unsigned int)w;
            }
            format++;
        }

        // 分析精度
        precision = 0U;
        if (*format == '.')
        {
            flags |= FLAGS_PRECISION;
            format++;
            if (_is_digit(*format))         //*format在'0'到'9'之间
            {
                precision = _atoi(&format); //_atoi转变数据格式
            }
            else if (*format == '*')
            {
                const int prec = (int)va_arg(va, int);
                precision = prec > 0 ? (unsigned int)prec : 0U;
                format++;
            }
        }

        // 分析长度
        // 推测flags标志位c语言中数据类型有很多种类，基本的数据类型有如下几种：char，short， int ， long int, long long, float, double, long double, bool 等等
        // 其中char类型占用1字节。short类型占用2字节。int占用4字节。long int 4字节。long long 8字节，float 4字节。double 8字节。long double 8字节。bool 占1个字节
        switch (*format)
        {
            case 'l' :                          // %l 类型
                flags |= FLAGS_LONG;
                format++;
                if (*format == 'l')
                {
                    flags |= FLAGS_LONG_LONG;   // %ll 类型
                    format++;
                }
                break;

            case 'h' :                          // %h 类型
                flags |= FLAGS_SHORT;
                format++;
                if (*format == 'h')             // %hh 类型
                {
                    flags |= FLAGS_CHAR;
                    format++;
                }
                break;

#if defined(PRINTF_SUPPORT_PTRDIFF_T)
            case 't' :                          // %t 类型
                flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;
#endif

            case 'j' :                          // %j 类型
                flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;

            case 'z' :                          // %z 类型
                flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;

            default :
                break;
        }

        // 分析说明符
        switch (*format)
        {
            case 'd' :
            case 'i' :
            case 'u' :
            case 'x' :
            case 'X' :
            case 'o' :
            case 'b' :
            {
                // 设置基础 set the base
                unsigned int base;
                if (*format == 'x' || *format == 'X')
                {
                    base = 16U;
                }
                else if (*format == 'o')
                {
                    base =  8U;
                }
                else if (*format == 'b')
                {
                    base =  2U;
                }
                else
                {
                    base = 10U;
                    flags &= ~FLAGS_HASH;   // 没有哈希为dec格式 no hash for dec format
                }

                // 大写字母 uppercase
                if (*format == 'X')
                {
                    flags |= FLAGS_UPPERCASE;
                }

                // u, x, x, o, b没有加号或空格 no plus or space flag for u, x, X, o, b
                if ((*format != 'i') && (*format != 'd'))
                {
                    flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
                }

                // 当给定精度时忽略'0'标志 ignore '0' flag when precision is given
                if (flags & FLAGS_PRECISION)
                {
                    flags &= ~FLAGS_ZEROPAD;
                }

                // 转换为整数
                if ((*format == 'i') || (*format == 'd'))
                {
                    // 有符号的数据
                    if (flags & FLAGS_LONG_LONG)
                    {
    #if defined(PRINTF_SUPPORT_LONG_LONG)   // 对LONG_LONG类型数据显示的支持
                        const long long value = va_arg(va, long long);
                        idx = _ntoa_long_long(out, buffer, idx, maxlen, (unsigned long long)(value > 0 ? value : 0 - value),
                                              value < 0, base, precision, width, flags);
    #endif
                    }
                    else if (flags & FLAGS_LONG)
                    {
                        const long value = va_arg(va, long);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)(value > 0 ? value : 0 - value),
                                         value < 0, base, precision, width, flags);
                    }
                    else
                    {
                        const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ?
                                          (short int)va_arg(va, int) : va_arg(va, int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned int)(value > 0 ? value : 0 - value),
                                         value < 0, base, precision, width, flags);
                    }
                }
                else
                {
                    // 无符号的数据
                    if (flags & FLAGS_LONG_LONG)
                    {
    #if defined(PRINTF_SUPPORT_LONG_LONG)   // 对LONG_LONG类型数据显示的支持
                        idx = _ntoa_long_long(out, buffer, idx, maxlen, va_arg(va, unsigned long long),false, base, precision, width, flags);
    #endif
                    }
                    else if (flags & FLAGS_LONG)
                    {
                        idx = _ntoa_long(out, buffer, idx, maxlen, va_arg(va, unsigned long), false, base, precision, width, flags);
                    }
                    else
                    {
                        const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) :
                                                   (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) :
                                                    va_arg(va, unsigned int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, value, false, base, precision, width, flags);
                    }
                }
                format++;
                break;
            }

    #if defined(PRINTF_SUPPORT_FLOAT)  // 对浮点数显示的支持（不支持浮点数必不支持指数）
            case 'f' :
            case 'F' :
                if (*format == 'F') flags |= FLAGS_UPPERCASE;
                idx = _ftoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
                format++;
                break;

        #if defined(PRINTF_SUPPORT_EXPONENTIAL)     // 对指数显示的支持
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if ((*format == 'g')||(*format == 'G')) flags |= FLAGS_ADAPT_EXP;
                if ((*format == 'E')||(*format == 'G')) flags |= FLAGS_UPPERCASE;
                idx = _etoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
                format++;
                break;
        #endif
    #endif

            case 'c' :
            {
                unsigned int l = 1U;
                // 预填充 pre padding
                if (!(flags & FLAGS_LEFT))
                {
                    while (l++ < width)
                    {
                        out(' ', buffer, idx++, maxlen);
                    }
                }

                // 字符输出 char output
                out((char)va_arg(va, int), buffer, idx++, maxlen);
                // 后填充 post padding
                if (flags & FLAGS_LEFT)
                {
                    while (l++ < width)
                    {
                        out(' ', buffer, idx++, maxlen);
                    }
                }

                format++;
                break;
            }

            case 's' :
            {
                const char* p = va_arg(va, char*);
                unsigned int l = _strnlen_s(p, precision ? precision : (size_t)-1);
                // 预填充 pre padding
                if (flags & FLAGS_PRECISION)
                {
                    l = (l < precision ? l : precision);
                }

                if (!(flags & FLAGS_LEFT))
                {
                    while (l++ < width)
                    {
                        out(' ', buffer, idx++, maxlen);
                    }
                }

                // 字符串输出 string output
                while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--))
                {
                    out(*(p++), buffer, idx++, maxlen);
                }

                // 后填充 post padding
                if (flags & FLAGS_LEFT)
                {
                    while (l++ < width)
                    {
                        out(' ', buffer, idx++, maxlen);
                    }
                }

                format++;
                break;
            }

            case 'p' :
            {
                width = sizeof(void*) * 2U;
                flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
    #if defined(PRINTF_SUPPORT_LONG_LONG)
                const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
                if (is_ll)
                {
                    idx = _ntoa_long_long(out, buffer, idx, maxlen, (uintptr_t)va_arg(va, void*),false, 16U, precision, width, flags);
                }
                else
                {
    #endif
                     idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)((uintptr_t)va_arg(va, void*)),false, 16U, precision, width, flags);
    #if defined(PRINTF_SUPPORT_LONG_LONG)
                }
    #endif

                format++;
                break;
            }

            case '%' :                              //%后边无任何意义，照常输出
                out('%', buffer, idx++, maxlen);
                format++;
                break;

            default :                               //%后边无任何意义，照常输出
                out(*format, buffer, idx++, maxlen);
                format++;
                break;
        }
    }

    //  终止  termination
    out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

    // 返回写入的字符而没有结束  return written chars without terminating \0
    return (int)idx;
}

//-------------------------------------------------------------------------------------------------
//开始时候用到
int printf(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int sprintf(char* buffer, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int snprintf(char* buffer, size_t count, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = _vsnprintf(_out_buffer, buffer, count, format, va);
    va_end(va);
    return ret;
}

int vprintf(const char* format, va_list va)
{
    char buffer[1];
    return _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
}

int vsprintf(char *buffer, const char* format, va_list va)
{
    return _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
}

int vsnprintf(char* buffer, size_t count, const char* format, va_list va)
{
    return _vsnprintf(_out_buffer, buffer, count, format, va);
}

int fctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    const out_fct_wrap_type out_fct_wrap = { out, arg };
    const int ret = _vsnprintf(_out_fct, (char*)(uintptr_t)&out_fct_wrap, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

//-------------------------------------------------------------------------------------------------
// printk support
//-------------------------------------------------------------------------------------------------

//结束时候用到
int printk(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

//-------------------------------------------------------------------------------------------------

int iprintf(const char* format, ...)
{
    uart3_initialize(115200,8,'N',1);
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = _vsnprintf(_out_char1, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}


/*
 * @@ END
 */


