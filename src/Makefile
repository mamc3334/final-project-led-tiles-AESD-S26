# Developed from PES assignment 1 makefile and googled how to make the echo statement in help phony
EXEC	= input_handler
CC 		= $(CROSS_COMPILE)gcc

# Flags
CFLAGS 	= -Wall -Werror
LDFLAGS  = 

SRC      = $(wildcard *.c)
OBJ      = $(SRC:.c=.o)

# Target
all: $(EXEC)

# Build
${EXEC}: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile
%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)

# Clean
.PHONY: clean
clean:
	@rm -rf $(EXEC) *.o

# Help
.PHONY: help
help:
	@@echo "\
Usage: make [target] [CROSS_COMPILE=<prefix>]\n\
\n\
Available targets: \n\
    all (default) - Build the input_handler application\n\
    clean         - Remove input_handler and all .o files\n\
    help          - Display this help message\n\
\n\
Cross-compilation:\n\
    make                                          # Native build\n\
    make CROSS_COMPILE=aarch64-none-linux-gnu-    # ARM64 cross-compile"