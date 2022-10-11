CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -DARKANOID_SLOW
LIBS = -lGLEW -lGL -lglfw -lGLU

arkanoid: arkanoid.cpp Makefile
	$(CC) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm arkanoid

.PHONY: clean
