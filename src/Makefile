TARGET = main
EXENAME = arkanoid
DEPS = Game.o Window.o Common.o Player.o Shader.o Triangle.o Ball.o Board.o

CC = g++
CFLAGS = -std=c++17 -Wall -Wextra
OBJS = $(TARGET).o
LIBS = -lGLEW -lGL -lglfw

arkanoid2: main.o Shader.o
	g++ -o arkanoid2 $^ $(LIBS)

all: $(TARGET)

install: $(TARGET) clean

$(TARGET): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(EXENAME) $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
distclean: clean
	rm -f $(EXENAME)

.PHONY: clean
