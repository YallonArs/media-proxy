#include "CameraStream.h"

CameraStream::CameraStream(CameraCapture &capture, VirtualCamera &virtual_camera, std::function<void(cv::Mat, byte *)> filter_callback) : capture(capture), virtual_camera(virtual_camera), filter_callback(filter_callback) {
	uint32_t frame_size_bytes = capture.get_frame_size_bytes();
	new_frame				  = new byte[frame_size_bytes];
}

CameraStream::~CameraStream() {
	stop();
	delete[] new_frame;
}

void CameraStream::start(std::atomic<bool> &is_paused) {
	is_started = true;

	while (is_started) {
		if (!is_paused)
			frame = capture.read_frame();

		if (frame.empty()) continue;

		filter_callback(frame, new_frame);

		virtual_camera.write_frame(new_frame, capture.get_frame_size_bytes());

		if (cv::waitKey(1)) {};
	}
}

void CameraStream::start_in_thread(std::atomic<bool> &is_paused) {
	thread = std::jthread([this, &is_paused] { start(is_paused); });
}

void CameraStream::stop() {
	if (!is_started)
		throw std::runtime_error("attempting to stop stream that is not started");
	
	is_started = false;
}
