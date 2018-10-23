// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 System Monitor Input/Output Card emulation

**********************************************************************/

/*
    Device SMIOC
    Board Copyright - IBM Corp 1989 Made in USA

    Labels:
        * 98R0083
          MN 90770AR

        * EC# A65048R

    Hardware:
        * CPU - Intel N80C188 L0450591 @ ??MHz - U23
        * MCU - Signetics SC87C451CCA68 220CP079109KA 97D8641 - U70
        * DMA - KS82C37A - U46, U47, U48, U49, U50
    * SCC - Signetics SCC2698BC1A84 - U67
        * Memory - NEC D43256AGU-10LL 8948A9038 SRAM 32KB - U51
        * Memory - Mitsubishi M5M187AJ 046101-35 SRAM 64K X 1?? - U37
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U53
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U54

    Logic:
        * U8 - 22V10-25JC
        * U33 - 22V10-25JC
        * U61 - 22V10-25JC
        * U63 - 22V10-25JC
        * U87 - 22V10-20JC
        * U88 - 22V10-20JC
        * U102 - 22V10-25JC
        * U111 - 22V10-25JC

    Switches:
        * S1 - Board reset

    Program Memory:
        * 0x00000 - 0x07FFF : SRAM D43256AGU-10LL 32KB
        * 0xF8000 - 0xFFFFF : ROM 27C256 PLCC32 32KB
    * 0xC0080 - 0xC008F : KS82C37A - Probably RAM DMA
    * 0xC0090 - 0xC009F : KS82C37A - Serial DMA (Port 1 and 2?)
    * 0xC00A0 - 0xC00AF : KS82C37A - Serial DMA (Port 3 and 4?)
    * 0xC00B0 - 0xC00BF : KS82C37A - Serial DMA (Port 5 and 6?)
    * 0xC00C0 - 0xC00CF : KS82C37A - Serial DMA (Port 7 and 8?)

    IO Memory:
        * Unknown

    TODO:
    * Emulate SCC and hook up RS232 ports
    * Hook up console to RS232 port 1
    * Hook up System Monitor II to RS232 port 2
    * Dump 87C451 rom data and emulate MCU
    * Dump 87C51 on SMIOC interconnect box
    * Dump all PAL chips
    * Hook up status LEDs
*/

#include "emu.h"
#include "smioc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I188_TAG     "smioc_i188" // U23

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMIOC, smioc_device, "rolm_smioc", "ROLM SMIOC")

//-------------------------------------------------
//  ROM( SMIOC )
//-------------------------------------------------

ROM_START( smioc )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "smioc.27256.u65", 0x0000, 0x8000, CRC(25b93192) SHA1(8ee9879033623490ce6703ba5429af2b16dbae84) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *smioc_device::device_rom_region() const
{
	return ROM_NAME( smioc );
}

//-------------------------------------------------
//  ADDRESS_MAP( smioc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( smioc_mem, AS_PROGRAM, 8, smioc_device )
	AM_RANGE(0x00000, 0x07FFF) AM_RAM AM_SHARE("smioc_ram")
	AM_RANGE(0x40000, 0x4FFFF) AM_READWRITE(ram2_mmio_r, ram2_mmio_w)
	AM_RANGE(0xC0080, 0xC008F) AM_DEVREADWRITE("dma8237_1",am9517a_device,read,write) // Probably RAM DMA
	AM_RANGE(0xC0090, 0xC009F) AM_DEVREADWRITE("dma8237_2",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00A0, 0xC00AF) AM_DEVREADWRITE("dma8237_3",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00B0, 0xC00BF) AM_DEVREADWRITE("dma8237_4",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00C0, 0xC00CF) AM_DEVREADWRITE("dma8237_5",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC0100, 0xC011F) AM_READWRITE(boardlogic_mmio_r, boardlogic_mmio_w)
	AM_RANGE(0xC0200, 0xC023F) AM_READWRITE(scc2698b_mmio_r, scc2698b_mmio_w) // Future: Emulate the SCC2698B UART as a separate component
	AM_RANGE(0xF8000, 0xFFFFF) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

MACHINE_CONFIG_START(smioc_device::device_add_mconfig)
	/* CPU - Intel 80C188 */
	MCFG_DEVICE_ADD(I188_TAG, I80188, XTAL(20'000'000) / 2) // Clock division unknown
	MCFG_DEVICE_PROGRAM_MAP(smioc_mem)

	/* DMA */
	for (required_device<am9517a_device> &dma : m_dma8237)
		AM9517A(config, dma, 20_MHz_XTAL / 4); // Clock division unknown

	/* RS232 */
	/* Port 1: Console */
	for (required_device<rs232_port_device> &rs232_port : m_rs232_p)
		RS232_PORT(config, rs232_port, default_rs232_devices, nullptr);
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  smioc_device - constructor
//-------------------------------------------------

smioc_device::smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMIOC, tag, owner, clock),
	m_smioccpu(*this, I188_TAG),
	m_dma8237(*this, "dma8237_%u", 1),
	m_rs232_p(*this, "rs232_p%u", 1),
	m_smioc_ram(*this, "smioc_ram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smioc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smioc_device::device_reset()
{
	/* Reset CPU */
	m_smioccpu->reset();
}


void smioc_device::SendCommand(u16 command)
{
	m_commandValue = command;
	m_requestFlags_11D |= 1;
	
	m_smioccpu->set_input_line(INPUT_LINE_IRQ2, HOLD_LINE);
	
}


void smioc_device::update_and_log(u16& reg, u16 newValue, const char* register_name)
{
	if (reg != newValue)
	{
		logerror("Update %s %04X -> %04X\n", register_name, reg, newValue);
		reg = newValue;
	}
}

READ8_MEMBER(smioc_device::ram2_mmio_r)
{
	u8 data = 0;
	switch (offset)
	{
	case 0xCC2: // Command from 68k?
		data = m_commandValue & 0xFF;
		break;
	case 0xCC3:
		data = m_commandValue >> 8;
		break;
	}


	logerror("ram2[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::ram2_mmio_w)
{
	switch (offset) // Offset based on C0100 register base
	{
	case 0xC84:
		update_and_log(m_status2, (m_status2 & 0xFF00) | data, "Status2[40C84]");
		return;
	case 0xC85:
		update_and_log(m_status2, (m_status2 & 0xFF) | (data<<8), "Status2[40C85]");
		return;

	case 0xC88:
		update_and_log(m_status, (m_status & 0xFF00) | data, "Status[40C88]");
		return;
	case 0xC89:
		update_and_log(m_status, (m_status & 0xFF) | (data<<8), "Status[40C89]");
		return;

	}


	logerror("ram2[%04X] <= %02X\n", offset, data);
}


READ8_MEMBER(smioc_device::boardlogic_mmio_r)
{
	u8 data = 0xFF;
	switch (offset)
	{
		case 0x1D: // C011D (HW Request flags)
			data = m_requestFlags_11D;
			break;

	}
	logerror("logic[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::boardlogic_mmio_w)
{
	switch (offset)
	{
	case 0x10: // C0110 (Command complete?)
		m_requestFlags_11D = 0;
		break;

	}
	logerror("logic[%04X] <= %02X\n", offset, data);
}



READ8_MEMBER(smioc_device::scc2698b_mmio_r)
{
	return 0;
}

WRITE8_MEMBER(smioc_device::scc2698b_mmio_w)
{

}

