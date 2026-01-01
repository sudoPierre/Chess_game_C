CC := cc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs)
TARGET := chess_game
SRCS := \
	src/main.c \
	src/ui.c \
	src/game_logic.c \
	src/ai.c \
	src/chat.c \
	src/file_io.c \
	src/bitmap_font.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
