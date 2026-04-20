CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g
TARGET  = server
SRCS    = server.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
