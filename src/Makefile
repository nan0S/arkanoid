LIBS := -lGLEW -lGL -lglfw -lGLU
DEPS := arkanoid.cpp arkanoid.h shader.cpp shader.h math.h random.h Makefile

debug: $(DEPS)
	g++ -std=c++17 -Wall -Wextra -O0 -DARKANOID_SLOW -ggdb -fno-omit-frame-pointer -o arkanoid_debug arkanoid.cpp $(LIBS)

release: $(DEPS)
	g++ -std=c++17 -O3 -o arkanoid $< $(LIBS)

clean:
	rm -f arkanoid arkanoid_debug

.PHONY: clean
