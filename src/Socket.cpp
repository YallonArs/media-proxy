#include "Socket.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Socket::Socket(const std::string path) : path(path) {};

Socket::~Socket() {
	if (is_opened)
		close(fd);
};

void Socket::connect() {
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		throw std::runtime_error("Failed to create socket");
	}

	sockaddr_un serverAddr {};
	serverAddr.sun_family = AF_UNIX;

	// Ensure the path string fits in the sun_path buffer to avoid overflow
	if (path.length() >= sizeof(serverAddr.sun_path)) {
		close(fd);
		throw std::runtime_error("Socket path is too long");
	}
	std::strncpy(serverAddr.sun_path, path.c_str(), sizeof(serverAddr.sun_path) - 1);

	// 4. Connect to the server socket
	if (::connect(fd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
		close(fd);
		throw std::runtime_error("Connection failed");
	}

	// 4. Set the socket to non-blocking mode
	int flags = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(fd);
	}

	is_opened = true;
}

bool Socket::check_data() {
	char buf[64];
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n > 0) return true;
	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return false;
	if (n == 0 || n == -1)
		std::cout << "Socket disconnected or error occurred." << std::endl;
	return false;
}
