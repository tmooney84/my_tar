# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -std=c99

# Target executable name
TARGET = my_tar

# Source files (add all your .c files here)
SRC = main.c utils.c print_error.c my_printf.c #my_ls.c utils.c string_quicksort.c time_quicksort.c

# Object files (derived automatically from SRC)
OBJ = $(SRC:.c=.o)

# Header files (optional but useful for dependency management)
HEADERS = utils.h print_error.h my_printf.h #utils.h string_quicksort.h time_quicksort.h

# Build target
all: $(TARGET)

# Linking the object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile each .c file into an object file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f *.o

fclean: clean
	rm -f $(TARGET)

re: fclean all

#########Previous Version
# # Makefile for compiling the program
# # Compiler
# CC = gcc

# # Compiler flags
# CFLAGS = -Wall -Wextra -Werror -std=c11

# # Target executable name
# TARGET = my_ls 

# # Source files
# SRC = my_ls.c

# # Build target
# all: $(TARGET)

# $(TARGET): $(SRC)
# 	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# # Clean up build files
# clean:
# 	rm -f *.o

# fclean: clean
# 	rm -f $(TARGET)

# re: fclean all