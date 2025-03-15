
#!/bin/bash
CFLAGS="-Wall -Wextra -Werror -g"

gcc $CFLAGS -c hashmap.c
gcc $CFLAGS -c utils.c
gcc $CFLAGS -c my_printf.c
gcc $CFLAGS -c print_error.c
gcc $CFLAGS -c file_header_fns.c

gcc $CFLAGS -o hashmap hashmap.o utils.o my_printf.o print_error.o file_header_fns.o


echo "hashmap.c compiled in local directory"
echo ""