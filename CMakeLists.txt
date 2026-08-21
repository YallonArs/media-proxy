cmake_minimum_required(VERSION 3.20)
project(media-proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-g -Wall -Wextra -O2 -fsanitize=undefined,address)
add_link_options(-fsanitize=undefined,address)

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS core videoio highgui)

# PipeWire
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

file(GLOB_RECURSE MAIN_SRCS src/*.cpp)

add_executable(main ${MAIN_SRCS})

target_include_directories(main PRIVATE
    include
    ${OpenCV_INCLUDE_DIRS}
    ${PIPEWIRE_INCLUDE_DIRS}
)

target_link_libraries(main PRIVATE
    ${OpenCV_LIBS}
    ${PIPEWIRE_LIBRARIES}
)

target_compile_options(main PRIVATE ${PIPEWIRE_CFLAGS_OTHER})
