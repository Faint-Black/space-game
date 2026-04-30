.PHONY: clean

GL_CFLAGS    := $(shell pkg-config --cflags opengl)
GLUT_CFLAGS  := $(shell pkg-config --cflags glut)
GLEW_CFLAGS  := $(shell pkg-config --cflags glew)
GL_LDFLAGS   := $(shell pkg-config --libs opengl)
GLUT_LDFLAGS := $(shell pkg-config --libs glut)
GLEW_LDFLAGS := $(shell pkg-config --libs glew)

CFLAGS       := -ansi -O2 -g -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Werror
CFLAGS       += $(GL_CFLAGS) $(GLUT_CFLAGS) $(GLEW_CFLAGS)
LDFLAGS      := -lm
LDFLAGS      += $(GL_LDFLAGS) $(GLUT_LDFLAGS) $(GLEW_LDFLAGS)

BINARY       := space-game
SOURCES      := $(wildcard src/*.c)
OBJECTS      := $(patsubst %.c,%.o,$(SOURCES))

$(BINARY): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(BINARY)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(BINARY) $(OBJECTS)
