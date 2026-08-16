#include "VirtualCamera.h"

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdexcept>

#include "utils.h"

namespace fs = std::filesystem;

VirtualCamera::VirtualCamera(const uint8_t id, const Resolution resolution, const std::string &name) : VideoDevice(id, resolution), name(name) {
	if (fs::exists(path)) {
		std::cerr << "cannot create " + path + ", it already exists.";
	}

	const string command = std::format("sudo modprobe v4l2loopback video_nr={} width={} height={} exclusive_caps=1 card_label=\"{}\"", id, resolution.width, resolution.height, name);

	int result = std::system(command.c_str());
	if (WEXITSTATUS(result) != 0) {
		throw std::runtime_error("Cannot create virtual camera: exit code " + std::to_string(WEXITSTATUS(result)));
	}
}

VirtualCamera::VirtualCamera(const string path, const Resolution size, const string& name) : VirtualCamera(VideoDevice::path_to_idx(path), resolution, name) {}

VirtualCamera::~VirtualCamera() {
	// const string command = "sudo v4l2loopback-ctl delete " + path;
	close(fd);
	
	int result = std::system("sudo modprobe -r v4l2loopback");
	if (WEXITSTATUS(result) != 0) {
		throw std::runtime_error("Cannot remove virtual camera: exit code " + std::to_string(WEXITSTATUS(result)));
	}
}

void VirtualCamera::configure() {
	struct v4l2_format vid_format;
	memset(&vid_format, 0, sizeof(vid_format));
	vid_format.type					= V4L2_BUF_TYPE_VIDEO_OUTPUT;
	vid_format.fmt.pix.width		= resolution.width;
	vid_format.fmt.pix.height		= resolution.height;
	vid_format.fmt.pix.pixelformat	= V4L2_PIX_FMT_BGR24; // Force 24-bit BGR
	vid_format.fmt.pix.sizeimage	= resolution.width * resolution.height * 3;
	vid_format.fmt.pix.field		= V4L2_FIELD_NONE;
	vid_format.fmt.pix.bytesperline = resolution.width * 3;
	vid_format.fmt.pix.colorspace	= V4L2_COLORSPACE_SRGB;

	int v4l2_fd = ::open(path.c_str(), O_WRONLY);
	if (ioctl(v4l2_fd, VIDIOC_S_FMT, &vid_format) < 0) {
		close(v4l2_fd);
		throw std::runtime_error("Failed to set video format on virtual camera");
	}
	close(v4l2_fd);
}

void VirtualCamera::open() {
	fd = ::open(path.c_str(), O_WRONLY);
	if (fd < 0)
		throw std::runtime_error("Failed to open " + path);
}

int VirtualCamera::write_frame(byte* frame, uint32_t length) {
	if (::write(fd, frame, length) == -1)
		throw std::runtime_error("cannot write frame");
}
