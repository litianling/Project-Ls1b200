
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "bsp.h"

//-----------------------------------------------------------------------------

/*
 * errno.c
 */
static int _errno;

int *__errno(void)
{
    return &_errno;
}

/*
 * ctype.c
 */
#define _UPC    _U
#define _LWR    _L
#define _DIG    _N
#define _SPC    _S
#define _PUN    _P
#define _CTR    _C
#define _HEX    _X
#define _BLK    _B

const char _ctype_[] =
{
    /*   0 */ 0,            /*   1 */ _CTR,         /*   2 */ _CTR,         /*   3 */ _CTR,
    /*   4 */ _CTR,         /*   5 */ _CTR,         /*   6 */ _CTR,         /*   7 */ _CTR,
    /*   8 */ _CTR,         /*   9 */ _SPC+_CTR,    /*  10 */ _SPC+_CTR,    /*  11 */ _SPC+_CTR,
    /*  12 */ _SPC+_CTR,    /*  13 */ _SPC+_CTR,    /*  14 */ _CTR,         /*  15 */ _CTR,
    /*  16 */ _CTR,         /*  17 */ _CTR,         /*  18 */ _CTR,         /*  19 */ _CTR,
    /*  20 */ _CTR,         /*  21 */ _CTR,         /*  22 */ _CTR,         /*  23 */ _CTR,
    /*  24 */ _CTR,         /*  25 */ _CTR,         /*  26 */ _CTR,         /*  27 */ _CTR,
    /*  28 */ _CTR,         /*  29 */ _CTR,         /*  30 */ _CTR,         /*  31 */ _CTR,
    /*  32 */ _SPC+_BLK,    /*  33 */ _PUN,         /*  34 */ _PUN,         /*  35 */ _PUN,
    /*  36 */ _PUN,         /*  37 */ _PUN,         /*  38 */ _PUN,         /*  39 */ _PUN,
    /*  40 */ _PUN,         /*  41 */ _PUN,         /*  42 */ _PUN,         /*  43 */ _PUN,
    /*  44 */ _PUN,         /*  45 */ _PUN,         /*  46 */ _PUN,         /*  47 */ _PUN,
    /*  48 */ _DIG,         /*  49 */ _DIG,         /*  50 */ _DIG,         /*  51 */ _DIG,
    /*  52 */ _DIG,         /*  53 */ _DIG,         /*  54 */ _DIG,         /*  55 */ _DIG,
    /*  56 */ _DIG,         /*  57 */ _DIG,         /*  58 */ _PUN,         /*  59 */ _PUN,
    /*  60 */ _PUN,         /*  61 */ _PUN,         /*  62 */ _PUN,         /*  63 */ _PUN,
    /*  64 */ _PUN,         /*  65 */ _UPC+_HEX,    /*  66 */ _UPC+_HEX,    /*  67 */ _UPC+_HEX,
    /*  68 */ _UPC+_HEX,    /*  69 */ _UPC+_HEX,    /*  70 */ _UPC+_HEX,    /*  71 */ _UPC,
    /*  72 */ _UPC,         /*  73 */ _UPC,         /*  74 */ _UPC,         /*  75 */ _UPC,
    /*  76 */ _UPC,         /*  77 */ _UPC,         /*  78 */ _UPC,         /*  79 */ _UPC,
    /*  80 */ _UPC,         /*  81 */ _UPC,         /*  82 */ _UPC,         /*  83 */ _UPC,
    /*  84 */ _UPC,         /*  85 */ _UPC,         /*  86 */ _UPC,         /*  87 */ _UPC,
    /*  88 */ _UPC,         /*  89 */ _UPC,         /*  90 */ _UPC,         /*  91 */ _PUN,
    /*  92 */ _PUN,         /*  93 */ _PUN,         /*  94 */ _PUN,         /*  95 */ _PUN,
    /*  96 */ _PUN,         /*  97 */ _LWR+_HEX,    /*  98 */ _LWR+_HEX,    /*  99 */ _LWR+_HEX,
    /* 100 */ _LWR+_HEX,    /* 101 */ _LWR+_HEX,    /* 102 */ _LWR+_HEX,    /* 103 */ _LWR,
    /* 104 */ _LWR,         /* 105 */ _LWR,         /* 106 */ _LWR,         /* 107 */ _LWR,
    /* 108 */ _LWR,         /* 109 */ _LWR,         /* 110 */ _LWR,         /* 111 */ _LWR,
    /* 112 */ _LWR,         /* 113 */ _LWR,         /* 114 */ _LWR,         /* 115 */ _LWR,
    /* 116 */ _LWR,         /* 117 */ _LWR,         /* 118 */ _LWR,         /* 119 */ _LWR,
    /* 120 */ _LWR,         /* 121 */ _LWR,         /* 122 */ _LWR,         /* 123 */ _PUN,
    /* 124 */ _PUN,         /* 125 */ _PUN,         /* 126 */ _PUN,         /* 127 */ _CTR,
    /* 128 */ 0,            /* 129 */ 0,            /* 130 */ 0,            /* 131 */ 0,
    /* 132 */ 0,            /* 133 */ 0,            /* 134 */ 0,            /* 135 */ 0,
    /* 136 */ 0,            /* 137 */ 0,            /* 138 */ 0,            /* 139 */ 0,
    /* 140 */ 0,            /* 141 */ 0,            /* 142 */ 0,            /* 143 */ 0,
    /* 144 */ 0,            /* 145 */ 0,            /* 146 */ 0,            /* 147 */ 0,
    /* 148 */ 0,            /* 149 */ 0,            /* 150 */ 0,            /* 151 */ 0,
    /* 152 */ 0,            /* 153 */ 0,            /* 154 */ 0,            /* 155 */ 0,
    /* 156 */ 0,            /* 157 */ 0,            /* 158 */ 0,            /* 159 */ 0,
    /* 160 */ 0,            /* 161 */ _PUN,         /* 162 */ _PUN,         /* 163 */ _PUN,
    /* 164 */ _PUN,         /* 165 */ _PUN,         /* 166 */ _PUN,         /* 167 */ _PUN,
    /* 168 */ _PUN,         /* 169 */ _PUN,         /* 170 */ _PUN,         /* 171 */ _PUN,
    /* 172 */ _PUN,         /* 173 */ _PUN,         /* 174 */ _PUN,         /* 175 */ _PUN,
    /* 176 */ _PUN,         /* 177 */ _PUN,         /* 178 */ _PUN,         /* 179 */ _PUN,
    /* 180 */ _PUN,         /* 181 */ _PUN,         /* 182 */ _PUN,         /* 183 */ _PUN,
    /* 184 */ _PUN,         /* 185 */ _PUN,         /* 186 */ _PUN,         /* 187 */ _PUN,
    /* 188 */ _PUN,         /* 189 */ _PUN,         /* 190 */ _PUN,         /* 191 */ _PUN,
    /* 192 */ _PUN,         /* 193 */ _PUN,         /* 194 */ _PUN,         /* 195 */ _PUN,
    /* 196 */ _PUN,         /* 197 */ _PUN,         /* 198 */ _PUN,         /* 199 */ _PUN,
    /* 200 */ _PUN,         /* 201 */ _PUN,         /* 202 */ _PUN,         /* 203 */ _PUN,
    /* 204 */ _PUN,         /* 205 */ _PUN,         /* 206 */ _PUN,         /* 207 */ _PUN,
    /* 208 */ _PUN,         /* 209 */ _PUN,         /* 210 */ _PUN,         /* 211 */ _PUN,
    /* 212 */ _PUN,         /* 213 */ _PUN,         /* 214 */ _PUN,         /* 215 */ _PUN,
    /* 216 */ _PUN,         /* 217 */ _PUN,         /* 218 */ _PUN,         /* 219 */ _PUN,
    /* 220 */ _PUN,         /* 221 */ _PUN,         /* 222 */ _PUN,         /* 223 */ _PUN,
    /* 224 */ 0,            /* 225 */ 0,            /* 226 */ 0,            /* 227 */ 0,
    /* 228 */ 0,            /* 229 */ 0,            /* 230 */ 0,            /* 231 */ 0,
    /* 232 */ 0,            /* 233 */ 0,            /* 234 */ 0,            /* 235 */ 0,
    /* 236 */ 0,            /* 237 */ 0,            /* 238 */ 0,            /* 239 */ 0,
    /* 240 */ 0,            /* 241 */ 0,            /* 242 */ 0,            /* 243 */ 0,
    /* 244 */ 0,            /* 245 */ 0,            /* 246 */ 0,            /* 247 */ 0,
    /* 248 */ 0,            /* 249 */ 0,            /* 250 */ 0,            /* 251 */ 0,
    /* 252 */ 0,            /* 253 */ 0,            /* 254 */ 0,            /* 255 */ 0
};

const char *__ctype_ptr__ = (const char *)_ctype_;

#if 0

//-----------------------------------------------------------------------------
// function: islower/tolower/isupper/toupper
//-----------------------------------------------------------------------------

int __islower(int c)
{
    return ((c >= 'a') && (c <= 'z'));
}

int __isupper(int c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

int __tolower(int c)
{
    return __isupper(c) ? (c+32) : c;
}

int __toupper(int c)
{
    return __islower(c) ? (c-32) : c;
}

//-----------------------------------------------------------------------------
// function: isxdigit
//-----------------------------------------------------------------------------

int __isxdigit(char c)
{
    switch (c)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return 1;
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// function: isprint
//-----------------------------------------------------------------------------

int is_print(char c)
{
    return ((c >= ' ') && (c <= '~'));
}

#endif

//-----------------------------------------------------------------------------
// function: strcpy
//-----------------------------------------------------------------------------

char *strcpy(char *s1, const char *s2)
{
    char *s = s1;

    while ((*s1++ = *s2++));
    return s;
}

//-----------------------------------------------------------------------------
// function: strncpy
//-----------------------------------------------------------------------------

char *strncpy(char *s1, const char *s2, size_t n)
{
    while (n--)
        s1[n] = s2[n];
    return s1;
}

//-----------------------------------------------------------------------------
// function: strlen
//-----------------------------------------------------------------------------

size_t strlen(const char *s)
{
    int i = 0;

    while (*s++) i++;
    return i;
}

//-----------------------------------------------------------------------------
// function: strnlen
//-----------------------------------------------------------------------------

size_t strnlen(const char *s, size_t n)
{
    int i = 0;

    while ((*s++) && (i <= n)) i++;
    return i;
}

//-----------------------------------------------------------------------------
// function: strcmp
//-----------------------------------------------------------------------------

int strcmp(const char *s1, const char *s2)
{
    const unsigned char *l = (const unsigned char *)s1;
    const unsigned char *r = (const unsigned char *)s2;

    while (*l && *l == *r)
    {
        ++l;
        ++r;
    }

    return (*l > *r) - (*r  > *l);
}

//-----------------------------------------------------------------------------
// function: strncmp
//-----------------------------------------------------------------------------

int strncmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *l = (const unsigned char *)s1;
    const unsigned char *r = (const unsigned char *)s2;

    while (n--)
    {
        if ((*l != *r) || (!*l))
            return (*l > *r) - (*r  > *l);
        l++;
        r++;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// function: strchr
//-----------------------------------------------------------------------------

char *strchr(const char *s, int c)
{
    unsigned char* p = (unsigned char *)s;

    while (*p)
        if (*p++ == (unsigned char)c)
            return (char *)p-1;

    return NULL;
}

//-----------------------------------------------------------------------------
// function: strcat
//-----------------------------------------------------------------------------

char *strcat(char *__restrict__ s1, const char *__restrict__ s2)
{
    unsigned char* l = (unsigned char *)s1;
    const unsigned char *r = (const unsigned char *)s2;

    while (*l++);
    while ((*l++ = *r++));
    *l = '\0';

    return s1;
}

//-----------------------------------------------------------------------------
// function: strtol
//-----------------------------------------------------------------------------

#ifndef MAXINT
#define MAXINT(w)   (\
        ((w) == sizeof(char))  ? 0x7F :   \
        ((w) == sizeof(short)) ? 0x7FFF : \
        ((w) == sizeof(int))   ? 0x7FFFFFFF : 0 )
#endif

#define max_allowable(A)     (MAXINT(sizeof(int))/A - 1)

long strtol(const char *str, char **endptr, int base)
{
    long i = 0;
    int  s = 1;
    int  c;

    /* skip white space */
    while (isspace((int)(*str)))
    {
        str++;
    }

    /* sign flag check */
    if (*str == '+')
        str++;
    else if (*str == '-')
    {
        s = -1;
        str++;
    }

    if (*str == '0')
    {
        if (toupper((int)(*++str)) == 'X')
            base = 16,str++;
        else if (base == 0)
            base = 8;
    }

    if (base == 0)
        base = 10;

    if (base <= 10)
    {
        /* digit str to number */
        for (; isdigit((int)(*str)); str++)
        {
            if (i < max_allowable(base))
                i = i * base + (*str - '0');
            else
            {
                i = MAXINT(sizeof(int));
                errno = ERANGE;
            }
        }
    }
    else if (base > 10)
    {
        for (; (c = *str); str++)
        {
            if (isdigit(c))
                c = c - '0';
            else
            {
                c = toupper(c);

                if (c >= 'A' && c < ('A' - 10 + base))
                    c = c - 'A' + 10;
                else
                    break;
            }

            if (i < max_allowable(base))
                i = i * base + c;
            else
            {
                i = MAXINT(sizeof(int));
                errno = ERANGE;
            }
        }
    }
    else
        return 0;       /* negative base is not allowed */

    if (endptr)
        *endptr = (char *) str;

    if (s == -1)
        i = -i;

    return i;
}

//-----------------------------------------------------------------------------
// function: atoi
//-----------------------------------------------------------------------------

int atoi(const char *s)
{
    return (int)strtol(s, (char **)0, 10);
}

//-----------------------------------------------------------------------------
// function: memchr
//-----------------------------------------------------------------------------

void *memchr(const void *s, int c, size_t n)
{
    char *mys = (char *)s;
    while ((int)--n >= 0)
        if (*mys++ == c)
            return (void *) --mys;
    return NULL;
}

//-----------------------------------------------------------------------------
// function: memmove
//-----------------------------------------------------------------------------

void* memmove(void *s1, const void *s2, size_t n)
{
    unsigned char *d = (unsigned char *)s1;
    const unsigned char *s = (const unsigned char *)s2;

    if (d < s)
        return memcpy(s1, s2, n);

    while (n--)
        d[n] = s[n];

    return s1;
}

//-----------------------------------------------------------------------------
// function: memcmp
//-----------------------------------------------------------------------------

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *l = (const unsigned char *)s1;
    const unsigned char *r = (const unsigned char *)s2;

    while (n--)
    {
        if (*l++ != *r++)
            return l[-1] < r[-1] ? -1 : 1;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// function: memset
//-----------------------------------------------------------------------------

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

//-----------------------------------------------------------------------------
// function: memcpy
//-----------------------------------------------------------------------------

void *memcpy(void *__restrict__ s1, const void *__restrict__ s2, size_t n)
{
    unsigned char *d = (unsigned char *)s1;
    const unsigned char *s = (const unsigned char *)s2;

    while (n--)
        *d++ = *s++;

    return s1;
}

//-----------------------------------------------------------------------------
// function: fls
//-----------------------------------------------------------------------------

int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }

    return r;
}

//-----------------------------------------------------------------------------
// function: strncasecmp
//-----------------------------------------------------------------------------
/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const unsigned char charmap[] =
{
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strncasecmp(const char *s1, const char *s2, register size_t n)
{
    register unsigned char u1, u2;

    for (; n != 0; --n)
    {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (charmap[u1] != charmap[u2])
        {
            return charmap[u1] - charmap[u2];
        }
        if (u1 == '\0')
        {
            return 0;
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// function: strcasecmp
//-----------------------------------------------------------------------------

int strcasecmp(const char *s1, const char *s2)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    int result;
  
    if (p1 == p2)
        return 0;

    while ((result = tolower(*p1) - tolower(*p2++)) == 0)
        if (*p1++ == '\0')
            break;

    return result;
}

/*
 * @@ End
 */
