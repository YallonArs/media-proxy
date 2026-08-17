#include <atomic>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <random>
#include <vector>

#include <fcntl.h>
#include <glob.h>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "CameraCapture.h"
#include "Socket.h"
#include "VideoDevice.h"
#include "VirtualCamera.h"
#include "utils.h"

using std::cout;
using std::endl;
namespace fs = std::filesystem;

const uint16_t VIRTUAL_CAMERA_ID = 10;
const string SOCKET_PATH		 = "./hotkey-socket.s";

volatile sig_atomic_t g_signal_received = 0;
std::atomic<bool> is_paused				= false;

inline void signal_handler(int _signum) {
	g_signal_received = 1;
}

void filter(cv::Mat frame, byte *new_frame) {
	for (uint32_t i = 0; i < frame.total() * 3; i++) {
		new_frame[i] = 255 - frame.data[i];
	}
}

void trigger_hotkey() {
	is_paused = !is_paused;
	cout << "cpp: hotkey detected" << endl;
}

int main(int argc, char *argv[]) {
	// some shit
	string path_to_camera = parse_args(argc, argv);
	if (path_to_camera == "") return 0;

	CameraCapture capture(path_to_camera);
	capture.open();

	VirtualCamera virtual_camera(VIRTUAL_CAMERA_ID, capture.resolution, "Virtual Camera by YallonArs");
	virtual_camera.configure();
	virtual_camera.open();

	std::signal(SIGINT, signal_handler);
	cout << "ctrl-c hadndler registered" << endl;

	Socket sock(SOCKET_PATH);
	sock.connect();
	cout << "socket created" << endl;

	cv::Mat frame;
	cout << "starting" << endl;

	uint32_t frame_size_bytes = capture.get_frame_size_bytes();
	vector<byte> new_frame(frame_size_bytes);

	while (!g_signal_received) {
		if (!is_paused) {
			frame = capture.read_frame();
		}

		if (frame.empty()) continue;

		// filter(frame, new_frame);
		memcpy(new_frame.data(), frame.data, frame_size_bytes);

		if (sock.check_data()) {
			trigger_hotkey();
		}

		virtual_camera.write_frame(new_frame.data(), capture.get_frame_size_bytes());
		if (cv::waitKey(1)) {};
	}

	return 0;
}
