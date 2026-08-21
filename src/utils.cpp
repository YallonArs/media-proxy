#include <iostream>
#include <filesystem>
#include <glob.h>
#include <random>
#include <algorithm>

#include "utils.h"
#include "VideoDevice.h"
#include <cstring>

namespace fs = std::filesystem;

std::vector<std::string> globFiles(const std::string& pattern) {
    glob_t glob_result;
    std::vector<std::string> files;

    if (glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            files.push_back(glob_result.gl_pathv[i]);
        }
    }

    globfree(&glob_result);
    return files;
}

void print_usage(char *exec) {
	std::cout << "Usage:\n"
		 << exec << " /dev/videoX" << std::endl;
	std::cout << "Available devices:" << std::endl;
	for (auto filename : VideoDevice::list_all())
		std::cout << "- " << filename << std::endl;
}

string parse_args(int argc, char *argv[]) {
	if (argc != 2) {
		print_usage(argv[0]);
		throw std::runtime_error("invalid number of args");
	}
	if (strcmp(argv[1], "-h") == 0) {
		print_usage(argv[0]);
		return "";
	}
	if (!fs::exists(argv[1])) {
		throw std::runtime_error("Path " + string(argv[1]) + " does not exist!");
	}

	return argv[1];
}
