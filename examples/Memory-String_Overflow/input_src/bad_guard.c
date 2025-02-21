#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *safeCopy(char *input) {
    int length = strlen(input);
    char *buffer = malloc(length * sizeof(char));
    strcpy(buffer, input);
    return input;
}

int main() {
    char userInput[64];

    printf("Enter a string: ");
    fgets(userInput, sizeof(userInput), stdin);

    userInput[strcspn(userInput, "\n")] = 0;
    safeCopy(userInput);

    printf("Program has ended.\n");
    return 0;
}
