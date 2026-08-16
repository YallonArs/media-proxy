# 1. Compiler and Flags
CXX      := g++
CXXFLAGS := -g -std=c++23 -Wall -Wextra -O2 -fsanitize=undefined,address
LDFLAGS  := 

# 2. Target Executable Names
TARGET       := main
CHILD_TARGET := hotkey-child

# 3. Directories
SRC_DIR  := src
OBJ_DIR  := .build
INCLUDE_DIR := include

CXXFLAGS += -I${INCLUDE_DIR} -I/usr/include/opencv4

# Flags only for the main binary
MAIN_CXXFLAGS := $(CXXFLAGS) -lopencv_core -lopencv_videoio -lopencv_highgui -lX11

# 4. Source and Object Files
MAIN_SRCS  := $(filter-out $(SRC_DIR)/wayland_hotkey.cpp $(SRC_DIR)/hotkey_child.cpp, $(wildcard $(SRC_DIR)/*.cpp))
MAIN_OBJS  := $(MAIN_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/main_%.o)

CHILD_SRCS := $(SRC_DIR)/hotkey_child.cpp $(SRC_DIR)/wayland_hotkey.cpp
CHILD_OBJS := $(CHILD_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/child_%.o)

# 5. Default Rule
all: $(TARGET) $(CHILD_TARGET)

# 6. Linking Rules
$(TARGET): $(MAIN_OBJS)
	@echo "Linking executable: $@"
	$(CXX) $(MAIN_CXXFLAGS) $(LDFLAGS) $^ -o $@

$(CHILD_TARGET): $(CHILD_OBJS)
	@echo "Linking executable: $@"
	$(CXX) $(CHILD_CXXFLAGS) $(LDFLAGS) $^ -o $@

# 7. Compilation Rules
$(OBJ_DIR)/main_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling (main): $<"
	$(CXX) $(MAIN_CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/child_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling (child): $<"
	$(CXX) $(CHILD_CXXFLAGS) -c $< -o $@

# 8. Clean Rule
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(TARGET) $(CHILD_TARGET)
