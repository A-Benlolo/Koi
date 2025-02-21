#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *ip = calloc(16, sizeof(char));
    strcpy(ip, "192.168.0.1.");
    printf("IPv4 Address = %s\n", ip);

    unsigned int packed = 0;
    for(int i = 0; i < 4; i++) {
        packed = (packed << 8) | atoi(ip);
        ip = strchr(ip, '.') + 1;
    }
    printf("Packed form = 0x%08x\n", packed);

    free(ip);
    return 0;
}
