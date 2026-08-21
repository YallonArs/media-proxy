#include <functional>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>

#include "CameraCapture.h"
#include "VirtualCamera.h"

class CameraStream {
protected:
	CameraCapture capture;
	VirtualCamera virtual_camera;
	std::function<void(cv::Mat, byte*)> filter_callback;

	uint32_t frame_size_bytes;
	cv::Mat frame;
	byte *new_frame;
	std::atomic<bool> is_started;
	std::jthread thread;

	void start(std::atomic<bool>& is_paused);

public:
	CameraStream(CameraCapture &capture, VirtualCamera &virtual_camera, std::function<void(cv::Mat, byte*)> filter_callback);
	~CameraStream();

	void start_in_thread(std::atomic<bool>& is_paused);
	void stop();
};
