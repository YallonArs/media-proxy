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
#include "PipewireFilter.h"
#include "Socket.h"
#include "VideoDevice.h"
#include "VirtualCamera.h"
#include "utils.h"
#include "CameraStream.h"

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

void filter_video(cv::Mat frame, byte *new_frame) {
	memcpy(new_frame, frame.data, frame.total() * frame.channels());
}

void filter_audio(float *src, float *dst, uint32_t n_samples) {
	if (is_paused)
		memset(dst, 0, n_samples * sizeof(float));
	else {
		memcpy(dst, src, n_samples * sizeof(float));
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

	PipewireFilterProps props;
	props.name        = "media-proxy-filter";
	props.description = "Filter by YallonArs";
	props.server_sock = "/run/user/1000/pipewire-0";
	props.callback    = filter_audio;

	PipewireFilter filter(argc, argv, props);
	filter.start_in_thread();
	cout << "pipewire filter started" << endl;
	
	CameraCapture capture(path_to_camera);
	capture.open();
	cout << "capture opened" << endl;
	
	VirtualCamera virtual_camera(VIRTUAL_CAMERA_ID, capture.resolution, "Virtual Camera by YallonArs");
	virtual_camera.configure();
	cout << "virtual camera configured" << endl;
	virtual_camera.open();
	
	CameraStream stream(capture, virtual_camera, filter_video);
	stream.start_in_thread(is_paused);
	cout << "stream started" << endl;

	Socket sock(SOCKET_PATH);
	sock.connect();
	cout << "socket connected" << endl;
	
	std::signal(SIGINT, signal_handler);
	cout << "ctrl-c hadndler registered" << endl;

	cout << "starting" << endl;

	while (!g_signal_received) {
		if (sock.check_data()) {
			trigger_hotkey();
		}
	}

	return 0;
}
