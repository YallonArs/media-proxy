#include "VideoDevice.h"
#include "utils.h"

#include <stdexcept>
#include <vector>
#include <cstdint>

uint32_t Resolution::pixel_count() const {
	return width * height;
}

Resolution::Resolution() : width(0), height(0) {}
Resolution::Resolution(const uint16_t width, const uint16_t height) : width(width), height(height) {}

Resolution::Resolution(const string s, const char delim) {
	size_t pos = s.find(delim);

	if (pos != s.rfind(delim)) {
		throw std::runtime_error("cannot parse string '" + s + "'");
	}

	width = std::stoi(s.substr(0, pos));
	height = std::stoi(s.substr(pos + 1));
}

VideoDevice::VideoDevice(const uint8_t id, const Resolution resolution) : id(id), resolution(resolution) {
	path = "/dev/video" + std::to_string(id);
}
VideoDevice::VideoDevice(const string path, const Resolution resolution) : path(path), resolution(resolution) {
	id = path_to_idx(path);
}

uint8_t VideoDevice::get_id() const {
	return id;
}

string VideoDevice::get_path() const {
	return path;
}

uint8_t VideoDevice::path_to_idx(const string path) {
	// 10 = length of string "/dev/video"

	if (path.size() <= 10) {
		throw std::runtime_error(path + " cannot be parsed!");
	}
	return std::stoi(path.substr(10, path.size() - 10));
}

std::vector<string> VideoDevice::list_all() {
	return globFiles("/dev/video*");
}
