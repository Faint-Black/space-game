.PHONY: all check clean run

GL_CFLAGS    := $(shell pkg-config --cflags opengl)
SDL3_CFLAGS  := $(shell pkg-config --cflags sdl3)
GLEW_CFLAGS  := $(shell pkg-config --cflags glew)
GL_LDFLAGS   := $(shell pkg-config --libs opengl)
SDL3_LDFLAGS := $(shell pkg-config --libs sdl3)
GLEW_LDFLAGS := $(shell pkg-config --libs glew)

CFLAGS       := -ansi -O2 -g -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Werror
CFLAGS       += $(GL_CFLAGS) $(SDL3_CFLAGS) $(GLEW_CFLAGS)
LDFLAGS      := -lm
LDFLAGS      += $(GL_LDFLAGS) $(SDL3_LDFLAGS) $(GLEW_LDFLAGS)

BINARY       := space-game
SOURCES      := $(wildcard src/*.c)
HEADERS      := $(wildcard src/*.h)
OBJECTS      := $(patsubst %.c,%.o,$(SOURCES))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BINARY) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) -c $^ -o $@ $(CFLAGS)

check:
	clang-tidy $(SOURCES) $(HEADERS)
	clang-format -i $(SOURCES) $(HEADERS)

run: all
	./$(BINARY)

clean:
	rm -f $(BINARY) $(OBJECTS)
