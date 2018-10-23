// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair


#ifndef MAME_MACHINE_SCC2698B_H
#define MAME_MACHINE_SCC2698B_H

class scc2698b_device;

#include "diserial.h"

#define SCC2698B_RX_FIFO_SIZE	3

// _port should be a letter port index from "a" .. "h"



class scc2698b_channel : public device_t, public device_serial_interface
{
	friend class scc2698b_device;
public:
	scc2698b_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(mpi0_w);
	DECLARE_WRITE_LINE_MEMBER(mpi1_w);

	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void write_TXH(int txh);
	int read_RXH();

	void reset_all();
	void reset_tx();
	void reset_rx();

	void set_tx_enable(bool enable);
	void set_rx_enable(bool enable);

	void update_serial_configuration();

	void set_tx_bittime(const attotime &bittime);
	void set_rx_bittime(const attotime &bittime);

	void set_mpp_output(bool output);


	int read_SR();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

//	devcb_write_line write_tx, write_mpp1, write_mpp2, write_mpo;

	int moderegister_ptr;

	int tx_transmitting;
	int tx_fifo;
	int tx_bytecount;
	u8 rx_fifo[SCC2698B_RX_FIFO_SIZE];
	int rx_bytecount;

	u8 MR1, MR2, SR, CR, CSR;

	bool tx_enable;
	bool rx_enable;

	void recompute_pin_output(bool force = false);
	int mpp1_value, mpp2_value;
	bool mpp_is_output;

	int channel_port;
	scc2698b_device* parent;
};

class scc2698b_device : public device_t
{
public:
	scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void write_reg(int offset, uint8_t data);
	uint8_t read_reg(int offset);

	template <class Object> devcb_base &set_intr_A(Object &&cb) { return write_intr_A.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intr_B(Object &&cb) { return write_intr_B.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intr_C(Object &&cb) { return write_intr_C.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intr_D(Object &&cb) { return write_intr_D.set_callback(std::forward<Object>(cb)); }

	required_device_array<scc2698b_channel, 8> m_channel;

	DECLARE_WRITE_LINE_MEMBER(port_a_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_b_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_c_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_d_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_e_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_f_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_g_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_h_rx_w);


	void write_line_tx(int port, int value);
	void write_line_mpp1(int port, int value);
	void write_line_mpp2(int port, int value);
	void write_line_mpo(int port, int value);


	auto tx_callback(char port) {
		switch (port) {
		case 'a': return write_a_tx.bind();
		case 'b': return write_b_tx.bind();
		case 'c': return write_c_tx.bind();
		case 'd': return write_d_tx.bind();
		case 'e': return write_e_tx.bind();
		case 'f': return write_f_tx.bind();
		case 'g': return write_g_tx.bind();
		case 'h': return write_h_tx.bind();
		default:
			fatalerror("Wrong port for tx_callback\n");			
		}
	}

	auto mpp1_callback(char port) {
		switch (port) {
		case 'a': return write_a_mpp1.bind();
		case 'b': return write_b_mpp1.bind();
		case 'c': return write_c_mpp1.bind();
		case 'd': return write_d_mpp1.bind();
		case 'e': return write_e_mpp1.bind();
		case 'f': return write_f_mpp1.bind();
		case 'g': return write_g_mpp1.bind();
		case 'h': return write_h_mpp1.bind();
		default:
			fatalerror("Wrong port for mpp1_callback\n");
		}
	}

	auto mpp2_callback(char port) {
		switch (port) {
		case 'a': return write_a_mpp2.bind();
		case 'b': return write_b_mpp2.bind();
		case 'c': return write_c_mpp2.bind();
		case 'd': return write_d_mpp2.bind();
		case 'e': return write_e_mpp2.bind();
		case 'f': return write_f_mpp2.bind();
		case 'g': return write_g_mpp2.bind();
		case 'h': return write_h_mpp2.bind();
		default:
			fatalerror("Wrong port for mpp2_callback\n");
		}
	}

	auto mpo_callback(char port) {
		switch (port) {
		case 'a': return write_a_mpo.bind();
		case 'b': return write_b_mpo.bind();
		case 'c': return write_c_mpo.bind();
		case 'd': return write_d_mpo.bind();
		case 'e': return write_e_mpo.bind();
		case 'f': return write_f_mpo.bind();
		case 'g': return write_g_mpo.bind();
		case 'h': return write_h_mpo.bind();
		default:
			fatalerror("Wrong port for mpo_callback\n");
		}
	}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

	devcb_write_line write_intr_A, write_intr_B, write_intr_C, write_intr_D;
	devcb_write_line write_a_tx, write_b_tx, write_c_tx, write_d_tx, write_e_tx, write_f_tx, write_g_tx, write_h_tx;
	devcb_write_line write_a_mpp1, write_b_mpp1, write_c_mpp1, write_d_mpp1, write_e_mpp1, write_f_mpp1, write_g_mpp1, write_h_mpp1;
	devcb_write_line write_a_mpp2, write_b_mpp2, write_c_mpp2, write_d_mpp2, write_e_mpp2, write_f_mpp2, write_g_mpp2, write_h_mpp2;
	devcb_write_line write_a_mpo, write_b_mpo, write_c_mpo, write_d_mpo, write_e_mpo, write_f_mpo, write_g_mpo, write_h_mpo;

	void log_register_access(int offset, int value, const char* direction, const char* reg_name);

	scc2698b_channel* get_channel(int port);

	void reset_port(int port);
	void reset_port_tx(int port);
	void reset_port_rx(int port);

	void write_MR(int port, int value);
	void write_CSR(int port, int value);
	void write_CR(int port, int value);
	void write_THR(int port, int value);

	int read_MR(int port);
	int read_SR(int port);
	int read_RHR(int port);

	void update_block_baudrate(int block);
	void update_port_baudrate(int port);
	attotime generate_baudrate(int block, int tx, int table_index);


private:
	struct Block {
		u8 ACR;
	};

	Block m_blocks[4];

};

DECLARE_DEVICE_TYPE(SCC2698B, scc2698b_device)
DECLARE_DEVICE_TYPE(SCC2698B_CHANNEL, scc2698b_channel)

#endif // MAME_MACHINE_SCC2698B_H
