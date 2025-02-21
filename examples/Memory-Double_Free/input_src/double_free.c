#include <stdlib.h>
int main(int argc, char *argv[]) {
    void *buf = malloc(4);
    free(buf);
    free(buf);
    return 0;
}