#pragma once

#include <string>
#include <vector>
#include <cstdint>

using std::string;
using std::vector;
typedef uint8_t byte;

vector<string> globFiles(const std::string& pattern);
void print_usage(char *exec);
string parse_args(int argc, char *argv[]);
