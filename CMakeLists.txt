cmake_minimum_required(VERSION 3.20)
project(media-proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -Og -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -fsanitize=address,undefined,leak -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -flto=auto -march=native -s -Wl,--gc-sections")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "-fsanitize=address,undefined,leak")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

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
