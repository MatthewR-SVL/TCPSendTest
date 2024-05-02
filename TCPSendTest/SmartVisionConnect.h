#pragma once

#include <string>

class SmartVisionConnect{
public:
	SmartVisionConnect();

	struct discoveredDevice {
		const char* ip_addr;
		const char* adapter_addr;
		char* device_type;
	};

	//members
	SmartVisionConnect::discoveredDevice* devices[100];
	int initialized;

	//methods
	int discover_devices();

private:
	std::string send_msg_to_device(const char* port, const char* address, const char* sendbuf);

};

