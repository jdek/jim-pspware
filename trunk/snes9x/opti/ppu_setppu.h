#define USE_REGISTER

static inline void SetPPU_2100 (uint8 Byte)
{
	// Brightness and screen blank bit
	if (Byte != Memory.FillRAM [0x2100])
	{
		FLUSH_REDRAW ();
		if (PPU.Brightness != (Byte & 0xF))
		{
			IPPU.ColorsChanged = TRUE;
			IPPU.DirectColourMapsNeedRebuild = TRUE;
			PPU.Brightness = Byte & 0xF;
			S9xFixColourBrightness ();
			if (PPU.Brightness > IPPU.MaxBrightness)
				IPPU.MaxBrightness = PPU.Brightness;
		}
		if ((Memory.FillRAM[0x2100] & 0x80) != (Byte & 0x80))
		{
			IPPU.ColorsChanged = TRUE;
			PPU.ForcedBlanking = (Byte >> 7) & 1;
		}
		Memory.FillRAM[0x2100] = Byte;
	}
}

static inline void SetPPU_2101 (uint8 Byte)
{
	// Sprite (OBJ) tile address
	if (Byte != Memory.FillRAM [0x2101])
	{
		FLUSH_REDRAW ();
		PPU.OBJNameBase   = (Byte & 3) << 14;
		PPU.OBJNameSelect = ((Byte >> 3) & 3) << 13;
		PPU.OBJSizeSelect = (Byte >> 5) & 7;
		IPPU.OBJChanged = TRUE;
		Memory.FillRAM[0x2101] = Byte;
	}
}

static inline void SetPPU_2102 (uint8 Byte)
{
	// Sprite write address (low)
	PPU.OAMAddr = ((Memory.FillRAM[0x2103]&1)<<8) | Byte;
	PPU.OAMFlip = 2;
	PPU.OAMReadFlip = 0;
	PPU.SavedOAMAddr = PPU.OAMAddr;
	if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
	{
		PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
		IPPU.OBJChanged = TRUE;
#ifdef DEBUGGER
		missing.sprite_priority_rotation = 1;
#endif
	}
	Memory.FillRAM[0x2102] = Byte;
}

static inline void SetPPU_2103 (uint8 Byte)
{
	// Sprite register write address (high), sprite priority rotation
	// bit.
	PPU.OAMAddr = ((Byte&1)<<8) | Memory.FillRAM[0x2102];

	PPU.OAMPriorityRotation=(Byte & 0x80)? 1 : 0;
	if (PPU.OAMPriorityRotation)
	{
		if (PPU.FirstSprite != (PPU.OAMAddr >> 1))
		{
			PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
			IPPU.OBJChanged = TRUE;
#ifdef DEBUGGER
			missing.sprite_priority_rotation = 1;
#endif
		}
	} else {
		if (PPU.FirstSprite != 0)
		{
			PPU.FirstSprite = 0;
			IPPU.OBJChanged = TRUE;
#ifdef DEBUGGER
			missing.sprite_priority_rotation = 1;
#endif
		}
	}
	PPU.OAMFlip = 0;
	PPU.OAMReadFlip = 0;
	PPU.SavedOAMAddr = PPU.OAMAddr;
	Memory.FillRAM[0x2103] = Byte;
}

#ifdef USE_REGISTER
#define SetPPU_2104		REGISTER_2104
#else
static inline void SetPPU_2104 (uint8 Byte)
{
	// Sprite register write
	REGISTER_2104(Byte);
	Memory.FillRAM[0x2104] = Byte;
}
#endif // USE_REGISTER

static inline void SetPPU_2105 (uint8 Byte)
{
	// Screen mode (0 - 7), background tile sizes and background 3
	// priority
	if (Byte != Memory.FillRAM [0x2105])
	{
		FLUSH_REDRAW ();
		PPU.BG[0].BGSize = (Byte >> 4) & 1;
		PPU.BG[1].BGSize = (Byte >> 5) & 1;
		PPU.BG[2].BGSize = (Byte >> 6) & 1;
		PPU.BG[3].BGSize = (Byte >> 7) & 1;
		PPU.BGMode = Byte & 7;
		// BJ: BG3Priority only takes effect if BGMode==1 and the bit is set
		PPU.BG3Priority  = ((Byte & 0x0f) == 0x09);
#ifdef DEBUGGER
		missing.modes[PPU.BGMode] = 1;
#endif
		if(PPU.BGMode==5||PPU.BGMode==6)
			IPPU.Interlace = Memory.FillRAM[0x2133]&1;

		Memory.FillRAM[0x2105] = Byte;
	}
}

static inline void SetPPU_2106 (uint8 Byte)
{
	// Mosaic pixel size and enable
	if (Byte != Memory.FillRAM [0x2106])
	{
		FLUSH_REDRAW ();
#ifdef DEBUGGER
		if ((Byte & 0xf0) && (Byte & 0x0f))
			missing.mosaic = 1;
#endif
		PPU.Mosaic = (Byte >> 4) + 1;
		PPU.BGMosaic [0] = (Byte & 1) && PPU.Mosaic > 1;
		PPU.BGMosaic [1] = (Byte & 2) && PPU.Mosaic > 1;
		PPU.BGMosaic [2] = (Byte & 4) && PPU.Mosaic > 1;
		PPU.BGMosaic [3] = (Byte & 8) && PPU.Mosaic > 1;
		Memory.FillRAM[0x2106] = Byte;
	}
}

static inline void SetPPU_2107 (uint8 Byte)
{
	// [BG0SC]
	if (Byte != Memory.FillRAM [0x2107])
	{
		FLUSH_REDRAW ();
		PPU.BG[0].SCSize = Byte & 3;
		PPU.BG[0].SCBase = (Byte & 0x7c) << 8;
		Memory.FillRAM[0x2107] = Byte;
	}
}

static inline void SetPPU_2108 (uint8 Byte)
{
	// [BG1SC]
	if (Byte != Memory.FillRAM [0x2108])
	{
		FLUSH_REDRAW ();
		PPU.BG[1].SCSize = Byte & 3;
		PPU.BG[1].SCBase = (Byte & 0x7c) << 8;
		Memory.FillRAM[0x2108] = Byte;
	}
}

static inline void SetPPU_2109 (uint8 Byte)
{
	// [BG2SC]
	if (Byte != Memory.FillRAM [0x2109])
	{
		FLUSH_REDRAW ();
		PPU.BG[2].SCSize = Byte & 3;
		PPU.BG[2].SCBase = (Byte & 0x7c) << 8;
		Memory.FillRAM[0x2109] = Byte;
	}
}

static inline void SetPPU_210A (uint8 Byte)
{
	// [BG3SC]
	if (Byte != Memory.FillRAM [0x210a])
	{
		FLUSH_REDRAW ();
		PPU.BG[3].SCSize = Byte & 3;
		PPU.BG[3].SCBase = (Byte & 0x7c) << 8;
		Memory.FillRAM[0x210A] = Byte;
	}
}

static inline void SetPPU_210B (uint8 Byte)
{
	// [BG01NBA]
	if (Byte != Memory.FillRAM [0x210b])
	{
		FLUSH_REDRAW ();
		PPU.BG[0].NameBase    = (Byte & 7) << 12;
		PPU.BG[1].NameBase    = ((Byte >> 4) & 7) << 12;
		Memory.FillRAM[0x210B] = Byte;
	}
}

static inline void SetPPU_210C (uint8 Byte)
{
	// [BG23NBA]
	if (Byte != Memory.FillRAM [0x210c])
	{
		FLUSH_REDRAW ();
		PPU.BG[2].NameBase    = (Byte & 7) << 12;
		PPU.BG[3].NameBase    = ((Byte >> 4) & 7) << 12;
		Memory.FillRAM[0x210C] = Byte;
	}
}

//This is the Theme Park fix - it appears all these registers
//share a previous byte value for setting them.

static inline void SetPPU_210D (uint8 Byte)
{
	//TEST9        if(last_written != 0x210d) PPU.BGnxOFSbyte = 0;
	PPU.BG[0].HOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[0].HOffset = %04x  %d)\n", Byte, Address, PPU.BG[0].HOffset, CPU.V_Counter);
	Memory.FillRAM[0x210D] = Byte;
}

static inline void SetPPU_210E (uint8 Byte)
{
	PPU.BG[0].VOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[0].VOffset = %04x  %d)\n", Byte, Address, PPU.BG[0].VOffset, CPU.V_Counter);
	Memory.FillRAM[0x210E] = Byte;
}

static inline void SetPPU_210F (uint8 Byte)
{
	PPU.BG[1].HOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[1].HOffset = %04x  %d)\n", Byte, Address, PPU.BG[1].HOffset, CPU.V_Counter);
	Memory.FillRAM[0x210F] = Byte;
}

static inline void SetPPU_2110 (uint8 Byte)
{
	PPU.BG[1].VOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[1].VOffset = %04x  %d)\n", Byte, Address, PPU.BG[1].VOffset, CPU.V_Counter);
	Memory.FillRAM[0x2110] = Byte;
}

static inline void SetPPU_2111 (uint8 Byte)
{
	PPU.BG[2].HOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[2].HOffset = %04x  %d)\n", Byte, Address, PPU.BG[2].HOffset, CPU.V_Counter);
	Memory.FillRAM[0x2111] = Byte;
}

static inline void SetPPU_2112 (uint8 Byte)
{
	PPU.BG[2].VOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[2].VOffset = %04x  %d)\n", Byte, Address, PPU.BG[2].VOffset, CPU.V_Counter);
	Memory.FillRAM[0x2112] = Byte;
}

static inline void SetPPU_2113 (uint8 Byte)
{
	PPU.BG[3].HOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[3].HOffset = %04x  %d)\n", Byte, Address, PPU.BG[3].HOffset, CPU.V_Counter);
	Memory.FillRAM[0x2113] = Byte;
}

static inline void SetPPU_2114 (uint8 Byte)
{
	PPU.BG[3].VOffset = (Byte<<8) | PPU.BGnxOFSbyte;
	PPU.BGnxOFSbyte = Byte;
	//        fprintf(stderr, "%02x to %04x (PPU.BG[3].VOffset = %04x  %d)\n", Byte, Address, PPU.BG[3].VOffset, CPU.V_Counter);
	Memory.FillRAM[0x2114] = Byte;
}

//end Theme Park


static inline void SetPPU_2115 (uint8 Byte)
{
	// VRAM byte/word access flag and increment
	PPU.VMA.High = (Byte & 0x80) == 0 ? FALSE : TRUE;
	switch (Byte & 3)
	{
	  case 0:
		PPU.VMA.Increment = 1;
		break;
	  case 1:
		PPU.VMA.Increment = 32;
		break;
	  case 2:
		PPU.VMA.Increment = 128;
		break;
	  case 3:
		PPU.VMA.Increment = 128;
		break;
	}
#ifdef DEBUGGER
	if ((Byte & 3) != 0)
		missing.vram_inc = Byte & 3;
#endif
	if (Byte & 0x0c)
	{
		static uint16 IncCount [4] = { 0, 32, 64, 128 };
		static uint16 Shift [4] = { 0, 5, 6, 7 };
#ifdef DEBUGGER
		missing.vram_full_graphic_inc = (Byte & 0x0c) >> 2;
#endif
//				PPU.VMA.Increment = 1;
		uint8 i = (Byte & 0x0c) >> 2;
		PPU.VMA.FullGraphicCount = IncCount [i];
		PPU.VMA.Mask1 = IncCount [i] * 8 - 1;
		PPU.VMA.Shift = Shift [i];
	}
	else
		PPU.VMA.FullGraphicCount = 0;

	Memory.FillRAM[0x2115] = Byte;
}

static inline void SetPPU_2116 (uint8 Byte)
{
	// VRAM read/write address (low)
	PPU.VMA.Address &= 0xFF00;
	PPU.VMA.Address |= Byte;
#ifdef CORRECT_VRAM_READS
	if (PPU.VMA.FullGraphicCount)
	{
		uint32 addr = PPU.VMA.Address;
		uint32 rem = addr & PPU.VMA.Mask1;
		uint32 address = (addr & ~PPU.VMA.Mask1) +
			(rem >> PPU.VMA.Shift) +
			((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
		IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM+((address << 1) & 0xFFFF));
	} else
		IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM+((PPU.VMA.Address << 1) & 0xffff));
#else
	IPPU.FirstVRAMRead = TRUE;
#endif

	Memory.FillRAM[0x2116] = Byte;
}

static inline void SetPPU_2117 (uint8 Byte)
{
	// VRAM read/write address (high)
	PPU.VMA.Address &= 0x00FF;
	PPU.VMA.Address |= Byte << 8;
#ifdef CORRECT_VRAM_READS
	if (PPU.VMA.FullGraphicCount)
	{
		uint32 addr = PPU.VMA.Address;
		uint32 rem = addr & PPU.VMA.Mask1;
		uint32 address = (addr & ~PPU.VMA.Mask1) +
			(rem >> PPU.VMA.Shift) +
			((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
		IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM+((address << 1) & 0xFFFF));
	} else
		IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM+((PPU.VMA.Address << 1) & 0xffff));
#else
	IPPU.FirstVRAMRead = TRUE;
#endif
	Memory.FillRAM[0x2117] = Byte;
}

#ifdef USE_REGISTER
#define SetPPU_2118		REGISTER_2118
#define SetPPU_2119		REGISTER_2119
#else
static inline void SetPPU_2118 (uint8 Byte)
{
	// VRAM write data (low)
#ifndef CORRECT_VRAM_READS
	IPPU.FirstVRAMRead = TRUE;
#endif
	REGISTER_2118(Byte);
	Memory.FillRAM[0x2118] = Byte;
}

static inline void SetPPU_2119 (uint8 Byte)
{
	// VRAM write data (high)
#ifndef CORRECT_VRAM_READS
	IPPU.FirstVRAMRead = TRUE;
#endif
	REGISTER_2119(Byte);
	Memory.FillRAM[0x2119] = Byte;
}
#endif // USE_REGISTER

static inline void SetPPU_211A (uint8 Byte)
{
	// Mode 7 outside rotation area display mode and flipping
	if (Byte != Memory.FillRAM [0x211a])
	{
		FLUSH_REDRAW ();
		PPU.Mode7Repeat = Byte >> 6;
		if (PPU.Mode7Repeat == 1)
			PPU.Mode7Repeat = 0;
		PPU.Mode7VFlip = (Byte & 2) >> 1;
		PPU.Mode7HFlip = Byte & 1;
	}
	Memory.FillRAM[0x211A] = Byte;
}

static inline void SetPPU_211B (uint8 Byte)
{
	// Mode 7 matrix A (low & high)
	PPU.MatrixA = ((PPU.MatrixA >> 8) & 0xff) | (Byte << 8);
	PPU.Need16x8Mulitply = TRUE;
	Memory.FillRAM[0x211B] = Byte;
}

static inline void SetPPU_211C (uint8 Byte)
{
	// Mode 7 matrix B (low & high)
	PPU.MatrixB = ((PPU.MatrixB >> 8) & 0xff) | (Byte << 8);
	PPU.Need16x8Mulitply = TRUE;
	Memory.FillRAM[0x211C] = Byte;
}

static inline void SetPPU_211D (uint8 Byte)
{
	// Mode 7 matrix C (low & high)
	PPU.MatrixC = ((PPU.MatrixC >> 8) & 0xff) | (Byte << 8);
	Memory.FillRAM[0x211D] = Byte;
}

static inline void SetPPU_211E (uint8 Byte)
{
	// Mode 7 matrix D (low & high)
	PPU.MatrixD = ((PPU.MatrixD >> 8) & 0xff) | (Byte << 8);
	Memory.FillRAM[0x211E] = Byte;
}

static inline void SetPPU_211F (uint8 Byte)
{
	// Mode 7 centre of rotation X (low & high)
	PPU.CentreX = ((PPU.CentreX >> 8) & 0xff) | (Byte << 8);
	Memory.FillRAM[0x211F] = Byte;
}

static inline void SetPPU_2120 (uint8 Byte)
{
	// Mode 7 centre of rotation Y (low & high)
	PPU.CentreY = ((PPU.CentreY >> 8) & 0xff) | (Byte << 8);
	Memory.FillRAM[0x2120] = Byte;
}

static inline void SetPPU_2121 (uint8 Byte)
{
	// CG-RAM address
	PPU.CGFLIP = 0;
	PPU.CGFLIPRead = 0;
	PPU.CGADD = Byte;
	Memory.FillRAM[0x2121] = Byte;
}

#ifdef USE_REGISTER
#define SetPPU_2122		REGISTER_2122
#else
static inline void SetPPU_2122 (uint8 Byte)
{
	REGISTER_2122(Byte);
	Memory.FillRAM[0x2122] = Byte;
}
#endif // USE_REGISTER

static inline void SetPPU_2123 (uint8 Byte)
{
	// Window 1 and 2 enable for backgrounds 1 and 2
	if (Byte != Memory.FillRAM [0x2123])
	{
		FLUSH_REDRAW ();
		PPU.ClipWindow1Enable [0] = !!(Byte & 0x02);
		PPU.ClipWindow1Enable [1] = !!(Byte & 0x20);
		PPU.ClipWindow2Enable [0] = !!(Byte & 0x08);
		PPU.ClipWindow2Enable [1] = !!(Byte & 0x80);
		PPU.ClipWindow1Inside [0] = !(Byte & 0x01);
		PPU.ClipWindow1Inside [1] = !(Byte & 0x10);
		PPU.ClipWindow2Inside [0] = !(Byte & 0x04);
		PPU.ClipWindow2Inside [1] = !(Byte & 0x40);
		PPU.RecomputeClipWindows = TRUE;
#ifdef DEBUGGER
		if (Byte & 0x80)
			missing.window2[1] = 1;
		if (Byte & 0x20)
			missing.window1[1] = 1;
		if (Byte & 0x08)
			missing.window2[0] = 1;
		if (Byte & 0x02)
			missing.window1[0] = 1;
#endif
		Memory.FillRAM[0x2123] = Byte;
	}
}

static inline void SetPPU_2124 (uint8 Byte)
{
	// Window 1 and 2 enable for backgrounds 3 and 4
	if (Byte != Memory.FillRAM [0x2124])
	{
		FLUSH_REDRAW ();
		PPU.ClipWindow1Enable [2] = !!(Byte & 0x02);
		PPU.ClipWindow1Enable [3] = !!(Byte & 0x20);
		PPU.ClipWindow2Enable [2] = !!(Byte & 0x08);
		PPU.ClipWindow2Enable [3] = !!(Byte & 0x80);
		PPU.ClipWindow1Inside [2] = !(Byte & 0x01);
		PPU.ClipWindow1Inside [3] = !(Byte & 0x10);
		PPU.ClipWindow2Inside [2] = !(Byte & 0x04);
		PPU.ClipWindow2Inside [3] = !(Byte & 0x40);
		PPU.RecomputeClipWindows = TRUE;
#ifdef DEBUGGER
		if (Byte & 0x80)
			missing.window2[3] = 1;
		if (Byte & 0x20)
			missing.window1[3] = 1;
		if (Byte & 0x08)
			missing.window2[2] = 1;
		if (Byte & 0x02)
			missing.window1[2] = 1;
#endif
		Memory.FillRAM[0x2124] = Byte;
	}
}

static inline void SetPPU_2125 (uint8 Byte)
{
	// Window 1 and 2 enable for objects and colour window
	if (Byte != Memory.FillRAM [0x2125])
	{
		FLUSH_REDRAW ();
		PPU.ClipWindow1Enable [4] = !!(Byte & 0x02);
		PPU.ClipWindow1Enable [5] = !!(Byte & 0x20);
		PPU.ClipWindow2Enable [4] = !!(Byte & 0x08);
		PPU.ClipWindow2Enable [5] = !!(Byte & 0x80);
		PPU.ClipWindow1Inside [4] = !(Byte & 0x01);
		PPU.ClipWindow1Inside [5] = !(Byte & 0x10);
		PPU.ClipWindow2Inside [4] = !(Byte & 0x04);
		PPU.ClipWindow2Inside [5] = !(Byte & 0x40);
		PPU.RecomputeClipWindows = TRUE;
#ifdef DEBUGGER
		if (Byte & 0x80)
			missing.window2[5] = 1;
		if (Byte & 0x20)
			missing.window1[5] = 1;
		if (Byte & 0x08)
			missing.window2[4] = 1;
		if (Byte & 0x02)
			missing.window1[4] = 1;
#endif
		Memory.FillRAM[0x2125] = Byte;
	}
}

static inline void SetPPU_2126 (uint8 Byte)
{
	// Window 1 left position
	if (Byte != Memory.FillRAM [0x2126])
	{
		FLUSH_REDRAW ();
		PPU.Window1Left = Byte;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x2126] = Byte;
	}
}

static inline void SetPPU_2127 (uint8 Byte)
{
	// Window 1 right position
	if (Byte != Memory.FillRAM [0x2127])
	{
		FLUSH_REDRAW ();
		PPU.Window1Right = Byte;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x2127] = Byte;
	}
}

static inline void SetPPU_2128 (uint8 Byte)
{
	// Window 2 left position
	if (Byte != Memory.FillRAM [0x2128])
	{
		FLUSH_REDRAW ();
		PPU.Window2Left = Byte;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x2128] = Byte;
	}
}

static inline void SetPPU_2129 (uint8 Byte)
{
	// Window 2 right position
	if (Byte != Memory.FillRAM [0x2129])
	{
		FLUSH_REDRAW ();
		PPU.Window2Right = Byte;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x2129] = Byte;
	}
}

static inline void SetPPU_212A (uint8 Byte)
{
	// Windows 1 & 2 overlap logic for backgrounds 1 - 4
	if (Byte != Memory.FillRAM [0x212a])
	{
		FLUSH_REDRAW ();
		PPU.ClipWindowOverlapLogic [0] = (Byte & 0x03);
		PPU.ClipWindowOverlapLogic [1] = (Byte & 0x0c) >> 2;
		PPU.ClipWindowOverlapLogic [2] = (Byte & 0x30) >> 4;
		PPU.ClipWindowOverlapLogic [3] = (Byte & 0xc0) >> 6;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x212A] = Byte;
	}
}

static inline void SetPPU_212B (uint8 Byte)
{
	// Windows 1 & 2 overlap logic for objects and colour window
	if (Byte != Memory.FillRAM [0x212b])
	{
		FLUSH_REDRAW ();
		PPU.ClipWindowOverlapLogic [4] = Byte & 0x03;
		PPU.ClipWindowOverlapLogic [5] = (Byte & 0x0c) >> 2;
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x212B] = Byte;
	}
}

static inline void SetPPU_212C (uint8 Byte)
{
	// Main screen designation (backgrounds 1 - 4 and objects)
	if (Byte != Memory.FillRAM [0x212c])
	{
		FLUSH_REDRAW ();
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM [0x212C] = Byte;
	}
}

static inline void SetPPU_212D (uint8 Byte)
{
	// Sub-screen designation (backgrounds 1 - 4 and objects)
	if (Byte != Memory.FillRAM [0x212d])
	{
		FLUSH_REDRAW ();
#ifdef DEBUGGER
		if (Byte & 0x1f)
			missing.subscreen = 1;
#endif
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x212D] = Byte;
	}
}

static inline void SetPPU_212E (uint8 Byte)
{
	// Window mask designation for main screen ?
	if (Byte != Memory.FillRAM [0x212e])
	{
		FLUSH_REDRAW ();
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x212E] = Byte;
	}
}

static inline void SetPPU_212F (uint8 Byte)
{
	// Window mask designation for sub-screen ?
	if (Byte != Memory.FillRAM [0x212f])
	{
		FLUSH_REDRAW ();
		PPU.RecomputeClipWindows = TRUE;
		Memory.FillRAM[0x212F] = Byte;
	}
}

static inline void SetPPU_2130 (uint8 Byte)
{
	// Fixed colour addition or screen addition
	if (Byte != Memory.FillRAM [0x2130])
	{
		FLUSH_REDRAW ();
		PPU.RecomputeClipWindows = TRUE;
#ifdef DEBUGGER
		if ((Byte & 1) && (PPU.BGMode == 3 || PPU.BGMode == 4 || PPU.BGMode == 7))
			missing.direct = 1;
#endif
		Memory.FillRAM[0x2130] = Byte;
	}
}

static inline void SetPPU_2131 (uint8 Byte)
{
	// Colour addition or subtraction select
	if (Byte != Memory.FillRAM[0x2131])
	{
		FLUSH_REDRAW ();
		// Backgrounds 1 - 4, objects and backdrop colour add/sub enable
#ifdef DEBUGGER
		if (Byte & 0x80)
		{
			// Subtract
			if (Memory.FillRAM[0x2130] & 0x02)
				missing.subscreen_sub = 1;
			else
				missing.fixed_colour_sub = 1;
		}
		else
		{
			// Addition
			if (Memory.FillRAM[0x2130] & 0x02)
				missing.subscreen_add = 1;
			else
				missing.fixed_colour_add = 1;
		}
#endif
		Memory.FillRAM[0x2131] = Byte;
	}
}

static inline void SetPPU_2132 (uint8 Byte)
{
	if (Byte != Memory.FillRAM [0x2132])
	{
		FLUSH_REDRAW ();
		// Colour data for fixed colour addition/subtraction
		if (Byte & 0x80)
			PPU.FixedColourBlue = Byte & 0x1f;
		if (Byte & 0x40)
			PPU.FixedColourGreen = Byte & 0x1f;
		if (Byte & 0x20)
			PPU.FixedColourRed = Byte & 0x1f;
		Memory.FillRAM[0x2132] = Byte;
	}
}

static inline void SetPPU_2133 (uint8 Byte)
{
	// Screen settings
	if (Byte != Memory.FillRAM [0x2133])
	{
#ifdef DEBUGGER
		if (Byte & 0x40)
			missing.mode7_bgmode = 1;
		if (Byte & 0x08)
			missing.pseudo_512 = 1;
#endif
		if (Byte & 0x04)
		{
			PPU.ScreenHeight = SNES_HEIGHT_EXTENDED;
			if(IPPU.DoubleHeightPixels)
				IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
			else
				IPPU.RenderedScreenHeight = PPU.ScreenHeight;
#ifdef DEBUGGER
			missing.lines_239 = 1;
#endif
		}
		else PPU.ScreenHeight = SNES_HEIGHT;

#ifdef DEBUGGER
		if (Byte & 0x02)
			missing.sprite_double_height = 1;

		if (Byte & 1)
			missing.interlace = 1;
#endif
		//if((Byte & 1)&&(PPU.BGMode==5||PPU.BGMode==6))
		//IPPU.Interlace=1;
		if((Memory.FillRAM [0x2133] ^ Byte)&3)
		{
			FLUSH_REDRAW ();
			if((Memory.FillRAM [0x2133] ^ Byte)&2)
				IPPU.OBJChanged = TRUE;
                                if(PPU.BGMode==5||PPU.BGMode==6)
                                    IPPU.Interlace = Byte&1;
			IPPU.InterlaceSprites=0;
			//   IPPU.InterlaceSprites = (Byte&2)>>1;
		}
		Memory.FillRAM[0x2133] = Byte;
	}
}

static inline void SetPPU_NOP (uint8 Byte)
{
/*
  case 0x2134:
  case 0x2135:
  case 0x2136:
	// Matrix 16bit x 8bit multiply result (read-only)
	return;

  case 0x2137:
	// Software latch for horizontal and vertical timers (read-only)
	return;
  case 0x2138:
	// OAM read data (read-only)
	return;
  case 0x2139:
  case 0x213a:
	// VRAM read data (read-only)
	return;
  case 0x213b:
	// CG-RAM read data (read-only)
	return;
  case 0x213c:
  case 0x213d:
	// Horizontal and vertical (low/high) read counter (read-only)
	return;
  case 0x213e:
	// PPU status (time over and range over)
	return;
  case 0x213f:
	// NTSC/PAL select and field (read-only)
	return;
*/
}

static inline void SetPPU_2140to217F(uint16 Address, uint8 Byte)
{
#ifdef SPCTOOL
	_SPCInPB (Address & 3, Byte);
#else
	//	CPU.Flags |= DEBUG_MODE_FLAG;
	Memory.FillRAM [Address] = Byte;
	IAPU.RAM [(Address & 3) + 0xf4] = Byte;
#ifdef SPC700_SHUTDOWN
	IAPU.APUExecuting = Settings.APUEnabled;
	IAPU.WaitCounter++;
#endif
#endif // SPCTOOL
}


#ifdef USE_REGISTER
#define SetPPU_2180		REGISTER_2180
#else
static inline void SetPPU_2180 (uint8 Byte)
{
	REGISTER_2180(Byte);
	Memory.FillRAM[0x2180] = Byte;
}
#endif // USE_REGISTER

static inline void SetPPU_2181 (uint8 Byte)
{
	PPU.WRAM &= 0x1FF00;
	PPU.WRAM |= Byte;
	Memory.FillRAM[0x2181] = Byte;
}

static inline void SetPPU_2182 (uint8 Byte)
{
	PPU.WRAM &= 0x100FF;
	PPU.WRAM |= Byte << 8;
	Memory.FillRAM[0x2182] = Byte;
}

static inline void SetPPU_2183 (uint8 Byte)
{
	PPU.WRAM &= 0x0FFFF;
	PPU.WRAM |= Byte << 16;
	PPU.WRAM &= 0x1FFFF;
	Memory.FillRAM[0x2183] = Byte;
}

void S9xSetPPU (uint8 Byte, uint16 Address)
{
//    fprintf(stderr, "%03d: %02x to %04x\n", CPU.V_Counter, Byte, Address);
	if (Address < 0x2100){
		Memory.FillRAM[Address] = Byte;
		return;
	} else if (Address <= 0x2183){
#define SETPPU_BEGIN(addr) adr##addr:
#define SETPPU_END return;

		static void *SetPPU_AddrTbl[] = {
			      &&adr2100,       &&adr2101,       &&adr2102,       &&adr2103,       &&adr2104,       &&adr2105,       &&adr2106,       &&adr2107,
			      &&adr2108,       &&adr2109,       &&adr210A,       &&adr210B,       &&adr210C,       &&adr210D,       &&adr210E,       &&adr210F,
			      &&adr2110,       &&adr2111,       &&adr2112,       &&adr2113,       &&adr2114,       &&adr2115,       &&adr2116,       &&adr2117,
			      &&adr2118,       &&adr2119,       &&adr211A,       &&adr211B,       &&adr211C,       &&adr211D,       &&adr211E,       &&adr211F,
			      &&adr2120,       &&adr2121,       &&adr2122,       &&adr2123,       &&adr2124,       &&adr2125,       &&adr2126,       &&adr2127,
			      &&adr2128,       &&adr2129,       &&adr212A,       &&adr212B,       &&adr212C,       &&adr212D,       &&adr212E,       &&adr212F,
			      &&adr2130,       &&adr2131,       &&adr2132,       &&adr2133,        &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,
			       &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,        &&adrNOP,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2180, &&adr2181, &&adr2182, &&adr2183
		};

		goto *SetPPU_AddrTbl[Address - 0x2100];

		SETPPU_BEGIN(2100)
			SetPPU_2100(Byte);
			SETPPU_END
		SETPPU_BEGIN(2101)
			SetPPU_2101(Byte);
			SETPPU_END
		SETPPU_BEGIN(2102)
			SetPPU_2102(Byte);
			SETPPU_END
		SETPPU_BEGIN(2103)
			SetPPU_2103(Byte);
			SETPPU_END
		SETPPU_BEGIN(2104)
			SetPPU_2104(Byte);
			SETPPU_END
		SETPPU_BEGIN(2105)
			SetPPU_2105(Byte);
			SETPPU_END
		SETPPU_BEGIN(2106)
			SetPPU_2106(Byte);
			SETPPU_END
		SETPPU_BEGIN(2107)
			SetPPU_2107(Byte);
			SETPPU_END
		SETPPU_BEGIN(2108)
			SetPPU_2108(Byte);
			SETPPU_END
		SETPPU_BEGIN(2109)
			SetPPU_2109(Byte);
			SETPPU_END
		SETPPU_BEGIN(210A)
			SetPPU_210A(Byte);
			SETPPU_END
		SETPPU_BEGIN(210B)
			SetPPU_210B(Byte);
			SETPPU_END
		SETPPU_BEGIN(210C)
			SetPPU_210C(Byte);
			SETPPU_END
		SETPPU_BEGIN(210D)
			SetPPU_210D(Byte);
			SETPPU_END
		SETPPU_BEGIN(210E)
			SetPPU_210E(Byte);
			SETPPU_END
		SETPPU_BEGIN(210F)
			SetPPU_210F(Byte);
			SETPPU_END

		SETPPU_BEGIN(2110)
			SetPPU_2110(Byte);
			SETPPU_END
		SETPPU_BEGIN(2111)
			SetPPU_2111(Byte);
			SETPPU_END
		SETPPU_BEGIN(2112)
			SetPPU_2112(Byte);
			SETPPU_END
		SETPPU_BEGIN(2113)
			SetPPU_2113(Byte);
			SETPPU_END
		SETPPU_BEGIN(2114)
			SetPPU_2114(Byte);
			SETPPU_END
		SETPPU_BEGIN(2115)
			SetPPU_2115(Byte);
			SETPPU_END
		SETPPU_BEGIN(2116)
			SetPPU_2116(Byte);
			SETPPU_END
		SETPPU_BEGIN(2117)
			SetPPU_2117(Byte);
			SETPPU_END
		SETPPU_BEGIN(2118)
			SetPPU_2118(Byte);
			SETPPU_END
		SETPPU_BEGIN(2119)
			SetPPU_2119(Byte);
			SETPPU_END
		SETPPU_BEGIN(211A)
			SetPPU_211A(Byte);
			SETPPU_END
		SETPPU_BEGIN(211B)
			SetPPU_211B(Byte);
			SETPPU_END
		SETPPU_BEGIN(211C)
			SetPPU_211C(Byte);
			SETPPU_END
		SETPPU_BEGIN(211D)
			SetPPU_211D(Byte);
			SETPPU_END
		SETPPU_BEGIN(211E)
			SetPPU_211E(Byte);
			SETPPU_END
		SETPPU_BEGIN(211F)
			SetPPU_211F(Byte);
			SETPPU_END

		SETPPU_BEGIN(2120)
			SetPPU_2120(Byte);
			SETPPU_END
		SETPPU_BEGIN(2121)
			SetPPU_2121(Byte);
			SETPPU_END
		SETPPU_BEGIN(2122)
			SetPPU_2122(Byte);
			SETPPU_END
		SETPPU_BEGIN(2123)
			SetPPU_2123(Byte);
			SETPPU_END
		SETPPU_BEGIN(2124)
			SetPPU_2124(Byte);
			SETPPU_END
		SETPPU_BEGIN(2125)
			SetPPU_2125(Byte);
			SETPPU_END
		SETPPU_BEGIN(2126)
			SetPPU_2126(Byte);
			SETPPU_END
		SETPPU_BEGIN(2127)
			SetPPU_2127(Byte);
			SETPPU_END
		SETPPU_BEGIN(2128)
			SetPPU_2128(Byte);
			SETPPU_END
		SETPPU_BEGIN(2129)
			SetPPU_2129(Byte);
			SETPPU_END
		SETPPU_BEGIN(212A)
			SetPPU_212A(Byte);
			SETPPU_END
		SETPPU_BEGIN(212B)
			SetPPU_212B(Byte);
			SETPPU_END
		SETPPU_BEGIN(212C)
			SetPPU_212C(Byte);
			SETPPU_END
		SETPPU_BEGIN(212D)
			SetPPU_212D(Byte);
			SETPPU_END
		SETPPU_BEGIN(212E)
			SetPPU_212E(Byte);
			SETPPU_END
		SETPPU_BEGIN(212F)
			SetPPU_212F(Byte);
			SETPPU_END

		SETPPU_BEGIN(2130)
			SetPPU_2130(Byte);
			SETPPU_END
		SETPPU_BEGIN(2131)
			SetPPU_2131(Byte);
			SETPPU_END
		SETPPU_BEGIN(2132)
			SetPPU_2132(Byte);
			SETPPU_END
		SETPPU_BEGIN(2133)
			SetPPU_2133(Byte);
			SETPPU_END
		SETPPU_BEGIN(NOP)
			SetPPU_NOP(Byte);
			SETPPU_END

		SETPPU_BEGIN(2140to217F)
			SetPPU_2140to217F(Address, Byte);
			SETPPU_END

		SETPPU_BEGIN(2180)
			SetPPU_2180(Byte);
			SETPPU_END
		SETPPU_BEGIN(2181)
			SetPPU_2181(Byte);
			SETPPU_END
		SETPPU_BEGIN(2182)
			SetPPU_2182(Byte);
			SETPPU_END
		SETPPU_BEGIN(2183)
			SetPPU_2183(Byte);
			SETPPU_END

		return;
	}
	else
	{
		if (Settings.SA1)
		{
			if (Address >= 0x2200 && Address <0x23ff)
				S9xSetSA1 (Byte, Address);
			else
				Memory.FillRAM [Address] = Byte;

			return;
		}
		else
			// Dai Kaijyu Monogatari II
			if (Address == 0x2801 && Settings.SRTC)
				S9xSetSRTC (Byte, Address);
			else
				if (Address < 0x3000 || Address >= 0x3000 + 768)
				{
#ifdef DEBUGGER
					missing.unknownppu_write = Address;
					if (Settings.TraceUnknownRegisters)
					{
						sprintf (String, "Unknown register write: $%02X->$%04X\n",
								 Byte, Address);
						S9xMessage (S9X_TRACE, S9X_PPU_TRACE, String);
					}
#endif
				}
				else
				{
					if (!Settings.SuperFX)
					{
						return;
					}

#ifdef ZSNES_FX
					Memory.FillRAM [Address] = Byte;
					if (Address < 0x3040)
						S9xSuperFXWriteReg (Byte, Address);
#else
					switch (Address)
					{
					  case 0x3030:
						if ((Memory.FillRAM [0x3030] ^ Byte) & FLG_G)
						{
							Memory.FillRAM [Address] = Byte;
							// Go flag has been changed
							if (Byte & FLG_G)
								S9xSuperFXExec ();
							else
								FxFlushCache ();
						}
						else
							Memory.FillRAM [Address] = Byte;
						break;

					  case 0x3031:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x3033:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x3034:
						Memory.FillRAM [Address] = Byte & 0x7f;
						break;
					  case 0x3036:
						Memory.FillRAM [Address] = Byte & 0x7f;
						break;
					  case 0x3037:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x3038:
						Memory.FillRAM [Address] = Byte;
						fx_dirtySCBR();
						break;
					  case 0x3039:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x303a:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x303b:
						break;
					  case 0x303c:
						Memory.FillRAM [Address] = Byte;
						fx_updateRamBank(Byte);
						break;
					  case 0x303f:
						Memory.FillRAM [Address] = Byte;
						break;
					  case 0x301f:
						Memory.FillRAM [Address] = Byte;
						Memory.FillRAM [0x3000 + GSU_SFR] |= FLG_G;
						S9xSuperFXExec ();
						return;

					  default:
						Memory.FillRAM[Address] = Byte;
						if (Address >= 0x3100)
						{
							FxCacheWriteAccess (Address);
						}
						break;
					}
#endif
					return;
				}
	}
	Memory.FillRAM[Address] = Byte;

}
