cmake_minimum_required(VERSION 3.6)
project(smarthome_arduino)

include (./arduino_modules/include_directories.cmake)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    DisplayController.cpp
    DisplayController.h
    IconController.cpp
    IconController.h
    MessageController.cpp
    MessageController.h
    PinController.cpp
    PinController.h
    SmartHome.cpp
    SmartHome.h)

add_executable(smarthome_arduino ${SOURCE_FILES})