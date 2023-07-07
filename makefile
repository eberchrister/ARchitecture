CC = g++
PROJECT = output
SRC = src/MarkerDetection.cpp src/MarkerDetection.h src/main.cpp src/ObjectRender.cpp src/ObjectRender.h
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

$(PROJECT): $(SRC)
	$(CC) $(SRC) -o $(PROJECT) -I$(INCLUDE_PATH) $(GLEW_INCLUDE_DIRS) $(OPENCV_INCLUDE_DIRS) $(OPENGL_INCLUDE_DIRS) \
	$(GLEW_LIBRARIES) $(OPENGL_LIBRARIES) $(OPENCV_LIBRARIES) "/usr/lib/x86_64-linux-gnu/libglfw.so"

clean:
	-rm -f $(PROJECT)