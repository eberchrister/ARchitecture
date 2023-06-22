# ARchitecture
Erweiterte Realität (IN2018) Project

# Current Directory Structure
``` txt
ARchitecture
├── resources
│   └── markers
│       ├── marker1.png
│       ├── ...
│       └── marker6.png
│   └── MarkerMovie.MP4
├── src
│   ├── main.cpp
│   ├── MarkerDetection.cpp
│   └── MarkerDetection.h
├── test
│   └── this is the old file
├── CMakeLists.txt
├── makefile
└── output (executable)
```

# CMakeLists.txt
``` CMake
cmake_minimum_required(VERSION 3.4)

project(ARchitecture)
set(IncludePath "/usr/include")
set(ARchitecture_SOURCES src/MarkerDetection.cpp src/MarkerDetection.h src/main.cpp)

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

add_executable(output ${ARchitecture_SOURCES})

# for Linux
target_link_libraries (output ${GLEW_LIBRARIES} "/usr/lib/x86_64-linux-gnu/libglfw.so" ${OPENGL_LIBRARIES} ${OpenCV_LIBS} ${GLUT_LIBRARY}) 

# for MacOS
target_link_libraries (Test GLEW::GLEW "/opt/homebrew/cellar/glfw/3.3.8/lib/libglfw.3.3.dylib" ${OPENGL_LIBRARIES} ${OpenCV_LIBS} ${GLUT_LIBRARY}) 

endif()
```

# CMake
``` MakeFile
CC = g++
PROJECT = output
SRC = src/MarkerDetection.cpp src/MarkerDetection.h src/main.cpp
INCLUDE_PATH = /usr/include

# GLEW
GLEW_INCLUDE_DIRS = $(shell pkg-config --cflags glew)
GLEW_LIBRARIES = $(shell pkg-config --libs glew)

# OpenGL
OPENGL_INCLUDE_DIRS = $(shell pkg-config --cflags opengl)
OPENGL_LIBRARIES = $(shell pkg-config --libs opengl)

# OpenCV
OPENCV_INCLUDE_DIRS = $(shell pkg-config --cflags opencv4)
OPENCV_LIBRARIES = $(shell pkg-config --libs opencv4)

# GLUT
GLUT_LIBRARIES = -lglut

$(PROJECT): $(SRC)
	$(CC) $(SRC) -o $(PROJECT) -I$(INCLUDE_PATH) $(GLEW_INCLUDE_DIRS) $(OPENCV_INCLUDE_DIRS) $(OPENGL_INCLUDE_DIRS) \
	$(GLEW_LIBRARIES) $(OPENGL_LIBRARIES) $(OPENCV_LIBRARIES) $(GLUT_LIBRARIES)

clean:
	-rm -f $(PROJECT)
```
