#pragma once

#include <string>

class Socket {
protected:
	std::string path;
	int fd;
	bool is_opened = false;
public:
	Socket(const std::string path);
	~Socket();

	void connect();
	bool check_data();
};
