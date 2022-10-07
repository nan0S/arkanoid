CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lGLEW -lGL -lglfw -lGLU

arkanoid: arkanoid.o shader.o error.o
	$(CC) $(CXXFLAGS) -o arkanoid $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
distclean: clean
	rm -f arkanoid

.PHONY: clean
