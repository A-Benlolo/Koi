#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *buffer = malloc(sizeof(char) * 4);
    char *msg = "This is a buffer overflow!";
    strcpy(buffer, msg);
    printf("Buffer content: %s\n", buffer);
    free(buffer);
    return 0;
}
