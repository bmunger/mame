// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

*/

#ifndef _E06200_H_
#define _E06200_H_

#include "emu.h"


class e0c6200_cpu_device : public cpu_device
{
public:
	// construction/destruction
	e0c6200_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, 16, prgwidth, -1, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, datawidth, 0, data)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
	{ }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 12; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();
	virtual void execute_one();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	void state_string_export(const device_state_entry &entry, std::string &str);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;
	
	UINT16 m_op;
	UINT16 m_prev_op;
	int m_icount;

	UINT16 m_pc;            // 13-bit programcounter: 1-bit bank, 4-bit page, 8-bit 'step'
	UINT16 m_prev_pc;
	UINT16 m_npc;           // new bank/page prepared by pset
	UINT16 m_jpc;           // actual bank/page destination for jumps
	
	// all work registers are 4-bit
	UINT8 m_a;              // accumulator
	UINT8 m_b;              // generic
	UINT8 m_xp, m_xh, m_xl; // 12-bit index register when combined
	UINT8 m_yp, m_yh, m_yl; // "
	UINT8 m_sp;             // stackpointer (SPH, SPL)
	UINT8 m_f;              // flags

	// misc internal helpers
	inline void set_zf(UINT8 data) { m_f = (m_f & ~2) | ((data & 0xf) ? 0 : 2); }
	
	// internal read/write (MX, MY, Mn/RP)
	inline UINT8 read_mx() { return m_data->read_byte(m_xp << 8 | m_xh << 4 | m_xl) & 0xf; }
	inline void write_mx(UINT8 data) { m_data->write_byte(m_xp << 8 | m_xh << 4 | m_xl, data); }
	inline UINT8 read_my() { return m_data->read_byte(m_yp << 8 | m_yh << 4 | m_yl) & 0xf; }
	inline void write_my(UINT8 data) { m_data->write_byte(m_yp << 8 | m_yh << 4 | m_yl, data); }
	inline UINT8 read_mn() { return m_data->read_byte(m_op & 0xf) & 0xf; }
	inline void write_mn(UINT8 data) { m_data->write_byte(m_op & 0xf, data); }

	inline void inc_x() { m_xl++; m_xh = (m_xh + (m_xl >> 4 & 1)) & 0xf; m_xl &= 0xf; }
	inline void inc_y() { m_yl++; m_yh = (m_yh + (m_yl >> 4 & 1)) & 0xf; m_yl &= 0xf; }

	// common stack ops
	inline void push(UINT8 data) { m_data->write_byte(--m_sp, data); }
	inline UINT8 pop() { return m_data->read_byte(m_sp++) & 0xf; }
	inline void push_pc() { push(m_pc >> 8 & 0xf); push(m_pc >> 4 & 0xf); push(m_pc & 0xf); }
	inline void pop_pc() { UINT16 bank = m_pc & 0x1000; m_pc = pop(); m_pc |= pop() << 4; m_pc |= pop() << 8; m_pc |= bank; }
	
	// opcode handlers
	//UINT8 op_rlc(UINT8 data);
	//UINT8 op_rrc(UINT8 data);
};


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type EPSON_E0C6S46;


#endif /* _E06200_H_ */
