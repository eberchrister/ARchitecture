cmake_minimum_required(VERSION 3.4)

project(ARchitecture)
set(IncludePath "/usr/include")
set(ARchitecture_SOURCES src/MarkerDetection.cpp src/MarkerDetection.h src/main.cpp src/ObjectRender.cpp src/ObjectRender.h)


# GLEW
message(STATUS "Locating GLEW...")
find_package(GLEW REQUIRED)

# OpenGL
message(STATUS "Locating OpenGL...")
find_package(OpenGL REQUIRED)

# OpenCV
message(STATUS "Locating OpenCV...")
find_package(OpenCV REQUIRED)

if (GLEW_FOUND AND OPENGL_FOUND AND OpenCV_FOUND)
message(STATUS "All required packages found!")

include_directories( ${GLEW_INCLUDE_DIRS}  IncludePath)
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${OPENGL_INCLUDE_DIRS} )


add_executable(ARchitecture ${ARchitecture_SOURCES})

target_link_libraries (ARchitecture ${GLEW_LIBRARIES} "/usr/lib/x86_64-linux-gnu/libglfw.so" ${OPENGL_LIBRARIES} ${OpenCV_LIBS}) 
endif()