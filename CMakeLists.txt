cmake_minimum_required(VERSION 3.10)
project(photobooth)

set(CMAKE_CXX_STANDARD 17)

option(RPI "Build for Raspberry Pi" OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(Boost_USE_STATIC_LIBS ON)

# Find the QtWidgets library
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS log REQUIRED )

if (RPI)
    add_definitions(-DRPI_BOARD)
    set(BOARD_SOURCES RaspberryCamera.cpp RaspberryCamera.h)
    set(BOARD_LIBS "-lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos")
else()
    set(BOARD_SOURCES)
    set(BOARD_LIBS)
endif()

set(PHOTOBOOTH_SOURCES
        main.cpp
        CameraInterface.h
        OpenCvCamera.cpp
        OpenCvCamera.h
        logging.h
        ${BOARD_SOURCES})



add_executable(photobooth ${PHOTOBOOTH_SOURCES})

target_link_libraries(photobooth ${OpenCV_LIBS} ${BOARD_LIBS} ${Boost_LIBRARIES} -lpthread)