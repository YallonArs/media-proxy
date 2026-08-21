#pragma once

#include "VideoDevice.h"
#include <string>
#include <opencv2/opencv.hpp>

class VirtualCamera : public VideoDevice {
protected:
	int fd;

public:
	VirtualCamera(const uint8_t id, const Resolution size, const string& name);
	VirtualCamera(const string path, const Resolution size, const string& name);
	~VirtualCamera();
	string name;

	void configure();
	void open();
	int write_frame(byte* frame, uint32_t length);
	int write_frame(cv::Mat frame);
};
