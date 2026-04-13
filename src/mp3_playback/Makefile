# Generated from Claude AI: https://claude.ai/chat/e05980ff-3152-4e54-8500-b6134312b416
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lmpg123 -lout123 -lpthread

TARGET  = mp3_player_demo
SRCS    = mp3_player.c main.c
OBJS    = $(SRCS:.c=.o)
DEPS    = $(OBJS:.o=.d)

# Buildroot cross-compile: make BR=1 CC=aarch64-linux-gnu-gcc SYSROOT=/path/to/staging
ifeq ($(BR), 1)
CFLAGS  += --sysroot=$(SYSROOT)
LDFLAGS += --sysroot=$(SYSROOT)
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)
