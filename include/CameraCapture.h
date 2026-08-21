#pragma once

#include <opencv2/opencv.hpp>

#include "VideoDevice.h"


class CameraCapture : public VideoDevice {
protected:
	uint32_t frame_size_bytes;
	cv::VideoCapture capture;

public:
	using VideoDevice::VideoDevice;
	~CameraCapture();

	void open();
	cv::Mat read_frame();
	uint32_t get_frame_size_bytes() const;
};
