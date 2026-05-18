.PHONY: all check clean run

BINARY       := space-game

GL_CFLAGS    := $(shell pkg-config --cflags opengl)
GL_LDFLAGS   := $(shell pkg-config --libs opengl)
SDL2_CFLAGS  := $(shell pkg-config --cflags sdl2)
SDL2_LDFLAGS := $(shell pkg-config --libs sdl2)
IMG_CFLAGS   := $(shell pkg-config --cflags SDL2_image)
IMG_LDFLAGS  := $(shell pkg-config --libs SDL2_image)
GLU_CFLAGS   := $(shell pkg-config --cflags glu)
GLU_LDFLAGS  := $(shell pkg-config --libs glu)

CFLAGS       += -ansi -O2 -g -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Werror
CFLAGS       += $(GL_CFLAGS) $(SDL2_CFLAGS) $(IMG_CFLAGS) $(GLU_CFLAGS)
LDFLAGS      += -lm
LDFLAGS      += $(GL_LDFLAGS) $(SDL2_LDFLAGS) $(IMG_LDFLAGS) $(GLU_LDFLAGS)

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
