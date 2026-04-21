# Used claude to help integrate submodule
# Link: https://claude.ai/share/8f671ca7-04a3-46ca-b01c-d55b669081a5

EXEC    = build/piano_tiles
CC      = $(CROSS_COMPILE)gcc

# Submodule
SUBMOD_DIR = ecen5713_rpi_ws281x
SUBMOD_SRCS = $(SUBMOD_DIR)/ws2812b_wrapper.c \
              $(SUBMOD_DIR)/ws2811.c \
              $(SUBMOD_DIR)/dma.c \
              $(SUBMOD_DIR)/pwm.c \
              $(SUBMOD_DIR)/pcm.c \
              $(SUBMOD_DIR)/mailbox.c \
              $(SUBMOD_DIR)/rpihw.c
SUBMOD_OBJ  = $(SUBMOD_SRCS:$(SUBMOD_DIR)/%.c=build/submod_%.o)
SUBMOD_DEPS = $(SUBMOD_OBJ:.o=.d)

# Flags
CFLAGS  = -Wall -Werror -Isrc -Iinc -I$(SUBMOD_DIR) -pthread -lm
LDFLAGS = -pthread -lm

# Auto-discover all .c files
SRCS = $(wildcard src/*.c)
OBJ  = $(SRCS:src/%.c=build/%.o)
DEPS = $(OBJ:.o=.d)

# Target
all: build $(EXEC)

# Create build directory
build:
	mkdir -p build

# Link
$(EXEC): $(OBJ) $(SUBMOD_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile (with dependency file generation)
build/%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS) -MMD -MP

# Compile submodule sources (prefixed with submod_ to avoid name collisions)
build/submod_%.o: $(SUBMOD_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS) -MMD -MP

# Pull in generated dependency files
-include $(DEPS) $(SUBMOD_DEPS)

# Clean
.PHONY: clean
clean:
	@rm -rf build

# Help
.PHONY: help
help:
	@echo "\
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