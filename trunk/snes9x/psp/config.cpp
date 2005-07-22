/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <psptypes.h>
#include <pspiofilemgr.h>

#include <stdarg.h>
#include <stddef.h>

/* std I/O buffer type constants. */
#define STD_IOBUF_TYPE_NONE            0
#define STD_IOBUF_TYPE_GE              1
#define STD_IOBUF_TYPE_MS			   2
#define STD_IOBUF_TYPE_UMD	           4
#define STD_IOBUF_TYPE_HOST            8
#define STD_IOBUF_TYPE_STDOUTHOST     16

#define _NFILE 16

#define _IOEOF                         0x0020
#define _IOERR                         0x0040

#define _IOREAD                        0x0001
#define _IOWRT                         0x0002
#define _IORW                          0x0200
#define _IOMYBUF                       0x0010

/* ensure FILE is defined. */
/* This is specific to psplibc, so we have to make sure it doesn't conflict with
   newlib's FILE definition. */;
#ifndef __FILE_DEFINED
#define __FILE_DEFINED
typedef struct {
        int  type;
        int  fd;
        int  cnt;
        int  flag;
        int  has_putback;
        u8   putback;
} __psplibc_FILE;
#endif // __FILE_DEFINED

/* Override newlib's definition of a FILE. */
#define LOCAL_FILE(f) ((__psplibc_FILE *) (f))

extern FILE __iob[_NFILE];


extern "C" {
void __fout(char *zNewText,int nNewChar,void *arg)
{
	fwrite(zNewText,1,nNewChar,(FILE*)arg);
}

FILE *fdopen(int fd, const char *mode)
{
  FILE *ret = NULL;
  int  flag = 0, i, iomode = 0;

  /* ensure valid descriptor, and that mode is not a NULL string. */
  if (fd >= 0) {
    if ((mode != NULL) && (*mode != '\0')) {
      /* test the file mode. */
      switch(*mode++) {
        case 'r':
          flag = _IOREAD;
          iomode = PSP_O_RDONLY;
          break;
        case 'w':
          flag = _IOWRT;
          iomode = (PSP_O_WRONLY | PSP_O_CREAT);
          break;
        case 'a':
          flag = _IORW;
          iomode = PSP_O_APPEND;
          break;
      }
      /* test the extended file mode. */
      for (; (*mode++ != '\0'); ) {
        switch(*mode) {
          case 'b':
            continue;
          case '+':
            flag |= (_IOREAD | _IOWRT);
            iomode |= (PSP_O_RDWR | PSP_O_CREAT | PSP_O_TRUNC);
            continue;
          default:
            break;
        }
      }
      /* search for an available fd slot. */
      for (i = 2; i < _NFILE; ++i) if (LOCAL_FILE(&__iob[i])->fd < 0) break;
      if (i < _NFILE) {
        /* attempt to open the fname file. */
        LOCAL_FILE(&__iob[i])->type = STD_IOBUF_TYPE_NONE;
        LOCAL_FILE(&__iob[i])->fd = fd;
        LOCAL_FILE(&__iob[i])->cnt = 0;
        LOCAL_FILE(&__iob[i])->flag = flag;
        LOCAL_FILE(&__iob[i])->has_putback = 0;
        ret = (__iob + i);
      }
    }
  }
  return (ret);
}

#define BUFSIZE 100  /* Size of the output buffer */

enum e_type {    /* The type of the format field */
   RADIX,            /* Integer types.  %d, %x, %o, and so forth */
   FLOAT,            /* Floating point.  %f */
   EXP,              /* Exponentional notation. %e and %E */
   GENERIC,          /* Floating or exponential, depending on exponent. %g */
   SIZE,             /* Return number of characters processed so far. %n */
   STRING,           /* Strings. %s */
   PERCENT,          /* Percent symbol. %% */
   CHAR,             /* Characters. %c */
   ERROR,            /* Used to indicate no such conversion type */
/* The rest are extensions, not normally found in printf() */
   CHARLIT,          /* Literal characters.  %' */
   SEEIT,            /* Strings with visible control characters. %S */
   MEM_STRING,       /* A string which should be deleted after use. %z */
   ORDINAL,          /* 1st, 2nd, 3rd and so forth */
};


typedef struct s_info {   /* Information about each format field */
  int  fmttype;              /* The format field code letter */
  int  base;                 /* The base for radix conversion */
  char *charset;             /* The character set for conversion */
  int  flag_signed;          /* Is the quantity signed? */
  char *prefix;              /* Prefix on non-zero values in alt format */
  enum e_type type;          /* Conversion paradigm */
} info;

#define MAXDIG 20

static info fmtinfo[] = {
  { 'd',  10,  "0123456789",       1,    0, RADIX,      },
  { 's',   0,  0,                  0,    0, STRING,     },
  { 'S',   0,  0,                  0,    0, SEEIT,      },
  { 'z',   0,  0,                  0,    0, MEM_STRING, },
  { 'c',   0,  0,                  0,    0, CHAR,       },
  { 'o',   8,  "01234567",         0,  "0", RADIX,      },
  { 'u',  10,  "0123456789",       0,    0, RADIX,      },
  { 'x',  16,  "0123456789abcdef", 0, "x0", RADIX,      },
  { 'X',  16,  "0123456789ABCDEF", 0, "X0", RADIX,      },
  { 'r',  10,  "0123456789",       0,    0, ORDINAL,    },
  { 'f',   0,  0,                  1,    0, FLOAT,      },
  { 'e',   0,  "e",                1,    0, EXP,        },
  { 'E',   0,  "E",                1,    0, EXP,        },
  { 'g',   0,  "e",                1,    0, GENERIC,    },
  { 'G',   0,  "E",                1,    0, GENERIC,    },
  { 'i',  10,  "0123456789",       1,    0, RADIX,      },
  { 'n',   0,  0,                  0,    0, SIZE,       },
  { 'S',   0,  0,                  0,    0, SEEIT,      },
  { '%',   0,  0,                  0,    0, PERCENT,    },
  { 'b',   2,  "01",               0, "b0", RADIX,      }, /* Binary notation */
  { 'p',  16,  "0123456789ABCDEF", 0, "x0", RADIX,      }, /* Pointers */
  { '\'',  0,  0,                  0,    0, CHARLIT,    }, /* Literal char */
};

#define NINFO  (sizeof(fmtinfo)/sizeof(info))  /* Size of the fmtinfo table */

static int getdigit(long double *val, int *cnt){
  int digit;
  long double d;
  if( (*cnt)++ >= MAXDIG ) return '0';
  digit = (int)*val;
  d = digit;
  digit += '0';
  *val = (*val - d)*10.0;
  return digit;
}

extern int isdigit(int __c);

int vxprintf(void (*func)(char*,int,void*),void *arg,const char *format,va_list ap)
{
  register const char *fmt; /* The format string. */
  register int c;           /* Next character in the format string */
  register char *bufpt;     /* Pointer to the conversion buffer */
  register int  precision;  /* Precision of the current field */
  register int  length;     /* Length of the field */
  register int  idx;        /* A general purpose loop counter */
  int count;                /* Total number of characters output */
  int width;                /* Width of the current field */
  int flag_leftjustify;     /* True if "-" flag is present */
  int flag_plussign;        /* True if "+" flag is present */
  int flag_blanksign;       /* True if " " flag is present */
  int flag_alternateform;   /* True if "#" flag is present */
  int flag_zeropad;         /* True if field width constant starts with zero */
  int flag_long;            /* True if "l" flag is present */
  int flag_center;          /* True if "=" flag is present */
  unsigned long long longvalue;  /* Value for integer types */

  long double realvalue;    /* Value for real types */
  info *infop;              /* Pointer to the appropriate info structure */
  char buf[BUFSIZE];        /* Conversion buffer */
  char prefix;              /* Prefix character.  "+" or "-" or " " or '\0'. */
  int  errorflag = 0;       /* True if an error is encountered */
  enum e_type xtype;        /* Conversion paradigm */
  char *zMem = 0;           /* String to be freed */
  static char spaces[] =
     "                                                    ";
#define SPACESIZE (sizeof(spaces)-1)
#ifndef NOFLOATINGPOINT
  int  exp;                 /* exponent of real numbers */
  long double rounder;      /* Used for rounding floating point values */
  int flag_dp;              /* True if decimal point should be shown */
  int flag_rtz;             /* True if trailing zeros should be removed */
  int flag_exp;             /* True to force display of the exponent */
  int nsd;                  /* Number of significant digits returned */
#endif

  fmt = format;                     /* Put in a register for speed */
  count = length = 0;
  bufpt = 0;
  for(; (c=(*fmt))!=0; ++fmt){
    if( c!='%' ){
      register int amt;
      bufpt = (char *)fmt;
      amt = 1;
      while( (c=(*++fmt))!='%' && c!=0 ) amt++;
      (*func)(bufpt,amt,arg);
      count += amt;
      if( c==0 ) break;
    }
    if( (c=(*++fmt))==0 ){
      errorflag = 1;
      (*func)("%",1,arg);
      count++;
      break;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_plussign = flag_blanksign =
     flag_alternateform = flag_zeropad = flag_center = 0;
    do{
      switch( c ){
        case '-':   flag_leftjustify = 1;     c = 0;   break;
        case '+':   flag_plussign = 1;        c = 0;   break;
        case ' ':   flag_blanksign = 1;       c = 0;   break;
        case '#':   flag_alternateform = 1;   c = 0;   break;
        case '0':   flag_zeropad = 1;         c = 0;   break;
        case '=':   flag_center = 1;          c = 0;   break;
        default:                                       break;
      }
    }while( c==0 && (c=(*++fmt))!=0 );
    if( flag_center ) flag_leftjustify = 0;
    /* Get the field width */
    width = 0;
    if( c=='*' ){
      width = va_arg(ap,int);
      if( width<0 ){
        flag_leftjustify = 1;
        width = -width;
      }
      c = *++fmt;
    }else{
      while( isdigit(c) ){
        width = width*10 + c - '0';
        c = *++fmt;
      }
    }
    if( width > BUFSIZE-10 ){
      width = BUFSIZE-10;
    }
    /* Get the precision */
    if( c=='.' ){
      precision = 0;
      c = *++fmt;
      if( c=='*' ){
        precision = va_arg(ap,int);
#ifndef COMPATIBILITY
        /* This is sensible, but SUN OS 4.1 doesn't do it. */
        if( precision<0 ) precision = -precision;
#endif
        c = *++fmt;
      }else{
        while( isdigit(c) ){
          precision = precision*10 + c - '0';
          c = *++fmt;
        }
      }
      /* Limit the precision to prevent overflowing buf[] during conversion */
      if( precision>BUFSIZE-40 ) precision = BUFSIZE-40;
    }else{
      precision = -1;
    }
    /* Get the conversion type modifier */
    if( c=='l' ){
      flag_long = 1;
      c = *++fmt;
      if( c == 'l' ){
	flag_long = 2;
	c = *++fmt;
      }
    }else{
      flag_long = 0;
    }
    /* Fetch the info entry for the field */
    infop = 0;
    for(idx=0; idx<NINFO; idx++){
      if( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        break;
      }
    }
    /* No info entry found.  It must be an error. */
    if( infop==0 ){
      xtype = ERROR;
    }else{
      xtype = (e_type)infop->type;
    }

    /*
    ** At this point, variables are initialized as follows:
    **
    **   flag_alternateform          TRUE if a '#' is present.
    **   flag_plussign               TRUE if a '+' is present.
    **   flag_leftjustify            TRUE if a '-' is present or if the
    **                               field width was negative.
    **   flag_zeropad                TRUE if the width began with 0.
    **   flag_long                   TRUE if the letter 'l' (ell) prefixed
    **                               the conversion character.
    **   flag_blanksign              TRUE if a ' ' is present.
    **   width                       The specified field width.  This is
    **                               always non-negative.  Zero is the default.
    **   precision                   The specified precision.  The default
    **                               is -1.
    **   xtype                       The class of the conversion.
    **   infop                       Pointer to the appropriate info struct.
    */
    switch( xtype ){
      case ORDINAL:
      case RADIX:
        if(( flag_long>1 )&&( infop->flag_signed )){
	    signed long long t = va_arg(ap,signed long long);
	    longvalue = t;
	}else if(( flag_long>1 )&&( !infop->flag_signed )){
	    unsigned long long t = va_arg(ap,unsigned long long);
	    longvalue = t;
	}else if(( flag_long )&&( infop->flag_signed )){
	    signed long t = va_arg(ap,signed long);
	    longvalue = t;
	}else if(( flag_long )&&( !infop->flag_signed )){
	    unsigned long t = va_arg(ap,unsigned long);
	    longvalue = t;
	}else if(( !flag_long )&&( infop->flag_signed )){
	    signed int t = va_arg(ap,signed int) & ((unsigned long) 0xffffffff);
	    longvalue = t;
	}else{
	    unsigned int t = va_arg(ap,unsigned int) & ((unsigned long) 0xffffffff);
	    longvalue = t;
	}
#ifdef COMPATIBILITY
        /* For the format %#x, the value zero is printed "0" not "0x0".
        ** I think this is stupid. */
        if( longvalue==0 ) flag_alternateform = 0;
#else
        /* More sensible: turn off the prefix for octal (to prevent "00"),
        ** but leave the prefix for hex. */
        if( longvalue==0 && infop->base==8 ) flag_alternateform = 0;
#endif
        if( infop->flag_signed ){
          if( *(long long*)&longvalue<0 ){
	    longvalue = -*(long long*)&longvalue;
            prefix = '-';
          }else if( flag_plussign )  prefix = '+';
          else if( flag_blanksign )  prefix = ' ';
          else                       prefix = 0;
        }else                        prefix = 0;
        if( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
	}
        bufpt = &buf[BUFSIZE];
        if( xtype==ORDINAL ){
          long a,b;
          a = longvalue%10;
          b = longvalue%100;
          bufpt -= 2;
          if( a==0 || a>3 || (b>10 && b<14) ){
            bufpt[0] = 't';
            bufpt[1] = 'h';
          }else if( a==1 ){
            bufpt[0] = 's';
            bufpt[1] = 't';
          }else if( a==2 ){
            bufpt[0] = 'n';
            bufpt[1] = 'd';
          }else if( a==3 ){
            bufpt[0] = 'r';
            bufpt[1] = 'd';
          }
        }
        {
          register char *cset;      /* Use registers for speed */
          register int base;
          cset = infop->charset;
          base = infop->base;
          do{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }while( longvalue>0 );
	}
        length = (int)(&buf[BUFSIZE]-bufpt);
	if(infop->fmttype == 'p')
        {
		precision = 8;
		flag_alternateform = 1;
        }

        for(idx=precision-length; idx>0; idx--){
          *(--bufpt) = '0';                             /* Zero pad */
	}
        if( prefix ) *(--bufpt) = prefix;               /* Add sign */
        if( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          char *pre, x;
          pre = infop->prefix;
          if( *bufpt!=pre[0] ){
            for(pre=infop->prefix; (x=(*pre))!=0; pre++) *(--bufpt) = x;
	  }
        }

        length = (int)(&buf[BUFSIZE]-bufpt);
        break;
      case FLOAT:
      case EXP:
      case GENERIC:
        realvalue = va_arg(ap,double);
#ifndef NOFLOATINGPOINT
        if( precision<0 ) precision = 6;         /* Set default precision */
        if( precision>BUFSIZE-10 ) precision = BUFSIZE-10;
        if( realvalue<0.0 ){
          realvalue = -realvalue;
          prefix = '-';
	}else{
          if( flag_plussign )          prefix = '+';
          else if( flag_blanksign )    prefix = ' ';
          else                         prefix = 0;
	}
        if( infop->type==GENERIC && precision>0 ) precision--;
        rounder = 0.0;
#ifdef COMPATIBILITY
        /* Rounding works like BSD when the constant 0.4999 is used.  Wierd! */
        for(idx=precision, rounder=0.4999; idx>0; idx--, rounder*=0.1);
#else
        /* It makes more sense to use 0.5 */
        if( precision>MAXDIG-1 ) idx = MAXDIG-1;
        else                     idx = precision;
        for(rounder=0.5; idx>0; idx--, rounder*=0.1);
#endif
        if( infop->type==FLOAT ) realvalue += rounder;
        /* Normalize realvalue to within 10.0 > realvalue >= 1.0 */
        exp = 0;
        if( realvalue>0.0 ){
          int k = 0;
          while( realvalue>=1e8 && k++<100 ){ realvalue *= 1e-8; exp+=8; }
          while( realvalue>=10.0 && k++<100 ){ realvalue *= 0.1; exp++; }
          while( realvalue<1e-8 && k++<100 ){ realvalue *= 1e8; exp-=8; }
          while( realvalue<1.0 && k++<100 ){ realvalue *= 10.0; exp--; }
          if( k>=100 ){
            bufpt = "NaN";
            length = 3;
            break;
          }
	}
        bufpt = buf;
        /*
        ** If the field type is GENERIC, then convert to either EXP
        ** or FLOAT, as appropriate.
        */
        flag_exp = xtype==EXP;
        if( xtype!=FLOAT ){
          realvalue += rounder;
          if( realvalue>=10.0 ){ realvalue *= 0.1; exp++; }
        }
        if( xtype==GENERIC ){
          flag_rtz = !flag_alternateform;
          if( exp<-4 || exp>precision ){
            xtype = EXP;
          }else{
            precision = precision - exp;
            xtype = FLOAT;
          }
	}else{
          flag_rtz = 0;
	}
        /*
        ** The "exp+precision" test causes output to be of type EXP if
        ** the precision is too large to fit in buf[].
        */
        nsd = 0;
        if( xtype==FLOAT && exp+precision<BUFSIZE-30 ){
          flag_dp = (precision>0 || flag_alternateform);
          if( prefix ) *(bufpt++) = prefix;         /* Sign */
          if( exp<0 )  *(bufpt++) = '0';            /* Digits before "." */
          else for(; exp>=0; exp--) *(bufpt++) = getdigit(&realvalue,&nsd);
          if( flag_dp ) *(bufpt++) = '.';           /* The decimal point */
          for(exp++; exp<0 && precision>0; precision--, exp++){
            *(bufpt++) = '0';
          }
          while( (precision--)>0 ) *(bufpt++) = getdigit(&realvalue,&nsd);
          *(bufpt--) = 0;                           /* Null terminate */
          if( flag_rtz && flag_dp ){     /* Remove trailing zeros and "." */
            while( bufpt>=buf && *bufpt=='0' ) *(bufpt--) = 0;
            if( bufpt>=buf && *bufpt=='.' ) *(bufpt--) = 0;
          }
          bufpt++;                            /* point to next free slot */
	}else{    /* EXP or GENERIC */
          flag_dp = (precision>0 || flag_alternateform);
          if( prefix ) *(bufpt++) = prefix;   /* Sign */
          *(bufpt++) = getdigit(&realvalue,&nsd);  /* First digit */
          if( flag_dp ) *(bufpt++) = '.';     /* Decimal point */
          while( (precision--)>0 ) *(bufpt++) = getdigit(&realvalue,&nsd);
          bufpt--;                            /* point to last digit */
          if( flag_rtz && flag_dp ){          /* Remove tail zeros */
            while( bufpt>=buf && *bufpt=='0' ) *(bufpt--) = 0;
            if( bufpt>=buf && *bufpt=='.' ) *(bufpt--) = 0;
          }
          bufpt++;                            /* point to next free slot */
          if( exp || flag_exp ){
            *(bufpt++) = infop->charset[0];
            if( exp<0 ){ *(bufpt++) = '-'; exp = -exp; } /* sign of exp */
            else       { *(bufpt++) = '+'; }
            if( exp>=100 ){
              *(bufpt++) = (exp/100)+'0';                /* 100's digit */
              exp %= 100;
  	    }
            *(bufpt++) = exp/10+'0';                     /* 10's digit */
            *(bufpt++) = exp%10+'0';                     /* 1's digit */
          }
	}
        /* The converted number is in buf[] and zero terminated. Output it.
        ** Note that the number is in the usual order, not reversed as with
        ** integer conversions. */
        length = (int)(bufpt-buf);
        bufpt = buf;

        /* Special case:  Add leading zeros if the flag_zeropad flag is
        ** set and we are not left justified */
        if( flag_zeropad && !flag_leftjustify && length < width){
          int i;
          int nPad = width - length;
          for(i=width; i>=nPad; i--){
            bufpt[i] = bufpt[i-nPad];
          }
          i = prefix!=0;
          while( nPad-- ) bufpt[i++] = '0';
          length = width;
        }
#endif
        break;
      case SIZE:
        *(va_arg(ap,int*)) = count;
        length = width = 0;
        break;
      case PERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        break;
      case CHARLIT:
      case CHAR:
        c = buf[0] = (xtype==CHAR ? va_arg(ap,int) : *++fmt);
        if( precision>=0 ){
          for(idx=1; idx<precision; idx++) buf[idx] = c;
          length = precision;
	}else{
          length =1;
	}
        bufpt = buf;
        break;
      case STRING:
      case MEM_STRING:
        zMem = bufpt = va_arg(ap,char*);
        if( bufpt==0 ) bufpt = "(null)";
        length = strlen(bufpt);
        if( precision>=0 && precision<length ) length = precision;
        break;
      case SEEIT:
        {
          int i;
          int c;
          char *arg = va_arg(ap,char*);
          for(i=0; i<BUFSIZE-1 && (c = *arg++)!=0; i++){
            if( c<0x20 || c>=0x7f ){
              buf[i++] = '^';
              buf[i] = (c&0x1f)+0x40;
            }else{
              buf[i] = c;
            }
          }
          bufpt = buf;
          length = i;
          if( precision>=0 && precision<length ) length = precision;
        }
        break;
      case ERROR:
        buf[0] = '%';
        buf[1] = c;
        errorflag = 0;
        idx = 1+(c!=0);
        (*func)("%",idx,arg);
        count += idx;
        if( c==0 ) fmt--;
        break;
    }/* End switch over the format type */
    /*
    ** The text of the conversion is pointed to by "bufpt" and is
    ** "length" characters long.  The field width is "width".  Do
    ** the output.
    */
    if( !flag_leftjustify ){
      register int nspace;
      nspace = width-length;
      if( nspace>0 ){
        if( flag_center ){
          nspace = nspace/2;
          width -= nspace;
          flag_leftjustify = 1;
	}
        count += nspace;
        while( nspace>=SPACESIZE ){
          (*func)(spaces,SPACESIZE,arg);
          nspace -= SPACESIZE;
        }
        if( nspace>0 ) (*func)(spaces,nspace,arg);
      }
    }
    if( length>0 ){
      (*func)(bufpt,length,arg);
      count += length;
    }
    if( xtype==MEM_STRING && zMem ){
      free(zMem);
    }
    if( flag_leftjustify ){
      register int nspace;
      nspace = width-length;
      if( nspace>0 ){
        count += nspace;
        while( nspace>=SPACESIZE ){
          (*func)(spaces,SPACESIZE,arg);
          nspace -= SPACESIZE;
        }
        if( nspace>0 ) (*func)(spaces,nspace,arg);
      }
    }
  }/* End for loop over the format string */
  return errorflag ? -1 : count;
} /* End of function */


/* The public interface routines */
int fprintf(FILE *pOut, const char *zFormat, ...){
  va_list ap;
  int retc;

  va_start(ap,zFormat);
  retc = vxprintf(__fout,pOut,zFormat,ap);
  va_end(ap);
  return retc;
}

int fputc(int c, FILE *stream)
{
  char ch;

  ch = (char)c;
  return ((fwrite(&ch, 1, 1, stream) == 1) ? 0 : EOF);
}

int fflush(FILE *stream)
{
  int ret = EOF; // Same as default case below.

  switch(LOCAL_FILE(stream)->type) {
    case STD_IOBUF_TYPE_GE:
    case STD_IOBUF_TYPE_STDOUTHOST:
      /* stdout & stderr are never buffered. */
    case STD_IOBUF_TYPE_UMD:
      /* cd-rom files are read-only so no write buffer to flush. */
      ret = 0;
      break;
    case STD_IOBUF_TYPE_MS:
      if (LOCAL_FILE(stream)->flag & (_IOWRT | _IORW)) {
        //if (ret != 0) ret = EOF;
		  /* Need to implement sync or something */
      }
      else ret = 0;
      break;
    case STD_IOBUF_TYPE_HOST:
      /* flush host file write buffer. */
      if (LOCAL_FILE(stream)->flag & (_IOWRT | _IORW)) ret = 0;
      else ret = 0;
      break;
    default:
      /* unknown/invalid I/O buffer type. */
      ret = EOF;
  }
  return (ret);
}

};

extern "C" {
int memcmp(const void *s1, const void *s2, size_t len)
{
	unsigned long int	a0, b0, res;
	int		i;

	i = 0;
	while ( len != 0 ){
		a0 = ((unsigned char*)s1)[i];
		b0 = ((unsigned char*)s2)[i];
		res = a0 - b0;
		if ( res != 0 )
			return res;
		len--;
    }

	return 0;
}

# define op_t	unsigned long int
# define OPSIZ	(sizeof(op_t))

void *memset(void *buf, int ch, size_t n)
{
	int		i;

	if (n == 0) return buf;

	if ( ch ){
		for ( i = 0; i < n; i++ ){
			((char*)buf)[i] = ch;
		}
	} else {
		long int dstp = (long int)buf;

		if ( n >= 8 ){
			size_t	xlen;

			while ( (dstp % OPSIZ) != 0 ){
				((char*)dstp)[0] = 0;
				dstp += 1;
				n -= 1;
			}

			xlen = n / (OPSIZ * 8);
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				((op_t*)dstp)[1] = 0;
				((op_t*)dstp)[2] = 0;
				((op_t*)dstp)[3] = 0;
				((op_t*)dstp)[4] = 0;
				((op_t*)dstp)[5] = 0;
				((op_t*)dstp)[6] = 0;
				((op_t*)dstp)[7] = 0;
				dstp += 8 * OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ * 8;

			xlen = n / OPSIZ;
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				dstp += OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ;
		}

		while ( n != 0 ){
			((char*)dstp)[0] = 0;
			dstp += 1;
			n -= 1;
		}
	}

	return buf;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	int		i;

	if (n == 0) return dest;

	for ( i = 0; i < n; i++ ){
		((char*)dest)[i] = ((char*)src)[i];
	}
	return dest;
}

void* memmove( void* dest, const void* src, size_t n )
{
	unsigned long int dstp = (long int) dest;
	unsigned long int srcp = (long int) src;
	int		i;

	if (n == 0) return dest;

	if ( dstp - srcp >= n ){
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[i] = ((char*)srcp)[i];
		}
	} else {
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[n - i - 1] = ((char*)srcp)[n - i - 1];
		}
	}
	return dest;
}
}
