# main.out: main.o Game.o Window.o Common.o Player.o Shader.o Triangle.o Ball.o Board.o

# %.o: %.cpp
	# g++ -c $< -std=c++17

# %.out: %.o
	# g++ $< -o $@ -std=c++17 -ggdb Game.o  Window.o Common.o Player.o Shader.o Triangle.o Ball.o Board.o -lGLEW -lGL -lglfw 

# clean:
	# rm *.o *.out *~

TARGET = main
DEPS = Game.o Window.o Common.o Player.o Shader.o Triangle.o Ball.o Board.o

CC = g++
CFLAGS = -std=c++17 -Wall -Wextra
OBJS = $(TARGET).o
LIBS = -lGLEW -lGL -lglfw
EXENAME = arkanoid

install: $(TARGET) clean

all: $(TARGET)

$(TARGET): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(EXENAME) $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
distclean: clean
	rm -f $(EXENAME)

.PHONY: clean
