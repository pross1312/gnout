CC=g++
CFLAGS=-Wall -Wextra -pedantic -std=c++20 `pkgconf --cflags sdl2 glew opengl SDL2_ttf` -ggdb
LIBS=`pkgconf --libs sdl2 glew opengl SDL2_ttf`
SRC=src/main.cpp src/Editor.cpp src/Vec.cpp src/EditorRenderer.cpp src/Utils.cpp
default:
	$(CC) -o gnout $(SRC) $(CFLAGS) $(LIBS)
