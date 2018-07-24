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
find_package(Gphoto2 REQUIRED)
find_package(X11 REQUIRED)

if (RPI)
    add_definitions(-DRPI_BOARD)
    find_package(Raspberry REQUIRED)
    set(BOARD_SOURCES RaspberryCamera.cpp RaspberryCamera.h)
    set(BOARD_LIBS ${RASPBERRY_LIBRARIES})
    include_directories(${RASPBERRY_INCLUDE_DIR})
else()
    find_package(OpenGL COMPONENTS EGL REQUIRED)
    find_package(OpenGLES2 REQUIRED)
    include_directories(${OpengGL_EGL_INCLUDE_DIR})
    include_directories(${OPENGLES2_INCLUDE_DIR})
    set(BOARD_SOURCES)

    set(BOARD_LIBS OpenGL::EGL OpenGLES2::OpenGLES2)
endif()

include_directories(${X11_INCLUDE_DIR})

set(DSLR_SOURCES
        dslr/DslrCamera.cpp
        dslr/DslrCamera.h
        dslr/DslrCameraInfo.cpp
        dslr/DslrCameraInfo.h
        dslr/DslrContext.cpp
        dslr/DslrContext.h)

set(PHOTOBOOTH_SOURCES
        main.cpp
        CameraInterface.h
        OpenCvCamera.cpp
        OpenCvCamera.h
        XWindowOutputContext.cpp
        XWindowOutputContext.h
        logging.h
        ${BOARD_SOURCES}
        ${DSLR_SOURCES})

include_directories(${GPHOTO2_INCLUDE_DIR})

add_executable(photobooth ${PHOTOBOOTH_SOURCES})

target_link_libraries(photobooth
        ${OpenCV_LIBS}
        ${BOARD_LIBS}
        ${Boost_LIBRARIES}
        ${Gphoto2_LIBRARIES}
        ${X11_LIBRARIES}
        -lpthread )