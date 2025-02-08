#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

char my_tolower(char c);
int my_strlen(char *string);
void *my_memset(void *str, int c, size_t n);
char *my_strlwr(char *string);
char *my_strncpy(char *dst, const char *src, size_t n);
int my_strcmp(const char *s1, const char *s2);
int lc_strcmp(char *array1, char *array2);
int ls_my_strcmp(const char *s1, const char *s2);

#endif
