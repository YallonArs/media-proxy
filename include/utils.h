#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;
typedef uint8_t byte;

int32_t pid_open_file(const std::string& target_path);
vector<string> globFiles(const std::string& pattern);
int randomint(int start, int end);
uint8_t randombyte();
