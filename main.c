#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int LEVELS = 12;
const int CLASS = 10;

typedef struct Student {
    char firstName[50];
    char lastName[50];
    char last[50];
    char phoneNumber[11];
    int grades[10];
    struct Student* next;
} Student;

// Function to read students from file and build the school array
void readStudents(FILE* filePointer, Student* school[LEVELS][CLASS]) {
    Student student;
    int level, class;

    while (fscanf(filePointer, "%s %s %s %d %d", student.firstName, student.lastName, student.phoneNumber, &level, &class) == 5) {
        // Check if level and class are within valid bounds
        if (level >= 1 && level <= LEVELS && class >= 1 && class <= CLASS) {
            // Dynamically allocate memory for the student
            Student* newStudent = (Student*)malloc(sizeof(Student));
            if (newStudent == NULL) {
                printf("Memory allocation failed.\n");
                exit(1);
            }

            // Copy the read student data to the newly allocated memory
            strcpy(newStudent->firstName, student.firstName);
            strcpy(newStudent->lastName, student.lastName);
            strcpy(newStudent->phoneNumber, student.phoneNumber);

            // Read the grades from the file for the current student
            for (int i = 0; i < 10; i++) {
                fscanf(filePointer, "%d", &newStudent->grades[i]);
            }
            newStudent->next = NULL;

            // Add the newStudent to the end of the linked list for the specific level and class
            if (school[level - 1][class - 1] == NULL) {
                school[level - 1][class - 1] = newStudent;
            } else {
                Student* p = school[level - 1][class - 1];
                while (p->next != NULL) {
                    p = p->next;
                }
                p->next = newStudent;
            }
        } else {
            printf("Invalid level or class: %d, %d\n", level, class);
        }
    }
}

// Function to print the details of each student in the school array
void printSchool(Student* school[LEVELS][CLASS]) {
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            printf("Level: %d   ", level + 1); // Adjust level and class to one-based indexing
            printf("Class: %d\n\n", class + 1); // Adjust level and class to one-based indexing
            Student* p = school[level][class];
            while (p != NULL) {
                printf("Name: %s %s\n", p->firstName, p->lastName);
                printf("Phone Number: %s\n", p->phoneNumber);
                printf("Grades: ");
                for (int i = 0; i < 10; i++) {
                    printf("%d ", p->grades[i]);
                }
                printf("\n\n");
                p = p->next;
            }
        }
    }
}

// Function to free the memory allocated for each student
void freeSchoolMemory(Student* school[LEVELS][CLASS]) {
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            Student* p = school[level][class];
            while (p != NULL) {
                Student* temp = p;
                p = p->next;
                free(temp);
            }
        }
    }
}

int main() {
    FILE* filePointer = fopen("students_with_class.txt", "r");
    if (filePointer == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    Student* school[LEVELS][CLASS];
    for (int i = 0; i < LEVELS; i++) {
        for (int j = 0; j < CLASS; j++) {
            school[i][j] = NULL; // Initialize all elements to NULL
        }
    }

    readStudents(filePointer, school);
    fclose(filePointer);

    printSchool(school);

    freeSchoolMemory(school);

    return 0;
}
