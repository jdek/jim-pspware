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
#include <math.h>
#include <stdlib.h>
#include "c4.h"
#include "memmap.h"
extern "C" {

short C4WFXVal;
short C4WFYVal;
short C4WFZVal;
short C4WFX2Val;
short C4WFY2Val;
short C4WFDist;
short C4WFScale;

#ifdef PSP
static int16 tanval;
static int16 c4x, c4y, c4z;
static int16 c4x2, c4y2, c4z2;

const short costbl[] = { 256,255,255,255,254,254,253,252,251,249,248,246,244,243,241,238,236,234,231,228,225,222,219,216,212,209,205,201,197,193,189,185,181,176,171,167,162,157,152,147,142,136,131,126,120,115,109,103, 97, 92, 86, 80, 74, 68, 62, 56, 49, 43, 37, 31, 25, 18, 12,  6,  0, -6,-12,-18,-25,-31,-37,-43,-49,-56,-62,-68,-74,-80,-86,-92,-97,-103,-109,-115,-120,-126,-131,-136,-142,-147,-152,-157,-162,-167,-171,-176,-181,-185,-189,-193,-197,-201,-205,-209,-212,-216,-219,-222,-225,-228,-231,-234,-236,-238,-241,-243,-244,-246,-248,-249,-251,-252,-253,-254,-254,-255,-255,-255,-256,-255,-255,-255,-254,-254,-253,-252,-251,-249,-248,-246,-244,-243,-241,-238,-236,-234,-231,-228,-225,-222,-219,-216,-212,-209,-205,-201,-197,-193,-189,-185,-181,-176,-171,-167,-162,-157,-152,-147,-142,-136,-131,-126,-120,-115,-109,-103,-97,-92,-86,-80,-74,-68,-62,-56,-49,-43,-37,-31,-25,-18,-12, -6,  0,  6, 12, 18, 25, 31, 37, 43, 49, 56, 62, 68, 74, 80, 86, 92, 97,103,109,115,120,126,131,136,142,147,152,157,162,167,171,176,181,185,189,193,197,201,205,209,212,216,219,222,225,228,231,234,236,238,241,243,244,246,248,249,251,252,253,254,254,255,255,255 };

const short atantbl[] = { 0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 16, 17, 18, 18, 19, 20, 20, 21, 21, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 34, 35, 36, 36, 37, 37, 38, 39, 39, 40, 40, 41, 42, 42, 43, 43, 44, 44, 45, 46, 46, 47, 47, 48, 49, 49, 50, 50, 51, 51, 52, 53, 53, 54, 54, 55, 55, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 62, 62, 63, 63, 64, 64, 65, 65, 66, 66, 67, 67, 68, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80, 81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 91, 91, 92, 92, 92, 93, 93, 94, 94, 95, 95, 96, 96, 96, 97, 97, 98, 98, 99, 99, 99, 100, 100, 101, 101, 101, 102, 102, 103, 103, 104, 104, 104, 105, 105, 106, 106, 106, 107, 107, 108, 108, 108, 109, 109, 109, 110, 110, 111, 111, 111, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116, 117, 117, 117, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 127, 127 };

int16 _atan2 (short x, short y)
{
	int x1,y1;
	x1 = abs (x);
	y1 = abs (y);
	
	if (x == 0) {
		if (y >= 0) return 127;
		else return -127;
	}
	if (y == 0) return 0;
	
	
	if ( ((x >= 0) && (y >= 0)) || ( (x < 0) && (y < 0)) ) {
		if (x1 > y1) {
			return atantbl[(int8)((y1 << 8) / x1)];
		} else {
			return atantbl[(int8)((x1 << 8) / y1)];
		}
	} else {
		if (x1 > y1) {
			return -atantbl[(int8)((y1 << 8) / x1)];
		} else {
			return -atantbl[(int8)((x1 << 8) / y1)];
		}
	}
}

short _cos (int16 p)
{
	return costbl[(int8)(p >> 8)];
}

short _sin (int16 p)
{
	return costbl[(int8)(((p >> 8)+192) & 0xff)];
}

int32 _isqrt(int32 x)
{
	int32 s, t;

	if (x <= 0) return 0;

	s = 1;  t = x;
	while (s < t) {  s <<= 1;  t >>= 1;  }
	do {
		t = s;
		s = (x / s + s) >> 1;
	} while (s < t);

	return t;
}

#else
static double tanval;
static double c4x, c4y, c4z;
static double c4x2, c4y2, c4z2;
#endif // PSP


void C4TransfWireFrame ()
{
#ifdef PSP
    c4x = C4WFXVal;
    c4y = C4WFYVal;
    c4z = C4WFZVal - 0x95;
    
    // Rotate X
    tanval = (- C4WFX2Val * 3141 / 250);
    c4y2 = (c4y * _cos (tanval) - c4z * _sin (tanval)) >> 8;
    c4z2 = (c4y * _sin (tanval) + c4z * _cos (tanval)) >> 8;
    
    // Rotate Y
    tanval = (- C4WFY2Val * 3141 / 250);
    c4x2 = (c4x * _cos (tanval) + c4z2 * _sin (tanval)) >> 8;
    c4z = (c4x * - _sin (tanval) + c4z2 * _cos (tanval)) >> 8;
    
    // Rotate Z
    tanval = (- C4WFDist * 3141 / 250);
    c4x = (c4x2 * _cos (tanval) - c4y2 * _sin (tanval)) >> 8;
    c4y = (c4x2 * _sin (tanval) + c4y2 * _cos (tanval)) >> 8;
    
    // Scale
    C4WFXVal = (short)(c4x*C4WFScale/(0x90*(c4z+0x95))*0x95);
    C4WFYVal = (short)(c4y*C4WFScale/(0x90*(c4z+0x95))*0x95);

#else
    c4x = (double) C4WFXVal;
    c4y = (double) C4WFYVal;
    c4z = (double) C4WFZVal - 0x95;
    
    // Rotate X
    tanval = -(double) C4WFX2Val * 3.14159265 * 2 / 128;
    c4y2 = c4y * cos (tanval) - c4z * sin (tanval);
    c4z2 = c4y * sin (tanval) + c4z * cos (tanval);
    
    // Rotate Y
    tanval = -(double)C4WFY2Val*3.14159265*2/128;
    c4x2 = c4x * cos (tanval) + c4z2 * sin (tanval);
    c4z = c4x * - sin (tanval) + c4z2 * cos (tanval);
    
    // Rotate Z
    tanval = -(double) C4WFDist * 3.14159265*2 / 128;
    c4x = c4x2 * cos (tanval) - c4y2 * sin (tanval);
    c4y = c4x2 * sin (tanval) + c4y2 * cos (tanval);
    
    // Scale
    C4WFXVal = (short) (c4x*(double)C4WFScale/(0x90*(c4z+0x95))*0x95);
    C4WFYVal = (short) (c4y*(double)C4WFScale/(0x90*(c4z+0x95))*0x95);

#endif // PSP
}

void C4TransfWireFrame2 ()
{
#ifdef PSP
    c4x = C4WFXVal;
    c4y = C4WFYVal;
    c4z = C4WFZVal;
    
    // Rotate X
    tanval = (- C4WFX2Val * 3141 / 250);
    c4y2 = (c4y * _cos (tanval) - c4z * _sin (tanval)) >> 8;
    c4z2 = (c4y * _sin (tanval) + c4z * _cos (tanval)) >> 8;
    
    // Rotate Y
    tanval = (- C4WFY2Val * 3141 / 250);
    c4x2 = (c4x * _cos (tanval) + c4z2 * _sin (tanval)) >> 8;
    c4z = (c4x * - _sin (tanval) + c4z2 * _cos (tanval)) >> 8;
    
    // Rotate Z
    tanval = (- C4WFDist * 3141 / 250);
    c4x = (c4x2 * _cos (tanval) - c4y2 * _sin (tanval)) >> 8;
    c4y = (c4x2 * _sin (tanval) + c4y2 * _cos (tanval)) >> 8;
    
    // Scale
    C4WFXVal =(short)(c4x * C4WFScale / 0x100);
    C4WFYVal =(short)(c4y * C4WFScale / 0x100);

#else
    c4x = (double)C4WFXVal;
    c4y = (double)C4WFYVal;
    c4z = (double)C4WFZVal;
    
    // Rotate X
    tanval = -(double) C4WFX2Val * 3.14159265 * 2 / 128;
    c4y2 = c4y * cos (tanval) - c4z * sin (tanval);
    c4z2 = c4y * sin (tanval) + c4z * cos (tanval);
    
    // Rotate Y
    tanval = -(double) C4WFY2Val * 3.14159265 * 2 / 128;
    c4x2 = c4x * cos (tanval) + c4z2 * sin (tanval);
    c4z = c4x * -sin (tanval) + c4z2 * cos (tanval);
    
    // Rotate Z
    tanval = -(double)C4WFDist * 3.14159265 * 2 / 128;
    c4x = c4x2 * cos (tanval) - c4y2 * sin (tanval);
    c4y = c4x2 * sin (tanval) + c4y2 * cos (tanval);
    
    // Scale
    C4WFXVal =(short)(c4x * (double)C4WFScale / 0x100);
    C4WFYVal =(short)(c4y * (double)C4WFScale / 0x100);
#endif // PSP
}

void C4CalcWireFrame ()
{
    C4WFXVal = C4WFX2Val - C4WFXVal;
    C4WFYVal = C4WFY2Val - C4WFYVal;
#ifdef PSP
    if (abs (C4WFXVal) > abs (C4WFYVal))
    {
        C4WFDist = abs (C4WFXVal) + 1;
        C4WFYVal = (short) ( (C4WFYVal << 8) / abs (C4WFXVal) );
        if (C4WFXVal < 0)
            C4WFXVal = -256;
        else 
            C4WFXVal = 256;
    }
    else
    {
        if (C4WFYVal != 0) 
        {
            C4WFDist = abs(C4WFYVal)+1;
            C4WFXVal = (short) ( (C4WFXVal << 8) / abs (C4WFYVal) );
            if (C4WFYVal < 0)
                C4WFYVal = -256;
            else 
                C4WFYVal = 256;
        }
        else 
            C4WFDist = 0;
    }
#else
    if (abs (C4WFXVal) > abs (C4WFYVal))
    {
        C4WFDist = abs (C4WFXVal) + 1;
        C4WFYVal = (short) (256 * (double) C4WFYVal / abs (C4WFXVal));
        if (C4WFXVal < 0)
            C4WFXVal = -256;
        else 
            C4WFXVal = 256;
    }
    else
    {
        if (C4WFYVal != 0) 
        {
            C4WFDist = abs(C4WFYVal)+1;
            C4WFXVal = (short) (256 * (double)C4WFXVal / abs (C4WFYVal));
            if (C4WFYVal < 0)
                C4WFYVal = -256;
            else 
                C4WFYVal = 256;
        }
        else 
            C4WFDist = 0;
    }
#endif // PSP
}

short C41FXVal;
short C41FYVal;
short C41FAngleRes;
short C41FDist;
short C41FDistVal;

void C4Op1F ()
{
    if (C41FXVal == 0) 
    {
        if (C41FYVal > 0) 
            C41FAngleRes = 0x80;
        else 
            C41FAngleRes = 0x180;
    }
    else 
    {

#ifdef PSP
        C41FAngleRes = (short) (_atan2 (C41FYVal, C41FXVal));
#else
        tanval = (double) C41FYVal / C41FXVal;
        C41FAngleRes = (short) (atan (tanval) / (3.141592675 * 2) * 512);
#endif // PSP
        C41FAngleRes = C41FAngleRes;
        if (C41FXVal< 0) 
            C41FAngleRes += 0x100;
        C41FAngleRes &= 0x1FF;
    }
}

void C4Op15()
{
#ifdef PSP
    tanval = (int16)_isqrt ((int32) C41FYVal * C41FYVal + (int32) C41FXVal * C41FXVal);
    C41FDist = tanval;
#else
    tanval = sqrt ((double) C41FYVal * C41FYVal + (double) C41FXVal * C41FXVal);
    C41FDist = (short) tanval;
#endif // PSP
}

void C4Op0D()
{
#ifdef PSP
    tanval = (int16)_isqrt ((int32) C41FYVal * C41FYVal + (int32) C41FXVal * C41FXVal);
    tanval = C41FDistVal / tanval;
    C41FYVal = (short) (C41FYVal * tanval * 99 / 100);
    C41FXVal = (short) (C41FXVal * tanval * 98 / 100);
#else
    tanval = sqrt ((double) C41FYVal * C41FYVal + (double) C41FXVal * C41FXVal);
    tanval = C41FDistVal / tanval;
    C41FYVal = (short) (C41FYVal * tanval * 0.99);
    C41FXVal = (short) (C41FXVal * tanval * 0.98);
#endif // PSP
}

#ifdef ZSNES_C4
void C4LoaDMem(char *C4RAM)
{
  memmove(C4RAM+(READ_WORD(C4RAM+0x1f45)&0x1fff), 
          S9xGetMemPointer(READ_3WORD(C4RAM+0x1f40)),
          READ_WORD(C4RAM+0x1f43));
}
#endif
}//end extern C

