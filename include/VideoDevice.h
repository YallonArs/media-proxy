#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "utils.h"

struct Resolution {
	uint16_t width, height;

	Resolution();
	Resolution(const uint16_t width, const uint16_t height);
	Resolution(const string s, const char delim = 'x');
	
	uint32_t pixel_count() const;
};


class VideoDevice {
protected:
	string path;
	uint8_t id;

public:
	VideoDevice(const uint8_t id, const Resolution resolution = Resolution());
	VideoDevice(const string path, const Resolution resolution = Resolution());
	
	Resolution resolution;
	uint8_t get_id() const;
	string get_path() const;
	static uint8_t path_to_idx(const string path);
	static std::vector<string> list_all();
};
