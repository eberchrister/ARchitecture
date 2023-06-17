CC = g++
PROJECT = Test
SRC = Testing/marker_detection.cpp
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