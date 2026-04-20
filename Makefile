# Developed from PES assignment 1 makefile and googled how to make the echo statement in help phony
EXEC	= build/piano_tiles
CC 		= $(CROSS_COMPILE)gcc

# Flags
CFLAGS 	= -Wall -Werror -Isrc -Iinc -pthread
LDFLAGS = -pthread

# Auto-discover all .c and .h files
SRCS = $(wildcard src/*.c)
OBJ = $(SRCS:src/%.c=build/%.o)

# Target
all: $(EXEC)

# Link
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile
build/%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

# Clean
.PHONY: clean
clean:
	@rm -rf build/*

# Help
.PHONY: help
help:
	@@echo "\
Usage: make [target] [CROSS_COMPILE=<prefix>]\n\
\n\
Available targets: \n\
    all (default) - Build the piano_tiles application\n\
    clean         - Remove the build directory\n\
    help          - Display this help message\n\
\n\
Cross-compilation:\n\
    make                                          # Native build\n\
    make CROSS_COMPILE=aarch64-none-linux-gnu-    # ARM64 cross-compile"