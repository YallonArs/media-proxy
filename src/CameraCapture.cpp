#include "CameraCapture.h"

#include <stdexcept>

CameraCapture::CameraCapture(const uint8_t id) : VideoDevice(id) {};
CameraCapture::CameraCapture(const string path) : CameraCapture(VideoDevice::path_to_idx(path)) {};

CameraCapture::~CameraCapture() {
	if (capture.isOpened())
		capture.release();
}

void CameraCapture::open() {
	capture = cv::VideoCapture(id, cv::CAP_V4L2);
	if (!capture.isOpened()) {
		throw std::runtime_error("Could not open camera");
	}

	// set max available resolution
	capture.set(cv::CAP_PROP_FRAME_WIDTH, 10000);
	capture.set(cv::CAP_PROP_FRAME_HEIGHT, 10000);

	// read resolution actually set
	resolution = {
		capture.get(cv::CAP_PROP_FRAME_WIDTH),
		capture.get(cv::CAP_PROP_FRAME_HEIGHT)
	};
	frame_size_bytes = resolution.pixel_count() * 3;
}

cv::Mat CameraCapture::read_frame() {
	cv::Mat frame;
	capture >> frame;
	return frame;
}

uint32_t CameraCapture::get_frame_size_bytes() const {
	return frame_size_bytes;
}
