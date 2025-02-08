#include <stdio.h>
#include "utils.h"

char my_tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        c = (c - 'A') + 'a';
    }
    return c;
}

int my_strlen(char *string)
{
    if (string == NULL)
    {
        return -1;
    }

    int i;
    for (i = 0; string[i] != '\0'; i++)
    {
        // loop to find the length
    }
    return i;
}

// my_memset works on a byte by byte basis; thus need to cast unsigned char
// (0-255); size_t in for loop to guarentee large size
void *my_memset(void *str, int c, size_t n)
{
    unsigned char *ptr = (unsigned char *)str;
    // loops over each byte
    for (size_t i = 0; i < n; i++)
    {
        /*
        -typecasting because the compiler only
        -may truncate c or leads to warnings of mismatched types
        -typecasting to unsigned char will guarentee that only the
        -least significant byte is used in relevent bitwise operations
        -allows portability across platforms
        */
        ptr[i] = (unsigned char)c;
    }

    return str;
}

char *my_strlwr(char *string)
{
    int n = my_strlen(string);

    for (int i = 0; i < n; i++)
    {
        string[i] = my_tolower(string[i]);
    }

    return string;
}

char *my_strncpy(char *dst, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dst[i] = src[i];
    }

    for (; i < n; i++)
    {
        dst[i] = '\0';
    }
    return dst;
}

int my_strcmp(const char *s1, const char *s2)
{
    for (int i = 0; s1[i] != '\0' || s2[i] != '\0'; i++)
    {
        if (s1[i] < s2[i])
        {
            return -1;
        }
        else if (s1[i] > s2[i])
        {
            return 1;
        }
    }

    return 0;
}

// // custom strcmp of ls

int ls_my_strcmp(const char *s1, const char *s2)
{
    // handle "." and ".." explicitly
    if (my_strcmp(s1, ".") == 0)
    {
        if (my_strcmp(s2, ".") == 0) return 0;
        return -1;  // '.' comes before everything
    }
    if (my_strcmp(s2, ".") == 0) return 1;

    if (my_strcmp(s1, "..") == 0 && my_strcmp(s2, ".") != 0)
    {
        if (my_strcmp(s2, "..") == 0) return 0;
        return -1;  // '..' comes before everything except "."
    }
    if (my_strcmp(s2, "..") == 0 && my_strcmp(s1, ".") != 0)
    {
        if (my_strcmp(s1, "..") == 0) return 0;
        return 1;
    }

    // count leading dots
    int dots1 = 0, dots2 = 0;
    while (s1[dots1] == '.') dots1++;
    while (s2[dots2] == '.') dots2++;

    // compare dot-prefixed files by dot count first
    if (dots1 != dots2) return dots2 - dots1;

    // compare remaining characters
    int i = 0;
    while (s1[dots1 + i] != '\0' && s2[dots2 + i] != '\0')
    {
        char c1 = s1[dots1 + i];
        char c2 = s2[dots2 + i];

        // numeric comparisons
        if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9')
        {
            // parse numbers
            long num1 = 0, num2 = 0;
            int j = dots1 + i;
            while (s1[j] >= '0' && s1[j] <= '9')
            {
                num1 = num1 * 10 + (s1[j] - '0');
                j++;
            }
            int k = dots2 + i;
            while (s2[k] >= '0' && s2[k] <= '9')
            {
                num2 = num2 * 10 + (s2[k] - '0');
                k++;
            }

            if (num1 != num2)
            {
                return (num1 < num2) ? -1 : 1;
            }

            // skip compared digits
            i += (j - (dots1 + i)) - 1;
        }
        // special rules
        else if (c1 == '.' && c2 >= '0' && c2 <= '9')
            return 1;
        else if (c2 == '.' && c1 >= '0' && c1 <= '9')
            return -1;
        else if (c1 == '_' &&
                 ((c2 >= 'a' && c2 <= 'z') || (c2 >= 'A' && c2 <= 'Z')))
            return 1;
        else if (c2 == '_' &&
                 ((c1 >= 'a' && c1 <= 'z') || (c1 >= 'A' && c1 <= 'Z')))
            return -1;

        else if (c1 != c2)
        {
            return (c1 < c2) ? -1 : 1;
        }

        i++;
    }

    if (s1[dots1 + i] == '\0' && s2[dots2 + i] != '\0') return -1;
    if (s2[dots2 + i] == '\0' && s1[dots1 + i] != '\0') return 1;

    return 0;
}

// ONLY USED IN THE UBUNTU VERSION OF MY_LS
int lc_strcmp(char *array1, char *array2)
{
    int n1 = my_strlen(array1);
    char temp1[n1 + 1];
    my_strncpy(temp1, array1, n1);
    temp1[n1] = '\0';
    my_strlwr(temp1);

    int n2 = my_strlen(array2);
    char temp2[n2 + 1];
    my_strncpy(temp2, array2, n2);
    temp2[n2] = '\0';
    my_strlwr(temp2);

    // special version of strcmp for my_ls
    return ls_my_strcmp(temp1, temp2);
}