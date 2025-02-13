#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vulnerableFunction(char *input) {
    char *buffer = malloc(sizeof(char) * 4);
    strcpy(buffer, input);
}

int main() {
    char *userInput = malloc(sizeof(char) * 32);
    printf("Enter a string: ");
    fgets(userInput, 32, stdin);
    userInput[strcspn(userInput, "\n")] = 0;
    vulnerableFunction(userInput);
    printf("Main function ends.\n");
    return 0;
}
