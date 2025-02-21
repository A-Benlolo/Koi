#include <stdio.h>
#include <string.h>

int main() {
    char buffer[10];
    char *msg = "This is a buffer overflow that leads to stack smashing!";
    strcpy(buffer, msg);
    printf("Buffer content: %s\n", buffer);
    return 0;
}
