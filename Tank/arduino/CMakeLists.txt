cmake_minimum_required(VERSION 3.6)
project(tank_arduino)

include (./arduino_modules/include_directories.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    BatteryController.cpp
    BatteryController.h
    DisplayController.cpp
    DisplayController.h
    GunController.cpp
    GunController.h
    MessageController.cpp
    MessageController.h
    Tank.cpp
    Tank.h)

add_executable(tank_arduino ${SOURCE_FILES})