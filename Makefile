CC=g++
CFLAGS=-Wall -Wextra -pedantic -std=c++20 `pkg-config --cflags sdl2 glew opengl SDL2_ttf` -ggdb
LIBS=`pkg-config --libs sdl2 glew opengl SDL2_ttf`
SRC=src/*.cpp
<<<<<<< HEAD
gnout: $(SRC)
=======
default:
>>>>>>> refs/remotes/origin/main
	$(CC) -o gnout $(SRC) $(CFLAGS) $(LIBS)
