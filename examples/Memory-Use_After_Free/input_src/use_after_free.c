/**********************************************/
/* THE FOLLOWING CODE WAS PROVIDED BY CHATGPT */
/**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 50
#define BUFFER_SIZE 100

// Print the main menu to the user
void print_menu() {
    printf("\nFile Management System\n");
    printf("1. Create a new file\n");
    printf("2. Read a file\n");
    printf("3. Append to a file\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}


// Get user input for the filename
void get_filename(char *filename) {
    printf("Enter the filename: ");
    fgets(filename, MAX_FILENAME_LENGTH, stdin);
    filename[strcspn(filename, "\n")] = '\0';  // Remove the newline character
}


// Create a new file
void create_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }
    printf("File %s created successfully.\n", filename);
    fclose(file);
}


// Read a file
void read_file(const char *filename) {
    char buffer[BUFFER_SIZE];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    printf("Contents of %s:\n", filename);
    while (fgets(buffer, BUFFER_SIZE, file)) {
        printf("%s", buffer);
    }
    fclose(file);
}


// Append to a file with a use-after-free vulnerability
void append_to_file(const char *filename) {
    // Dynamically allocate memory for a buffer
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (buffer == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    // Read input into memory
    printf("Enter text to append to the file (max 100 characters):\n");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    // Echo what will be appended
    printf("You entered: %s\n", buffer);
    free(buffer);

    // Open the file to append to
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    // Perform the appending
    fprintf(file, "%s\n", buffer);
    printf("Text appended to %s.\n", filename);
    fclose(file);
}


// Handle user input for menu selection
int get_menu_choice() {
    int choice;
    scanf("%d", &choice);
    getchar(); // To consume the newline character left by scanf
    return choice;
}


// Perform the selected file operation
void perform_file_operation(int choice) {
    char filename[MAX_FILENAME_LENGTH];
    switch (choice) {
        case 1:
            get_filename(filename);
            create_file(filename);
            break;
        case 2:
            get_filename(filename);
            read_file(filename);
            break;
        case 3:
            get_filename(filename);
            append_to_file(filename);
            break;
        case 4:
            printf("Exiting program.\n");
            break;
        default:
            printf("Invalid choice, please try again.\n");
    }
}


// Main loop to keep the program running
void file_management_system() {
    int choice;
    while (choice != 4) {
        print_menu();
        choice = get_menu_choice();
        perform_file_operation(choice);
    }
}


int main() {
    file_management_system();
    return 0;
}
