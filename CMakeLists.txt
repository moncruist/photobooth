cmake_minimum_required(VERSION 3.10)
project(photobooth)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

option(RPI "Build for Raspberry Pi" OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(Boost_USE_STATIC_LIBS ON)

# Find the QtWidgets library
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS log REQUIRED )
find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED )
find_package(Gphoto2 REQUIRED)
find_package(Threads REQUIRED)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(DSLR_SOURCES
        dslr/DslrCamera.cpp
        dslr/DslrCamera.h
        dslr/DslrCameraInfo.cpp
        dslr/DslrCameraInfo.h
        dslr/DslrContext.cpp
        dslr/DslrContext.h)

set(GUI_SOURCES
        gui/AppWindow.cpp
        gui/AppWindow.h gui/FrameWidget.cpp gui/FrameWidget.h)

set(CAMERA_SOURCES
        camera/CameraInterface.h
        camera/OpenCvCamera.cpp
        camera/OpenCvCamera.h)

set(UTIL_SOURCES
        util/TimeUtil.cpp
        util/TimeUtil.h)

set(PLATFORM_SOURCES)
set(PLATFORM_LIBS)

if (RPI)
    add_definitions(-DRPI_BOARD)
    find_package(Raspberry REQUIRED)

    list(APPEND PLATFORM_LIBS ${RASPBERRY_LIBRARIES})

    include_directories(${RASPBERRY_INCLUDE_DIR})

    list(APPEND CAMERA_SOURCES
            camera/RaspberryCamera.cpp
            camera/RaspberryCamera.h)
endif()

set(PHOTOBOOTH_SOURCES
        main.cpp
        logging.h
        ${PLATFORM_SOURCES}
        ${DSLR_SOURCES}
        ${GUI_SOURCES}
        ${CAMERA_SOURCES}
        ${UTIL_SOURCES} CameraConsumerThread.cpp CameraConsumerThread.h)

include_directories(${GPHOTO2_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})

add_executable(photobooth ${PHOTOBOOTH_SOURCES})

target_link_libraries(photobooth
        ${OpenCV_LIBS}
        ${PLATFORM_LIBS}
        ${Boost_LIBRARIES}
        ${Gphoto2_LIBRARIES}
        Threads::Threads
        Qt5::Core
        Qt5::OpenGL
        Qt5::Widgets)