
# ARchitecture

## Members
- Eber Christer
- Jessica Sumargo
- Piotr Nobis


## Description

ARchitecture is an application that aids in room planning and visualization. The user is provided with a set of markers, each representing different common objects that one would find in a room (walls, tables, bed, chairs, TV, and other furnitures). By placing these markers in front of the camera, a 3D visualization of the aforementioned objects are rendered into the scene.


## Running ARchitecture
1. Clone this repository
2. Open `ARchitecture/src/main.cpp`and change the `VIDEOPATH` and `MARKERPATH` macro in **line 13-14** to adapt to the user's system<sup>1</sup>.
3. Change current directory to ARchitecture `cd <yourpath>/ARchitecture`
4. Build the program using CMake<sup>2</sup> `cmake .`
5. Compile the program using makefile<sup>3</sup> `make`
6. Run generated executable to start the program<sup>4</sup> `./output`

Default: Immediately after the program runs, it will automatically start the webcam built into the device. Otherwise, it will read the contents of `resources/MarkerMovie.MP4`, and display the AR functionality on the video instead,

<font size="2"> <sup>1</sup> Can be relative or absolute path. 

<font size="2"> <sup>2</sup> If the provided CMakeLists.txt (made for linux) doesn't work in the user's system, then some modifications must be made depending on the operating system. The include path might have to be changed. 

<font size="2"> <sup>3</sup> Aside from building using CMake, a manual makefile has been provided> This allows the user to compile the source codes manually without using CMake. However, the `glfw` dynamic library (`.so`) path must be changed to fit the operating system. Note: this file will be overriden if CMake is used.

<font size="2"> <sup>4</sup> If somehow there is an error concerning the video encoding, the user can remove the `cv::CAP_FFMPEG` in line 32 and 37 in `ARchitecture/src/main.cpp`.


## Files
This is an overview of this repository's file structure:
``` txt
ARchitecture
├── src
│   ├── main.cpp
│   ├── MarkerDetection.(cpp|h)
│   ├── ObjectRender.(cpp|h)
├── resources
│   └── markers
│       ├── marker<x>.png
│   └── MarkerMovie.MP4	
│   └── markers_all.png	
├── CMakeLists.txt
├── makefile
```
`MarkerDetection.(cpp|h)` contains a class and helper classes that essentially takes care of the necessary OpenCV implementation, which includes marker detection, marker identification, and pose estimation. 

`ObjectRender.(cpp|h)` contains a class that takes care of visualization and object creation with OpenGL. This includes helper functions to convert OpenCV coordinates into OpenGL coordinates, vector algebra, as well as furniture object creation.

`main.cpp` implements all the modules mentioned above.

`resources` stores all the necessary resources for the program to function. `resources/markers` contains multiple unique arUco markers that will be used to create a marker dictionary for the marker detection and object creation. The video files are also stored here.


## Frameworks
- [OpenGL](https://www.genome.gov/) : Object creation & 3D rendering.
- [OpenCV](https://opencv.org/) : Marker detection & Pose estimation





## CMakeLists.txt
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

if (GLEW_FOUND AND OPENGL_FOUND AND OpenCV_FOUND)
message(STATUS "All required packages found!")

include_directories( ${GLEW_INCLUDE_DIRS}  IncludePath)
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${OPENGL_INCLUDE_DIRS} )

add_executable(output ${ARchitecture_SOURCES})

# select one of these based on the user's operating system
# for Linux
target_link_libraries (output ${GLEW_LIBRARIES} "/usr/lib/x86_64-linux-gnu/libglfw.so" ${OPENGL_LIBRARIES} ${OpenCV_LIBS}) 

# for MacOS
target_link_libraries (output GLEW::GLEW "/opt/homebrew/cellar/glfw/3.3.8/lib/libglfw.3.3.dylib" ${OPENGL_LIBRARIES} ${OpenCV_LIBS}) 

endif()
```
