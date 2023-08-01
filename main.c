#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 


#define LEVELS 12
#define CLASS 10
#define MAXNUMF 50
#define MAXNUML 50
#define MAXNUMP 11

typedef struct Student {
    char firstName[MAXNUMF];
    char lastName[MAXNUML];
    char phoneNumber[MAXNUMP];
    int grades[10];
    struct Student* next;
} Student;

Student* school[LEVELS][CLASS];

enum menu_inputs {
    Insert = '0',
    Delete = '1',
    Edit = '2',
    Search = '3',
    Showall = '4',
    Top10 = '5',
    UnderperformedStudents = '6',
    Average = '7',
    Export = '8',
    Exit = '9'
};

/*
 * The function gets input from the user and ensures the input
 * doesn't exceed max_size. if yes - it will shrink it and will add \0.
 */
size_t get_input(char* dst, size_t max_size) {
    char* input = NULL;
    size_t len = 0;
    size_t len_size = 0;
    len_size = getline(&input, &len, stdin);
    if (len_size == -1)
        return -1;
    if (len_size < max_size) {
        input[len_size - 1] = '\0';
        strncpy(dst, input, len_size);
    }
    else {
        input[max_size - 1] = '\0';
        strncpy(dst, input, max_size);
        len_size = max_size;
    }
    free(input);
    return len_size;
}

size_t get_input_file(FILE* file,char* dst, size_t max_size) {
    char* input = NULL;
    size_t len = 0;
    size_t len_size = 0;
    len_size = getline(&input, &len, file);
    if (len_size == -1)
        return -1;
    if (len_size < max_size) {
        input[len_size - 1] = '\0';
        strncpy(dst, input, len_size);
    }
    else {
        input[max_size - 1] = '\0';
        strncpy(dst, input, max_size);
        len_size = max_size;
    }
    free(input);
    return len_size;
}

// Function to read students from file and build the school array

void readStudents(FILE* filePointer) {
    Student student;
    int level, class;
    while (fscanf(filePointer, "%s %s %s %d %d", student.firstName, student.lastName, student.phoneNumber, &level, &class) == 5) {
        // Check if level and class are within valid bounds
        if (level >= 1 && level <= LEVELS && class >= 1 && class <= CLASS) {
            int phoneNumberLength = strlen(student.phoneNumber);
            int containsOnlyDigits = 1;
             if(phoneNumberLength>MAXNUMP)
                containsOnlyDigits=0;

            for (int i = 0; i < phoneNumberLength; i++) {
                if (!isdigit(student.phoneNumber[i])) {
                    containsOnlyDigits = 0;
                    break;
                }
            }

            if (!containsOnlyDigits) {
                printf("Invalid phone number format for student: %s %s\n", student.firstName, student.lastName);
                continue;
            }

            int firstNameLength = strlen(student.firstName);
            if(strlen(student.firstName)>MAXNUMF){
                printf("firstname too long for student with phone number: %s\n", student.phoneNumber);
                continue;
            }

            int lastNameLength = strlen(student.lastName);
            if(strlen(student.lastName)>MAXNUML){
                printf("lastName too long for student with phone number: %s\n", student.phoneNumber);
                continue;
            }
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
            for (int i = 0; i < 10; i++) 
                fscanf(filePointer, "%d", &newStudent->grades[i]);
            
            newStudent->next = NULL;

            // Add the newStudent to the end of the linked list for the specific level and class
            if (school[level - 1][class - 1] == NULL) 
                school[level - 1][class - 1] = newStudent;
             else {
                Student* p = school[level - 1][class - 1];
                while (p->next != NULL) 
                    p = p->next;
                
                p->next = newStudent;
            }
        } else 
            printf("Invalid level or class: %d, %d\n", level, class);
 
    }

}

// Function to print the details of each student in the school array
void printAllStudents() {
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

void deleteStudent() {
    char phoneNumber[MAXNUMP];
    printf("Enter the phone number of the student to delete: ");
    scanf("%s", phoneNumber);
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            Student* p = school[level][class];
            Student* prev = NULL;

            while (p != NULL) {
                if (strcmp(p->phoneNumber, phoneNumber) == 0) {
                    // Student with matching phone number found, delete it
                    if (prev == NULL) {
                        // It's the first node, update school array
                        school[level][class] = p->next;
                    } else {
                        // Update previous node's next pointer
                        prev->next = p->next;
                    }
                    free(p);
                    printf("Student with phone number %s deleted successfully.\n", phoneNumber);
                    return; 
                }
                prev = p;
                p = p->next;
            }
        }
    }

    // If the function reaches this point, the student was not found
    printf("Student with phone number %s not found.\n", phoneNumber);
}

void insertNewStudent() {
    char firstName[50];
    char lastName[50];
    char phoneNumber[11];
    int level, class;

    
    
    printf("Enter the student's first name: ");
    get_input(firstName,MAXNUMF);
    while(strlen(firstName)>MAXNUMF){
        printf("first name too long:, Enter again:");
        get_input(firstName,MAXNUMF);
    }
    

    printf("Enter the student's last name: ");
    get_input(lastName,MAXNUML);
    while(strlen(lastName)>MAXNUML){
        printf("last name too long, Enter again:");
        get_input(lastName,MAXNUML);
    }

    printf("Enter the student's phone number: ");
    get_input(phoneNumber,MAXNUMP);
    int phoneNumberLength = strlen(phoneNumber);
    int containsOnlyDigits = 1;
    if(phoneNumberLength>MAXNUMP)
        containsOnlyDigits=0;

    for (int i = 0; i < phoneNumberLength; i++) {
            if (!isdigit(phoneNumber[i])) {
                containsOnlyDigits = 0;
                break;
            }
        }
     while (!containsOnlyDigits) {
        printf("Invalid phone number format, Enter again:");
        get_input(phoneNumber,MAXNUMP);
        phoneNumberLength = strlen(phoneNumber);
        containsOnlyDigits = 1;
         if(phoneNumberLength>MAXNUMP)
            containsOnlyDigits=0;
        for (int i = 0; i < phoneNumberLength; i++) {
            if (!isdigit(phoneNumber[i])) {
                containsOnlyDigits = 0;
                break;
            }
        }   
    }
    

    printf("Enter the student's level (1-12): ");
    scanf("%d", &level);

    printf("Enter the student's class (1-10): ");
    scanf("%d", &class);

    // Check if level and class are within valid bounds
    while(level<0||level>LEVELS||class<0||class>CLASS){
        printf("class or level not in range. Enter again:");
        printf("Enter the student's level (1-12): ");
        scanf("%d", &level);
        printf("Enter the student's class (1-10): ");
        scanf("%d", &class);
    }
    if (level >= 1 && level <= LEVELS && class >= 1 && class <= CLASS) {
        // Dynamically allocate memory for the student
        Student* newStudent = (Student*)malloc(sizeof(Student));
        if (newStudent == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }

        // Copy the entered student data to the newly allocated memory
        strcpy(newStudent->firstName, firstName);
        strcpy(newStudent->lastName, lastName);
        strcpy(newStudent->phoneNumber, phoneNumber);

        // Read the grades from the user for the current student
        printf("Enter the student's grades (10 integers separated by spaces): ");
        for (int i = 0; i < 10; i++) {
            scanf("%d", &newStudent->grades[i]);
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


void editStudentGrade() {
    char phoneNumber[11];
    int found = 0;

    printf("Enter the phone number of the student whose grades you want to edit: ");
    scanf("%s", phoneNumber);

    // Traverse the entire school array to find the student with the given phone number
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            Student* p = school[level][class];
            while (p != NULL) {
                if (strcmp(p->phoneNumber, phoneNumber) == 0) {
                    found = 1;
                    printf("Student found!\n");
                    printf("Name: %s %s\n", p->firstName, p->lastName);
                    printf("Phone Number: %s\n", p->phoneNumber);
                    printf("Grades before editing: ");
                    for (int i = 0; i < 10; i++) {
                        printf("%d ", p->grades[i]);
                    }
                    printf("\n");
                    int choice;
                    int grade;
                    do{
                    printf("Enter the number of grade you want to edit(0-9): ");
                    scanf("%d", &choice);
                    }while(choice<0||choice>9);
                    do{
                    printf("Enter the grade: ");
                    scanf("%d", &grade);
                    }while(grade<0||grade>100);
                    p->grades[choice]=grade;

                    printf("Grades after editing: ");
                    for (int i = 0; i < 10; i++) {
                        printf("%d ", p->grades[i]);
                    }
                    printf("\n");

                    break; // Exit the loop as we have found the student
                }
                p = p->next;
            }
            if (found) {
                break; // Exit the outer loop as we have found the student
            }
        }
        if (found) {
            break; // Exit the outermost loop as we have found the student
        }
    }

    if (!found) {
        printf("Student with phone number %s not found.\n", phoneNumber);
    }
}

Student searchStudent(char firstName[MAXNUMF],char lastName[MAXNUML]) {
    
    
    Student* listofstudents[CLASS*LEVELS];
    int found = 0;

   
    // Traverse the entire school array to find the student with the given first name and last name
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            Student* p = school[level][class];
            while (p != NULL) {
                if (strcmp(p->firstName, firstName) == 0 && strcmp(p->lastName, lastName) == 0) {
                    listofstudents[found]=p;
                    found++;
                    printf("Name: %s %s\n", p->firstName, p->lastName);
                    printf("Phone Number: %s\n", p->phoneNumber);
                    printf("Level: %d\n", level + 1);
                    printf("Class: %d\n", class + 1);
                    printf("Grades: ");
                    for (int i = 0; i < 10; i++) {
                        printf("%d ", p->grades[i]);
                    }
                    printf("\n");

                    
                }
                p = p->next;
            }
            
        }
        
    }
    int index=0;
    if (!found) {
        printf("Student with first name %s and last name %s not found.\n", firstName, lastName);
        return;
    }else if(found>1){
        printf("There are some students with this name. choose one of them from the list above with index between 0 to %d\n",found);
        do{
        printf("Enter the number of index: ");
        scanf("%d", &index);
        }while(index<0||index>found);
    }
    return *listofstudents[index];
}


void printTopNStudentsPerCourse() {
     int subjectIndex;
     int numberOfStudent=10;
    do{
        printf("Enter the number of subject(1-10): ");
        scanf("%d", &subjectIndex);
    }while(subjectIndex<1||subjectIndex>10);
    

    // Create an array to store the top 10 outstanding students for each class
    Student* topStudents[LEVELS][numberOfStudent]; // Assuming each class can have up to 10 outstanding students
    int numTopStudents[LEVELS] = {0}; // Number of outstanding students in each class

    // Traverse the entire school array and find the top 10 outstanding students for each class
    for (int level = 0; level < LEVELS; level++) {
        for (int class = 0; class < CLASS; class++) {
            Student* p = school[level][class];
            while (p != NULL) {
               
                // Sort the students by the given subject's grades in descending order
                for (int i = 0; i < numTopStudents[level]; i++) {
                    if (p->grades[subjectIndex-1] >= topStudents[level][i]->grades[subjectIndex-1])
                    {
                        // Shift the existing outstanding students down to make space for the new one
                        for (int j = numTopStudents[level]; j > i; j--) {
                            topStudents[level][j] = topStudents[level][j - 1];
                        }
                        topStudents[level][i] = p;
                        if (numTopStudents[level] < 10) {
                            numTopStudents[level]++;
                        }
                        break;
                    }
                   
                }

                // If there are less than 10 outstanding students in the class, add the student to the list
                if (numTopStudents[level] < numberOfStudent) {
                    topStudents[level][numTopStudents[level]] = p;
                    numTopStudents[level]++;
                }

                p = p->next;
            }
        }
    }

    // Print the top 10 outstanding students for each class
    for (int level = 0; level < LEVELS; level++) {
        printf("Top 10 outstanding students in level %d for Subject %d:\n", level + 1, subjectIndex);
        for (int i = 0; i < numTopStudents[level]; i++) {
            printf("%s %s\n", topStudents[level][i]->firstName, topStudents[level][i]->lastName);
        }
        printf("\n");
    }
}



void menu() {
    

    char input;
    // school.name = "schoolName";
    do {
        printf("\n|School Manager<::>Home|\n");
        printf("--------------------------------------------------------------------------------\n");
        //printf("Welcome to ( %s ) School!\nYou have inserted ( %zu ) students.\n\n", school.name, school.numOfStudents);
        printf("\t[0] |--> Insert\n");
        printf("\t[1] |--> Delete\n");
        printf("\t[2] |--> Edit\n");
        printf("\t[3] |--> Search\n");
        printf("\t[4] |--> Show All\n");
        printf("\t[5] |--> Top 10 students per course\n");
        printf("\t[6] |--> Underperformed students\n");
        printf("\t[7] |--> Average per course\n");
        printf("\t[8] |--> Export\n");
        printf("\t[9] |--> Exit\n");
        printf("\n\tPlease Enter Your Choice (0-9): ");
        input = getc(stdin);
        fflush(stdin);
        getc(stdin);
        switch (input) {
            case Insert:
                insertNewStudent();
                break;
            case Delete:
                deleteStudent();
                break;
            case Edit:
                editStudentGrade();
                break;
            case Search:
            char firstName[MAXNUMF];
            char lastName[MAXNUML];
            printf("Enter the first name of the student: ");
            scanf("%s", firstName);
            printf("Enter the last name of the student: ");
            scanf("%s", lastName);
            searchStudent(firstName,lastName);
                break;
            case Showall:
                printAllStudents();
                break;
            case Top10:
                printTopNStudentsPerCourse();
                break;
            case UnderperformedStudents:
                //printUnderperformedStudents();
                break;
            case Average:
                //printAverage();
                break;
            case Export:
                //exportDatabase();
                break;
            case Exit:
                //handleClosing();
                break;
            default:
                printf("\nThere is no item with symbol \"%c\".Please enter a number between 1-10!\nPress any key to continue...",
                       input);
                getc(stdin);
                getc(stdin);
                break;
        }
    } while (input != Exit);
}


int main() {
     FILE* filePointer = fopen("students_with_class.txt", "r");
    if (filePointer == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    
    for (int i = 0; i < LEVELS; i++) {
        for (int j = 0; j < CLASS; j++) {
            school[i][j] = NULL; // Initialize all elements to NULL
        }
    }

    readStudents(filePointer);
    fclose(filePointer);

    printAllStudents(school);
    menu();
    freeSchoolMemory(school);
    return 0;
}

