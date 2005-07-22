
static inline uint8 GetPPU_BUS ()
{
	return OpenBus;
}

static inline uint8 GetPPU_BUS1 ()
{
	return PPU.OpenBus1;
}


static inline uint8 GetPPU_2134 ()
{
    // 16bit x 8bit multiply read result.
    if (PPU.Need16x8Mulitply)
    {
	int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);

	Memory.FillRAM[0x2134] = (uint8) r;
	Memory.FillRAM[0x2135] = (uint8)(r >> 8);
	Memory.FillRAM[0x2136] = (uint8)(r >> 16);
	PPU.Need16x8Mulitply = FALSE;
    }
#ifdef DEBUGGER
    missing.matrix_multiply = 1;
#endif
	return (PPU.OpenBus1 = Memory.FillRAM[0x2134]);
}

static inline uint8 GetPPU_2135 ()
{
    // 16bit x 8bit multiply read result.
    if (PPU.Need16x8Mulitply)
    {
	int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);

	Memory.FillRAM[0x2134] = (uint8) r;
	Memory.FillRAM[0x2135] = (uint8)(r >> 8);
	Memory.FillRAM[0x2136] = (uint8)(r >> 16);
	PPU.Need16x8Mulitply = FALSE;
    }
#ifdef DEBUGGER
    missing.matrix_multiply = 1;
#endif
	return (PPU.OpenBus1 = Memory.FillRAM[0x2135]);
}

static inline uint8 GetPPU_2136 ()
{
    // 16bit x 8bit multiply read result.
    if (PPU.Need16x8Mulitply)
    {
	int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);

	Memory.FillRAM[0x2134] = (uint8) r;
	Memory.FillRAM[0x2135] = (uint8)(r >> 8);
	Memory.FillRAM[0x2136] = (uint8)(r >> 16);
	PPU.Need16x8Mulitply = FALSE;
    }
#ifdef DEBUGGER
    missing.matrix_multiply = 1;
#endif
	return (PPU.OpenBus1 = Memory.FillRAM[0x2136]);
}

static inline uint8 GetPPU_2137 ()
{
	S9xLatchCounters(0);
	return OpenBus;
}

static inline uint8 GetPPU_2138 ()
{
 	uint8 byte;

	// Read OAM (sprite) control data
	if(PPU.OAMAddr&0x100){
		if (!(PPU.OAMFlip&1))
		{
			byte = PPU.OAMData [(PPU.OAMAddr&0x10f) << 1];
		}
		else
		{
			byte = PPU.OAMData [((PPU.OAMAddr&0x10f) << 1) + 1];
			PPU.OAMAddr=(PPU.OAMAddr+1)&0x1ff;
			if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
			{
				PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
				IPPU.OBJChanged = TRUE;
#ifdef DEBUGGER
				missing.sprite_priority_rotation = 1;
#endif
			}
		}
	} else {
		if (!(PPU.OAMFlip&1))
		{
			byte = PPU.OAMData [PPU.OAMAddr << 1];
		}
		else
		{
			byte = PPU.OAMData [(PPU.OAMAddr << 1) + 1];
			++PPU.OAMAddr;
			if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
			{
				PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
				IPPU.OBJChanged = TRUE;
#ifdef DEBUGGER
				missing.sprite_priority_rotation = 1;
#endif
			}
		}
	}
	PPU.OAMFlip ^= 1;
#ifdef DEBUGGER
    missing.oam_read = 1;
#endif
	return (PPU.OpenBus1 = byte);
}

static inline uint8 GetPPU_2139 ()
{
 	uint8 byte;

	// Read vram low byte
#ifdef DEBUGGER
	missing.vram_read = 1;
#endif
#ifdef CORRECT_VRAM_READS
	byte = IPPU.VRAMReadBuffer & 0xff;
	if (!PPU.VMA.High)
	{
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
		PPU.VMA.Address += PPU.VMA.Increment;
	}
#else
	if (IPPU.FirstVRAMRead)
		byte = Memory.VRAM[(PPU.VMA.Address << 1)&0xFFFF];
	else
		if (PPU.VMA.FullGraphicCount)
		{
			uint32 addr = PPU.VMA.Address - 1;
			uint32 rem = addr & PPU.VMA.Mask1;
			uint32 address = (addr & ~PPU.VMA.Mask1) +
				(rem >> PPU.VMA.Shift) +
				((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
			byte = Memory.VRAM [((address << 1) - 2) & 0xFFFF];
		}
		else
			byte = Memory.VRAM[((PPU.VMA.Address << 1) - 2) & 0xffff];

	if (!PPU.VMA.High)
	{
		PPU.VMA.Address += PPU.VMA.Increment;
		IPPU.FirstVRAMRead = FALSE;
	}
#endif
	PPU.OpenBus1 = byte;
    return (byte);
}

static inline uint8 GetPPU_213A ()
{
 	uint8 byte;

	// Read vram high byte
#ifdef DEBUGGER
	missing.vram_read = 1;
#endif
#ifdef CORRECT_VRAM_READS
	byte = (IPPU.VRAMReadBuffer>>8) & 0xff;
	if (PPU.VMA.High)
	{
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
		PPU.VMA.Address += PPU.VMA.Increment;
	}
#else
	if (IPPU.FirstVRAMRead)
		byte = Memory.VRAM[((PPU.VMA.Address << 1) + 1) & 0xffff];
	else
		if (PPU.VMA.FullGraphicCount)
		{
			uint32 addr = PPU.VMA.Address - 1;
			uint32 rem = addr & PPU.VMA.Mask1;
			uint32 address = (addr & ~PPU.VMA.Mask1) +
				(rem >> PPU.VMA.Shift) +
				((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
			byte = Memory.VRAM [((address << 1) - 1) & 0xFFFF];
		}
		else
			byte = Memory.VRAM[((PPU.VMA.Address << 1) - 1) & 0xFFFF];
	if (PPU.VMA.High)
	{
		PPU.VMA.Address += PPU.VMA.Increment;
		IPPU.FirstVRAMRead = FALSE;
	}
#endif
	PPU.OpenBus1 = byte;
    return (byte);
}

static inline uint8 GetPPU_213B ()
{
 	uint8 byte;

    // Read palette data
#ifdef DEBUGGER
    missing.cgram_read = 1;
#endif
    if (PPU.CGFLIPRead)
	byte = PPU.CGDATA [PPU.CGADD++] >> 8;
    else
	byte = PPU.CGDATA [PPU.CGADD] & 0xff;

    PPU.CGFLIPRead ^= 1;
    return (PPU.OpenBus2 = byte);
}

static inline uint8 GetPPU_213C ()
{
 	uint8 byte;

    // Horizontal counter value 0-339
#ifdef DEBUGGER
    missing.h_counter_read = 1;
#endif
    if (PPU.HBeamFlip)
	byte = (PPU.OpenBus2 & 0xfe) | ((PPU.HBeamPosLatched >> 8) & 0x01);
    else
	byte = (uint8)PPU.HBeamPosLatched;
        PPU.OpenBus2 = byte;
    PPU.HBeamFlip ^= 1;
    return (byte);
}

static inline uint8 GetPPU_213D ()
{
 	uint8 byte;

    // Vertical counter value 0-262
#ifdef DEBUGGER
    missing.v_counter_read = 1;
#endif
    if (PPU.VBeamFlip)
            byte = (PPU.OpenBus2 & 0xfe) | ((PPU.VBeamPosLatched >> 8) & 0x01);
    else
            byte = (uint8)PPU.VBeamPosLatched;
        PPU.OpenBus2 = byte;
    PPU.VBeamFlip ^= 1;
    return (byte);
}

static inline uint8 GetPPU_213E ()
{
    // PPU time and range over flags
    FLUSH_REDRAW ();

    //so far, 5c77 version is always 1.
	return (PPU.OpenBus1 = (Model->_5C77 | PPU.RangeTimeOver));
}

static inline uint8 GetPPU_213F ()
{
    // NTSC/PAL and which field flags
    PPU.VBeamFlip = PPU.HBeamFlip = 0;
        //neviksti found a 2 and a 3 here. SNEeSe uses a 3.
        //XXX: field flags not emulated
    return ((Settings.PAL ? 0x10 : 0) | (Memory.FillRAM[0x213f] & 0xc0)| Model->_5C78) | (~PPU.OpenBus2 & 0x20);
}

static inline uint8 GetPPU_2180 ()
{
 	uint8 byte;

    // Read WRAM
#ifdef DEBUGGER
    missing.wram_read = 1;
#endif
    byte = Memory.RAM [PPU.WRAM++];
    PPU.WRAM &= 0x1FFFF;
    return (byte);
}

static inline uint8 GetPPU_2140to217F (uint16 Address)
{
#ifdef SPCTOOL
    return ((uint8) _SPCOutP [Address & 3]);
#else
	//	CPU.Flags |= DEBUG_MODE_FLAG;
#ifdef SPC700_SHUTDOWN
    IAPU.APUExecuting = TRUE;
    IAPU.WaitCounter++;
#endif
#ifdef CPU_SHUTDOWN
	//		CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif
	if (SNESGameFixes.APU_OutPorts_ReturnValueFix &&
	    Address >= 0x2140 && Address <= 0x2143 && !CPU.V_Counter)
	{
		return (uint8)((Address & 1) ? ((rand() & 0xff00) >> 8) : (rand() & 0xff));
	}
	return (APU.OutPorts [Address & 3]);
#endif // SPCTOOL
}

uint8 S9xGetPPU (uint16 Address)
{
 	uint8 byte = OpenBus;

	if(Address<0x2100)//not a real PPU reg
		return OpenBus; //treat as unmapped memory returning last byte on the bus
    if (Address <= 0x2190){
#define GETPPU_BEGIN(addr) adr##addr:
#define GETPPU_END(func) return (func);

		static void *GetPPU_AddrTbl[] = {
			       &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,       &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,
			      &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,
			       &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,       &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,
			      &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,
			       &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,       &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,
			      &&adrBUS1,       &&adrBUS1,       &&adrBUS1,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,
			       &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,       &&adr2134,       &&adr2135,       &&adr2136,       &&adr2137,
			      &&adr2138,       &&adr2139,       &&adr213A,       &&adr213B,       &&adr213C,       &&adr213D,       &&adr213E,       &&adr213F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			&&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F, &&adr2140to217F,
			      &&adr2180,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,
			       &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,        &&adrBUS,
			       &&adrBUS
		};

		goto *GetPPU_AddrTbl[Address - 0x2100];

		GETPPU_BEGIN(BUS)
			GETPPU_END(GetPPU_BUS());
		GETPPU_BEGIN(BUS1)
			GETPPU_END(GetPPU_BUS1());

		GETPPU_BEGIN(2134)
			GETPPU_END(GetPPU_2134());
		GETPPU_BEGIN(2135)
			GETPPU_END(GetPPU_2135());
		GETPPU_BEGIN(2136)
			GETPPU_END(GetPPU_2136());
		GETPPU_BEGIN(2137)
			GETPPU_END(GetPPU_2137());
		GETPPU_BEGIN(2138)
			GETPPU_END(GetPPU_2138());
		GETPPU_BEGIN(2139)
			GETPPU_END(GetPPU_2139());
		GETPPU_BEGIN(213A)
			GETPPU_END(GetPPU_213A());
		GETPPU_BEGIN(213B)
			GETPPU_END(GetPPU_213B());
		GETPPU_BEGIN(213C)
			GETPPU_END(GetPPU_213C());
		GETPPU_BEGIN(213D)
			GETPPU_END(GetPPU_213D());
		GETPPU_BEGIN(213E)
			GETPPU_END(GetPPU_213E());
		GETPPU_BEGIN(213F)
			GETPPU_END(GetPPU_213F());

		GETPPU_BEGIN(2140to217F)
			GETPPU_END(GetPPU_2140to217F(Address));

		GETPPU_BEGIN(2180)
			GETPPU_END(GetPPU_2180());
    } else {
		if (Settings.SA1)
		    return (S9xGetSA1 (Address));

		if (Address <= 0x2fff || Address >= 0x3000 + 768)
		{
		    switch (Address){
		    case 0x21c2:
				if(Model->_5C77 ==2)
		        return (0x20);

				//	fprintf(stderr, "Read from $21c2!\n");
				return OpenBus;
		    case 0x21c3:
				if(Model->_5C77 ==2)
			        return (0);
				//	fprintf(stderr, "Read from $21c3!\n");
				return OpenBus;
		    case 0x2800:
				// For Dai Kaijyu Monogatari II
				if (Settings.SRTC)
				    return (S9xGetSRTC (Address));
				/*FALL*/

		    default:
#ifdef DEBUGGER
		        missing.unknownppu_read = Address;
		        if (Settings.TraceUnknownRegisters)
				{
				    sprintf (String, "Unknown register read: $%04X\n", Address);
				    S9xMessage (S9X_TRACE, S9X_PPU_TRACE, String);
				}
#endif
				return OpenBus;
		    }
		}

		if (!Settings.SuperFX)
				return OpenBus;
#ifdef ZSNES_FX
		if (Address < 0x3040)
		    byte = S9xSuperFXReadReg (Address);
		else
		    byte = Memory.FillRAM [Address];

#ifdef CPU_SHUTDOWN
		if (Address == 0x3030)
		    CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif
		if (Address == 0x3031)
		    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE);
#else
		byte = Memory.FillRAM [Address];

//if (Address != 0x3030 && Address != 0x3031)
//printf ("%04x\n", Address);
#ifdef CPU_SHUTDOWN
		if (Address == 0x3030)
		{
		    CPU.WaitAddress = CPU.PCAtOpcodeStart;
		}
		else
#endif
		if (Address == 0x3031)
		{
		    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE);
		    Memory.FillRAM [0x3031] = byte & 0x7f;
		}
		return (byte);
#endif
    }
//    fprintf(stderr, "%03d: %02x from %04x\n", CPU.V_Counter, byte, Address);
    return (byte);
}
