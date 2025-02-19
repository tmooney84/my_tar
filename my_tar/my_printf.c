/*
Each conversion specification is introduced by the PERCENT character.
The arguments must correspond properly (after type promotion) with the
conversion specifier. After the PERCENT, the following appear in sequence:

doux The int (or appropriate variant) argument is converted to signed decimal
(d). unsigned octal (o), unsigned decimal (u), unsigned hexadecimal (x). c The
int argument is converted to an unsigned char, and the resulting character is
written. s The char * argument is expected to be a pointer to an array of
character type (pointer to a string). Characters from the array are written up
to (but not including) a terminating NUL character. p The void * pointer
argument is printed in hexadecimal.

Authorized function(s)
• write(2)
• malloc
• free
• va_start, va_arg, va_copy, va_end (see man 3 stdarg)
*/

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int my_printf(char *fmt, ...)
{
    va_list ap;
    void *pval;
    char *sval;
    char *p;
    char c;
    char cval;
    int ival;
    unsigned int oval, xval;
    int counter = 0;  // counter of number of char for return
    unsigned int uival;

    va_start(ap, fmt);

    for (p = fmt; *p; p++)
    {
        if (*p != '%')
        {
            int n;
            c = *p;
            n = write(1, &c, sizeof(char));
            if (n <= 0)
            {
                va_end(ap);
                return -1;
            }
            counter++;
            continue;
        }

        switch (*++p)
        {
            // signed int
            case 'd':
                ival = va_arg(ap, int);

                int is_neg = 0;

                char buff[20];  // buffer for chars from digits (reverse order)

                int i = 0;  // counter to know how many digits in number

                if (ival < 0)
                {
                    is_neg = 1;
                    ival = -ival;
                }

                do
                {
                    buff[i++] = '0' + (ival % 10);
                    ival = ival / 10;
                } while (ival > 0);

                if (is_neg)
                {
                    buff[i++] = '-';
                }

                // swap with temp to reorder
                for (int j = 0; j < i / 2; j++)
                {
                    char temp = buff[j];
                    buff[j] = buff[i - j - 1];
                    buff[i - j - 1] = temp;
                }
                // buff[i] = '\0';

                int wc1 = write(1, buff, i * sizeof(char));

                if (wc1 <= 0)
                {
                    va_end(ap);
                    return -1;
                }

                counter += i;

                break;

                // unsigned int

            case 'u':
                uival = va_arg(ap, unsigned int);

                char buff_un[20];  // buffer for chars from digits (reverse
                                   // order)

                int k = 0;  // counter to know how many digits in number

                do
                {
                    buff_un[k++] = '0' + (uival % 10);
                    uival = uival / 10;
                } while (uival > 0);

                // swap with temp to reorder
                for (int j = 0; j < k / 2; j++)
                {
                    char temp = buff_un[j];
                    buff_un[j] = buff_un[k - j - 1];
                    buff_un[k - j - 1] = temp;
                }

                int wc2 = write(1, buff_un, k * sizeof(char));

                if (wc2 <= 0)
                {
                    va_end(ap);
                    return -1;
                }

                counter += k;
                break;

            // char
            case 'c':
                cval = va_arg(ap, int);
                c = (char)
                    cval;  // need to cast because va_arg will come back as int
                int wc3 = write(1, &cval, sizeof(char));

                if (wc3 <= 0)
                {
                    va_end(ap);
                    return -1;
                }

                counter += 1;
                break;

            // octal
            case 'o':
            {
                oval = va_arg(ap, unsigned int);
                char buffer[32];
                int index = 0;

                // Convert to octal representation
                do
                {
                    buffer[index++] = '0' + (oval % 8);
                    oval /= 8;
                } while (oval > 0);

                // leading zero in oct notation
                // buff_oct[a++] = '0';

                // Reverse the string for correct order
                for (int i = index - 1; i >= 0; i--)
                {
                    if (write(1, &buffer[i], sizeof(char)) <= 0)
                    {
                        va_end(ap);
                        return -1;
                    }
                    counter++;
                }
                break;
            }

            // hexadecimal
            case 'x':
                xval = va_arg(ap, unsigned int);

                char buff_hex[20];  // buffer for chars from digits (reverse
                                    // order)

                int b = 0;  // counter to know how many digits in number

                do
                {
                    // hex pre-process to base of 16 and add '0x' in front

                    {
                        buff_hex[b] = '0' + (xval % 16);
                    }

                    if (buff_hex[b] > '9')
                    {
                        buff_hex[b] = 'A' + (xval % 16) - 10;
                    }

                    xval = xval / 16;
                    b++;
                } while (xval > 0);

                // leading "0x" in hex notation
                // buff_hex[b++] = 'x';
                // buff_hex[b++] = '0';

                // swap with temp to reorder
                for (int j = 0; j < b / 2; j++)
                {
                    char temp = buff_hex[j];
                    buff_hex[j] = buff_hex[b - j - 1];
                    buff_hex[b - j - 1] = temp;
                }

                int wc5 = write(1, buff_hex, b * sizeof(char));

                if (wc5 <= 0)
                {
                    va_end(ap);
                    return -1;
                }

                counter += b;
                break;

            // pointer address in hex
            case 'p':
                pval = va_arg(ap, void *);
                uintptr_t addr =
                    (uintptr_t)pval;  // safer int type typically used for
                                      // pointer arithmetic

                char buff_point[20];  // buffer for chars from digits (reverse
                                      // order)

                int f = 0;  // counter to know how many digits in number

                if (addr == 0)
                {
                    write(1, "(null)", 6);
                    counter +=6;
                    break;
                }

                do
                {
                    // hex pre-process to base of 16 and add '0x' in front
                    {
                        buff_point[f] = '0' + (addr % 16);
                    }

                    if (buff_point[f] > '9')
                    {
                        buff_point[f] = 'a' + (addr % 16) - 10;
                    }

                    addr = addr / 16;

                    f++;
                } while (addr > 0);

                // leading zero in hex notation
                buff_point[f++] = 'x';
                buff_point[f++] = '0';

                // swap with temp to reorder
                for (int j = 0; j < f / 2; j++)
                {
                    char temp = buff_point[j];
                    buff_point[j] = buff_point[f - j - 1];
                    buff_point[f - j - 1] = temp;
                }

                int wc6 = write(1, buff_point, f * sizeof(char));

                if (wc6 <= 0)
                {
                    va_end(ap);
                    return -1;
                }

                counter += f;
                break;

                // char string
            case 's':
                sval = va_arg(ap, char *);
                if (sval == NULL)
                {
                    sval = "(null)";
                }

                while (*sval)
                {
                    int wc7 = write(1, sval, sizeof(char));
                    if (wc7 <= 0)
                    {
                        va_end(ap);
                        return -1;
                    }
                    sval++;
                    counter++;
                }
                break;

            default:
                c = *p;
                int def_wc = write(1, &c, sizeof(char));
                if (def_wc <= 0)
                {
                    va_end(ap);
                    return -1;
                }
        }
    }
    va_end(ap);

    return counter;
}

// int main()
// {
//     // char letter = '@';
//     // int number = -2345;
//     // unsigned int un_num = 123456;

//     // int n = 50;
//     // void *p = &n;
//     // char test_string[20] = "HEY YOU THERRE!";
//     // my_printf("test string: %s\n", test_string);

// int ret_val = my_printf("Hello %s!\n", "QWASAR.IO");
// printf("%d", ret_val);

//     return 0;
// }
