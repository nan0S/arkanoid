CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0
LIBS = -lGLEW -lGL -lglfw -lGLU

arkanoid: arkanoid.cpp
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm arkanoid

.PHONY: clean
