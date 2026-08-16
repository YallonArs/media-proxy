# 1. Compiler and Flags
CXX      := g++
CXXFLAGS := -g -std=c++23 -Wall -Wextra -O2 -fsanitize=undefined,address
LDFLAGS  := 

# 2. Target Executable Names
TARGET       := main

# 3. Directories
SRC_DIR  := src
OBJ_DIR  := .build
INCLUDE_DIR := include

CXXFLAGS += -I${INCLUDE_DIR}

CXXFLAGS := $(CXXFLAGS) -lopencv_core -lopencv_videoio -lopencv_highgui

# 4. Source and Object Files
MAIN_OBJS  := $(MAIN_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/main_%.o)

# 5. Default Rule
all: $(TARGET) $(CHILD_TARGET)

# 6. Linking Rules
$(TARGET): $(MAIN_OBJS)
	@echo "Linking executable: $@"
	$(CXX) $(MAIN_CXXFLAGS) $(LDFLAGS) $^ -o $@

# 7. Compilation Rules
$(OBJ_DIR)/main_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling (main): $<"
	$(CXX) $(MAIN_CXXFLAGS) -c $< -o $@

# 8. Clean Rule
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(TARGET)
