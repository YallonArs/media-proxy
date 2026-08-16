#pragma once

#include <string>
#include "utils.h"

class Socket {
protected:
	string path;
	int fd;
	bool is_opened = false;
public:
	Socket(const string path);
	~Socket();

	void connect();
	bool check_data();
};
