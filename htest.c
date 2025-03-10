#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>


int hash_fn(char * name)
{
    int sum = 0;

    for(int i = 0; i < strlen(name); i++)
    {
        sum += (int)name[i];        
    }
        return sum % 10;
}

int main()
{
    char * name = malloc(13 * sizeof(char));
    if(name == NULL)
    {
        printf("Error\n");
        return -1;
    }

    strcpy(name, "! ");
    
    int num = hash_fn(name);
    printf("Hash is: %d", num);

    free(name);
}