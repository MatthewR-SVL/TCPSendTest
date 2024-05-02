#ifndef TCPIO_H
#define TCPIO_H

#include <string>

class TCPIO {
public:
	TCPIO(const char* ip);

	enum TriggerMode
	{
		Standard = 0, FreeRun = 2, PulseInitiated = 1
	};

	// members
	const char* ip_addr;
	const char* port = "1199";
	std::string mac_addr;
	std::string gateway;
	std::string subnet;
	bool dhcp_enabled;
	std::string firmware_version;

	bool pnp_passthrough;
	bool pnp_state;
	bool npn_passthrough;
	bool npn_state;
	bool do3_state;
	bool do4_state;
	int analog_passthrough;
	int analog_state;
	int input_polarity;
	int trigger_mode;
	int trigger_delay;
	long on_time;
	long off_time;
	int repeat_count;

	void get_info();
	void set_io();
	void set_io(bool pnp_state, bool npn_state, bool do3_state, bool do4_state, int analog_state);
	void set_io(long on_time, long off_time, int repeat_count);

private:
	// methods
	std::string send_msg_to_device(const char* port, const char* address, const char* sendbuf);
};

#endif  // TCPIO_HPP
