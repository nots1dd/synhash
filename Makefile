# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Libraries for linking
LIBS = -lncurses -lyaml

# Source files
SRCS = yaml-parser.c src/hashtable.c

# Header files
HEADERS = include/hashtable.h include/highlight.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = syntax_highlighter

# Default rule
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compile source files into object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
