cmake_minimum_required(VERSION 3.10)
project(auto_watering_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

# yaml-cpp is only needed from Phase 3 onward. Uncomment once you get there:
# pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_executable(water
    src/main.cpp
    src/Pump.cpp
)

target_include_directories(water PRIVATE include ${GPIOD_INCLUDE_DIRS})
target_link_libraries(water PRIVATE ${GPIOD_LIBRARIES})

# On Raspberry Pi OS: sudo apt install libgpiod-dev pkg-config cmake build-essential
