#include <iostream>
#include <filesystem>
#include <glob.h>
#include <random>

#include "utils.h"

namespace fs = std::filesystem;

int32_t pid_open_file(const std::string& target_path) {
    try {
        fs::path canonical_target = fs::canonical(target_path);

        // Iterate through all entries in /proc
        for (const auto& proc_entry : fs::directory_iterator("/proc")) {
            if (!proc_entry.is_directory()) continue;

            // Check if the directory name is a PID (numeric string)
            std::string pid = proc_entry.path().filename().string();
            if (!std::all_of(pid.begin(), pid.end(), ::isdigit)) continue;

            fs::path fd_dir = proc_entry.path() / "fd";
            if (!fs::exists(fd_dir)) continue;

            // Iterate through the open file descriptors of this process
            for (const auto& fd_entry : fs::directory_iterator(fd_dir)) {
                try {
                    // Resolve the symlink inside the fd folder
                    if (fs::is_symlink(fd_entry) && fs::read_symlink(fd_entry) == canonical_target) {
                        // std::cout << "File opened by PID: " << pid << "\n";
                        return std::stoi(pid);
                    }
                } catch (...) {
                    // Ignore permission errors for individual file descriptors
                    continue;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return -1;
}


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

int randomint(int start, int end) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	
	std::uniform_int_distribution<int> randombyte_(start, end);

	return randombyte_(gen);
}

uint8_t randombyte() {
	return static_cast<uint8_t>(randomint(0, 255));
}
