#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME 50
#define TABLE_SIZE 100

// ---------------- STRUCTURES ----------------

typedef struct Student {
    int roll;
    char name[MAX_NAME];
    char status;  // 'P' or 'A'
    struct Student* next;
} Student;

typedef struct QueueNode {
    Student* student;
    struct QueueNode* next;
} QueueNode;

// Hash table + Queue pointers
Student* hashTable[TABLE_SIZE] = {0};
QueueNode *front = NULL, *rear = NULL;

// ---------------- INPUT FLUSH ----------------

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ---------------- DATE VALIDATION ----------------

int isValidDateFormat(const char* date) {
    if (strlen(date) != 10) return 0;
    if (!isdigit(date[0]) || !isdigit(date[1]) || date[2] != '-' ||
        !isdigit(date[3]) || !isdigit(date[4]) || date[5] != '-' ||
        !isdigit(date[6]) || !isdigit(date[7]) || !isdigit(date[8]) || !isdigit(date[9]))
        return 0;

    int dd = (date[0]-'0')*10 + (date[1]-'0');
    int mm = (date[3]-'0')*10 + (date[4]-'0');
    int yyyy = (date[6]-'0')*1000 + (date[7]-'0')*100 + (date[8]-'0')*10 + (date[9]-'0');

    if (dd < 1 || dd > 31 || mm < 1 || mm > 12 || yyyy < 1900)
        return 0;

    return 1;
}

// ---------------- HASH FUNCTIONS ----------------

int hashFunction(int roll) {
    return roll % TABLE_SIZE;
}

Student* searchStudent(int roll) {
    int index = hashFunction(roll);
    Student* temp = hashTable[index];
    while (temp != NULL) {
        if (temp->roll == roll)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

// ---------------- FILE HANDLING FOR STUDENTS ----------------

void saveStudentToFile(Student* s) {
    FILE* fp = fopen("students.txt", "a");
    if (!fp) return;
    fprintf(fp, "%d %s\n", s->roll, s->name);
    fclose(fp);
}

void loadStudentsFromFile() {
    FILE* fp = fopen("students.txt", "r");
    if (!fp) return;

    int roll;
    char name[MAX_NAME];
    while (fscanf(fp, "%d %[^\n]\n", &roll, name) == 2) {
        Student* newStudent = (Student*)malloc(sizeof(Student));
        newStudent->roll = roll;
        strcpy(newStudent->name, name);
        newStudent->status = 'A';
        newStudent->next = NULL;

        int index = hashFunction(roll);
        if (hashTable[index] == NULL)
            hashTable[index] = newStudent;
        else {
            Student* temp = hashTable[index];
            while (temp->next != NULL)
                temp = temp->next;
            temp->next = newStudent;
        }
    }
    fclose(fp);
}

// ---------------- STUDENT FUNCTIONS ----------------

void insertStudent(int roll, char name[]) {
    if (searchStudent(roll) != NULL) {
        printf("Student with roll %d already exists!\n", roll);
        return;
    }

    Student* newStudent = (Student*)malloc(sizeof(Student));
    newStudent->roll = roll;
    strcpy(newStudent->name, name);
    newStudent->status = 'A';
    newStudent->next = NULL;

    int index = hashFunction(roll);
    if (hashTable[index] == NULL)
        hashTable[index] = newStudent;
    else {
        Student* temp = hashTable[index];
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = newStudent;
    }

    printf("Added: %s (Roll %d)\n", name, roll);
    saveStudentToFile(newStudent);
}

void deleteStudent(int roll) {
    int index = hashFunction(roll);
    Student* temp = hashTable[index];
    Student* prev = NULL;

    while (temp != NULL && temp->roll != roll) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Student with roll %d not found!\n", roll);
        return;
    }

    if (prev == NULL)
        hashTable[index] = temp->next;
    else
        prev->next = temp->next;

    free(temp);
    printf("Student with roll %d deleted from system.\n", roll);

    FILE* fp = fopen("students.txt", "r");
    FILE* tempFile = fopen("temp.txt", "w");
    if (!fp || !tempFile) return;

    int r;
    char name[MAX_NAME];
    while (fscanf(fp, "%d %[^\n]\n", &r, name) == 2) {
        if (r != roll)
            fprintf(tempFile, "%d %s\n", r, name);
    }

    fclose(fp);
    fclose(tempFile);

    remove("students.txt");
    rename("temp.txt", "students.txt");
}

// ---------------- QUEUE FUNCTIONS ----------------

void enqueue(Student* s) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->student = s;
    newNode->next = NULL;
    if (rear == NULL)
        front = rear = newNode;
    else {
        rear->next = newNode;
        rear = newNode;
    }
}

Student* dequeue() {
    if (front == NULL)
        return NULL;
    QueueNode* temp = front;
    Student* s = temp->student;
    front = front->next;
    if (front == NULL)
        rear = NULL;
    free(temp);
    return s;
}

// ---------------- CHECK IF ANY STUDENT EXISTS ----------------

int studentCount() {
    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Student* temp = hashTable[i];
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
    }
    return count;
}

// ---------------- ATTENDANCE ----------------

void markAttendance() {
    if (studentCount() == 0) {
        printf("\nNo students available to mark attendance!\n");
        return;
    }

    // Fill queue
    for (int i = 0; i < TABLE_SIZE; i++) {
        Student* temp = hashTable[i];
        while (temp != NULL) {
            enqueue(temp);
            temp = temp->next;
        }
    }

    Student* s;
    while ((s = dequeue()) != NULL) {
        char c;
        do {
            printf("Mark attendance for %s (Roll %d) [P/A]: ", s->name, s->roll);
            scanf(" %c", &c);
            flushInput();
            c = toupper(c);

            if (c != 'P' && c != 'A')
                printf("Invalid input! Enter only P or A.\n");

        } while (c != 'P' && c != 'A');

        s->status = c;
    }

    printf("\nAttendance completed!\n");
}

// ---------------- FILE HANDLING FOR ATTENDANCE ----------------

void saveAttendanceToFile(char date[]) {
    char filename[100];
    sprintf(filename, "attendance_%s.txt", date);

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Error creating file!\n");
        return;
    }

    fprintf(fp, "Attendance for Date: %s\n", date);
    fprintf(fp, "-----------------------------------------\n");
    fprintf(fp, "Roll\tName\t\tStatus\n");
    fprintf(fp, "-----------------------------------------\n");

    for (int i = 0; i < TABLE_SIZE; i++) {
        Student* temp = hashTable[i];
        while (temp != NULL) {
            fprintf(fp, "%d\t%-15s\t%c\n", temp->roll, temp->name, temp->status);
            temp = temp->next;
        }
    }

    fclose(fp);
    printf("Saved attendance to file: %s\n", filename);
}

// ---------------- DISPLAY ----------------

void displayStudents() {
    if (studentCount() == 0) {
        printf("\nNo students found!\n");
        return;
    }

    printf("\n----- Student List -----\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Student* temp = hashTable[i];
        while (temp != NULL) {
            printf("Roll: %d\tName: %s\tStatus: %c\n",
                   temp->roll, temp->name, temp->status);
            temp = temp->next;
        }
    }
    printf("-------------------------\n");
}

// ---------------- MAIN MENU ----------------

int main() {
    loadStudentsFromFile();

    char date[20];
    do {
        printf("Enter date (DD-MM-YYYY): ");
        scanf("%s", date);
        flushInput();
        if (!isValidDateFormat(date))
            printf("Invalid date format! Use DD-MM-YYYY.\n");

    } while (!isValidDateFormat(date));

    int choice;

    while (1) {
        printf("\n========= Attendance System =========\n");
        printf("1. Add Student\n");
        printf("2. Mark Attendance\n");
        printf("3. Display Students\n");
        printf("4. Save & Exit\n");
        printf("5. Delete Student\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        flushInput();

        if (choice == 1) {
            int roll;
            char name[MAX_NAME];
            printf("Enter Roll No: ");
            scanf("%d", &roll);
            flushInput();
            printf("Enter Name: ");
            scanf(" %[^\n]", name);
            insertStudent(roll, name);
        }
        else if (choice == 2)
            markAttendance();

        else if (choice == 3)
            displayStudents();

        else if (choice == 4) {
            saveAttendanceToFile(date);
            printf("Exiting...\n");
            break;
        }
        else if (choice == 5) {
            int roll;
            printf("Enter Roll No to delete: ");
            scanf("%d", &roll);
            flushInput();
            deleteStudent(roll);
        }
        else
            printf("Invalid choice!\n");
    }

    return 0;
}
