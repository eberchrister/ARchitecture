cmake_minimum_required(VERSION 3.4)

project(Test)
set(IncludePath "/usr/include")
set(Test_SOURCES Testing/marker_detection.cpp Testing/DrawPrimitives)


# GLEW
message(STATUS "Locating GLEW...")
find_package(GLEW REQUIRED)

# OpenGL
message(STATUS "Locating OpenGL...")
find_package(OpenGL REQUIRED)

# OpenCV
message(STATUS "Locating OpenCV...")
find_package(OpenCV REQUIRED)

# glut
message(STATUS "Locating glut...")
find_package(GLUT REQUIRED)


if (GLEW_FOUND AND OPENGL_FOUND AND OpenCV_FOUND)
message(STATUS "All required packages found!")

include_directories( ${GLEW_INCLUDE_DIRS}  IncludePath)
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${OPENGL_INCLUDE_DIRS} )
include_directories( ${GLUT_INCLUDE_DIRS} )


add_executable(Test ${Test_SOURCES})

target_link_libraries (Test GLEW::GLEW "/opt/homebrew/cellar/glfw/3.3.8/lib/libglfw.3.3.dylib" ${OPENGL_LIBRARIES} ${OpenCV_LIBS} ${GLUT_LIBRARY}) 
endif()