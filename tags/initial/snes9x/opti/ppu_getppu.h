
static uint8 GetPPU_BUS (uint16 Address)
{
	return OpenBus;
}

static uint8 GetPPU_BUS1 (uint16 Address)
{
	return PPU.OpenBus1;
}


static uint8 GetPPU_213x (uint16 Address)
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
	return (PPU.OpenBus1 = Memory.FillRAM[Address]);
}

static uint8 GetPPU_2137 (uint16 Address)
{
	S9xLatchCounters(0);
	return OpenBus;
}

static uint8 GetPPU_2138 (uint16 Address)
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

static uint8 GetPPU_2139 (uint16 Address)
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

static uint8 GetPPU_213A (uint16 Address)
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

static uint8 GetPPU_213B (uint16 Address)
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
	    
static uint8 GetPPU_213C (uint16 Address)
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

static uint8 GetPPU_213D (uint16 Address)
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

static uint8 GetPPU_213E (uint16 Address)
{
    // PPU time and range over flags
    FLUSH_REDRAW ();

    //so far, 5c77 version is always 1.
	return (PPU.OpenBus1 = (Model->_5C77 | PPU.RangeTimeOver));
}

static uint8 GetPPU_213F (uint16 Address)
{
    // NTSC/PAL and which field flags
    PPU.VBeamFlip = PPU.HBeamFlip = 0;
        //neviksti found a 2 and a 3 here. SNEeSe uses a 3.
        //XXX: field flags not emulated
    return ((Settings.PAL ? 0x10 : 0) | (Memory.FillRAM[0x213f] & 0xc0)| Model->_5C78) | (~PPU.OpenBus2 & 0x20);
}

static uint8 GetPPU_APUR (uint16 Address)
{
/*
	case 0x2140: case 0x2141: case 0x2142: case 0x2143:
	case 0x2144: case 0x2145: case 0x2146: case 0x2147:
	case 0x2148: case 0x2149: case 0x214a: case 0x214b:
	case 0x214c: case 0x214d: case 0x214e: case 0x214f:
	case 0x2150: case 0x2151: case 0x2152: case 0x2153:
	case 0x2154: case 0x2155: case 0x2156: case 0x2157:
	case 0x2158: case 0x2159: case 0x215a: case 0x215b:
	case 0x215c: case 0x215d: case 0x215e: case 0x215f:
	case 0x2160: case 0x2161: case 0x2162: case 0x2163:
	case 0x2164: case 0x2165: case 0x2166: case 0x2167:
	case 0x2168: case 0x2169: case 0x216a: case 0x216b:
	case 0x216c: case 0x216d: case 0x216e: case 0x216f:
	case 0x2170: case 0x2171: case 0x2172: case 0x2173:
	case 0x2174: case 0x2175: case 0x2176: case 0x2177:
	case 0x2178: case 0x2179: case 0x217a: case 0x217b:
	case 0x217c: case 0x217d: case 0x217e: case 0x217f:
*/
#ifdef SPCTOOL
    return ((uint8) _SPCOutP [Address & 3]);
#else
//	CPU.Flags |= DEBUG_MODE_FLAG;
#ifdef SPC700_SHUTDOWN	
    IAPU.APUExecuting = Settings.APUEnabled;
    IAPU.WaitCounter++;
#endif
    if (Settings.APUEnabled)
    {
#ifdef CPU_SHUTDOWN
//		CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif	
		if (SNESGameFixes.APU_OutPorts_ReturnValueFix &&
		    Address >= 0x2140 && Address <= 0x2143 && !CPU.V_Counter)
		{
			return (uint8)((Address & 1) ? ((rand() & 0xff00) >> 8) : (rand() & 0xff));
		}
		return (APU.OutPorts [Address & 3]);
    }

    switch (Settings.SoundSkipMethod)
    {
    case 0:
    case 1:
		CPU.BranchSkip = TRUE;
		break;
    case 2:
		break;
    case 3:
		CPU.BranchSkip = TRUE;
		break;
    }
    if ((Address & 3) < 2)
    {
		int r = rand ();
		if (r & 2)
		{
		    if (r & 4)
			return ((Address & 3) == 1 ? 0xaa : 0xbb);
		    else
			return ((r >> 3) & 0xff);
		}
    }
    else
    {
		int r = rand ();
		if (r & 2)
		    return ((r >> 3) & 0xff);
    }
    return (Memory.FillRAM[Address]);
#endif // SPCTOOL
}

static uint8 GetPPU_2180 (uint16 Address)
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

uint8 (*GetPPU[])(uint16 Address) = {
	GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,
	GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,
	GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,
	GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,
	GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,
	GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS1, GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,
	GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_213x, GetPPU_213x, GetPPU_213x, GetPPU_2137,
	GetPPU_2138, GetPPU_2139, GetPPU_213A, GetPPU_213B, GetPPU_213C, GetPPU_213D, GetPPU_213E, GetPPU_213F,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR,
	GetPPU_2180, GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,
	GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,  GetPPU_BUS,
	GetPPU_BUS
};

uint8 S9xGetPPU (uint16 Address)
{
 	uint8 byte = OpenBus;

	if(Address<0x2100)//not a real PPU reg
		return OpenBus; //treat as unmapped memory returning last byte on the bus
    if (Address <= 0x2190){
		return GetPPU[Address - 0x2100](Address);
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
