#include <stdio.h>
#include <string.h>

void vulnerableFunction(char *input) {
    char buffer[16];
    strcpy(buffer, input);
}

int main() {
    char userInput[32];
    printf("Enter a string: ");
    fgets(userInput, sizeof(userInput), stdin);
    userInput[strcspn(userInput, "\n")] = 0;
    vulnerableFunction(userInput);
    printf("Main function ends.\n");
    return 0;
}
