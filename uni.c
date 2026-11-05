#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 500
#define MAX_COURSES 50
#define MAX_TEACHERS 50
#define MAX_PER_COURSE 100
#define NAME_LEN 50
#define DATA_FILE "university.dat"

typedef struct {
    int id;
    char name[NAME_LEN];
    char dept[NAME_LEN];
} Student;

typedef struct {
    int id;
    char name[NAME_LEN];
    char dept[NAME_LEN];
} Teacher;

typedef struct {
    int id;
    char code[16];
    char title[NAME_LEN];
    int teacherId;                 /* 0 = no teacher assigned */
    int enrolled;                  /* number of enrolled students */
    int studentIds[MAX_PER_COURSE];
} Course;

static Student students[MAX_STUDENTS];
static Teacher teachers[MAX_TEACHERS];
static Course  courses[MAX_COURSES];
static int studentCount = 0, teacherCount = 0, courseCount = 0;
static int nextStudentId = 1001, nextTeacherId = 2001, nextCourseId = 3001;

/* ---------- input helpers ---------- */
static void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}

static int readInt(const char *prompt) {
    char buf[32];
    readLine(prompt, buf, sizeof buf);
    return atoi(buf);
}

/* ---------- lookups (return index or -1) ---------- */
static int findStudent(int id) {
    for (int i = 0; i < studentCount; i++) if (students[i].id == id) return i;
    return -1;
}
static int findTeacher(int id) {
    for (int i = 0; i < teacherCount; i++) if (teachers[i].id == id) return i;
    return -1;
}
static int findCourse(int id) {
    for (int i = 0; i < courseCount; i++) if (courses[i].id == id) return i;
    return -1;
}

/* ---------- save / load ---------- */
static void saveData(void) {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) { printf("Could not save data.\n"); return; }
    fwrite(&studentCount, sizeof(int), 1, f);
    fwrite(&teacherCount, sizeof(int), 1, f);
    fwrite(&courseCount, sizeof(int), 1, f);
    fwrite(&nextStudentId, sizeof(int), 1, f);
    fwrite(&nextTeacherId, sizeof(int), 1, f);
    fwrite(&nextCourseId, sizeof(int), 1, f);
    fwrite(students, sizeof(Student), studentCount, f);
    fwrite(teachers, sizeof(Teacher), teacherCount, f);
    fwrite(courses, sizeof(Course), courseCount, f);
    fclose(f);
}

static void loadData(void) {
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    if (fread(&studentCount, sizeof(int), 1, f) != 1 ||
        fread(&teacherCount, sizeof(int), 1, f) != 1 ||
        fread(&courseCount, sizeof(int), 1, f) != 1 ||
        fread(&nextStudentId, sizeof(int), 1, f) != 1 ||
        fread(&nextTeacherId, sizeof(int), 1, f) != 1 ||
        fread(&nextCourseId, sizeof(int), 1, f) != 1) {
        studentCount = teacherCount = courseCount = 0;
        fclose(f);
        return;
    }
    fread(students, sizeof(Student), studentCount, f);
    fread(teachers, sizeof(Teacher), teacherCount, f);
    fread(courses, sizeof(Course), courseCount, f);
    fclose(f);
}

/* ---------- student / admission features ---------- */
static void admitStudent(void) {
    if (studentCount >= MAX_STUDENTS) { printf("Student limit reached.\n"); return; }
    Student s;
    s.id = nextStudentId++;
    readLine("Student name: ", s.name, NAME_LEN);
    readLine("Department: ", s.dept, NAME_LEN);
    if (s.name[0] == '\0') { printf("Name cannot be empty.\n"); nextStudentId--; return; }
    students[studentCount++] = s;
    saveData();
    printf("Admission successful! Student ID: %d\n", s.id);
}

static void listStudents(void) {
    printf("\nTotal students admitted: %d\n", studentCount);
    if (studentCount == 0) return;
    printf("%-8s %-25s %-20s\n", "ID", "Name", "Department");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < studentCount; i++)
        printf("%-8d %-25s %-20s\n", students[i].id, students[i].name, students[i].dept);
}

/* ---------- course features ---------- */
static void addCourse(void) {
    if (courseCount >= MAX_COURSES) { printf("Course limit reached.\n"); return; }
    Course c;
    memset(&c, 0, sizeof c);
    c.id = nextCourseId++;
    readLine("Course code (e.g. CSE101): ", c.code, sizeof c.code);
    readLine("Course title: ", c.title, NAME_LEN);
    if (c.code[0] == '\0') { printf("Code cannot be empty.\n"); nextCourseId--; return; }
    courses[courseCount++] = c;
    saveData();
    printf("Course added! Course ID: %d\n", c.id);
}

static void enrollStudent(void) {
    int sid = readInt("Student ID: ");
    if (findStudent(sid) < 0) { printf("Student not found.\n"); return; }
    int cid = readInt("Course ID: ");
    int ci = findCourse(cid);
    if (ci < 0) { printf("Course not found.\n"); return; }
    Course *c = &courses[ci];
    for (int i = 0; i < c->enrolled; i++)
        if (c->studentIds[i] == sid) { printf("Already enrolled in this course.\n"); return; }
    if (c->enrolled >= MAX_PER_COURSE) { printf("Course is full.\n"); return; }
    c->studentIds[c->enrolled++] = sid;
    saveData();
    printf("Student %d enrolled in %s.\n", sid, c->code);
}

static void listCourses(void) {
    printf("\n%-8s %-10s %-28s %-10s %-8s\n", "ID", "Code", "Title", "Teacher", "Enrolled");
    printf("-------------------------------------------------------------------\n");
    for (int i = 0; i < courseCount; i++) {
        int ti = findTeacher(courses[i].teacherId);
        printf("%-8d %-10s %-28s %-10s %-8d\n", courses[i].id, courses[i].code,
               courses[i].title, ti >= 0 ? teachers[ti].name : "-", courses[i].enrolled);
    }
    if (courseCount == 0) printf("No courses yet.\n");
}

static void printCourseStudents(const Course *c) {
    printf("\n[%s] %s -- %d student(s) enrolled\n", c->code, c->title, c->enrolled);
    for (int j = 0; j < c->enrolled; j++) {
        int si = findStudent(c->studentIds[j]);
        if (si >= 0)
            printf("   %d. %-6d %-25s %s\n", j + 1, students[si].id, students[si].name, students[si].dept);
    }
}

static void courseReport(void) {
    if (courseCount == 0) { printf("No courses yet.\n"); return; }
    for (int i = 0; i < courseCount; i++) printCourseStudents(&courses[i]);
}

/* ---------- teacher features ---------- */
static void addTeacher(void) {
    if (teacherCount >= MAX_TEACHERS) { printf("Teacher limit reached.\n"); return; }
    Teacher t;
    t.id = nextTeacherId++;
    readLine("Teacher name: ", t.name, NAME_LEN);
    readLine("Department: ", t.dept, NAME_LEN);
    if (t.name[0] == '\0') { printf("Name cannot be empty.\n"); nextTeacherId--; return; }
    teachers[teacherCount++] = t;
    saveData();
    printf("Teacher added! Teacher ID: %d\n", t.id);
}

static void listTeachers(void) {
    printf("\nTotal teachers: %d\n", teacherCount);
    if (teacherCount == 0) return;
    printf("%-8s %-25s %-20s\n", "ID", "Name", "Department");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < teacherCount; i++)
        printf("%-8d %-25s %-20s\n", teachers[i].id, teachers[i].name, teachers[i].dept);
}

static void assignTeacher(void) {
    int tid = readInt("Teacher ID: ");
    if (findTeacher(tid) < 0) { printf("Teacher not found.\n"); return; }
    int cid = readInt("Course ID: ");
    int ci = findCourse(cid);
    if (ci < 0) { printf("Course not found.\n"); return; }
    courses[ci].teacherId = tid;
    saveData();
    printf("Teacher %d assigned to %s.\n", tid, courses[ci].code);
}

static void teacherCourses(void) {
    int tid = readInt("Your Teacher ID: ");
    int ti = findTeacher(tid);
    if (ti < 0) { printf("Teacher not found.\n"); return; }
    printf("\nWelcome, %s (%s)\n", teachers[ti].name, teachers[ti].dept);
    int found = 0;
    for (int i = 0; i < courseCount; i++) {
        if (courses[i].teacherId == tid) { printCourseStudents(&courses[i]); found = 1; }
    }
    if (!found) printf("No courses assigned to you yet.\n");
}

/* ---------- menus ---------- */
static void studentMenu(void) {
    int ch;
    do {
        printf("\n=== STUDENT / ADMISSION PANEL ===\n"
               "1. New admission\n2. View all admitted students (count + list)\n"
               "3. Add course\n4. Enroll student in course\n"
               "5. List courses\n6. Course-wise enrollment report\n0. Back\n");
        ch = readInt("Choice: ");
        switch (ch) {
            case 1: admitStudent(); break;
            case 2: listStudents(); break;
            case 3: addCourse(); break;
            case 4: enrollStudent(); break;
            case 5: listCourses(); break;
            case 6: courseReport(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (ch != 0);
}

static void teacherMenu(void) {
    int ch;
    do {
        printf("\n=== TEACHER PANEL ===\n"
               "1. Add teacher\n2. View all teachers\n"
               "3. Assign teacher to course\n4. My courses & students\n0. Back\n");
        ch = readInt("Choice: ");
        switch (ch) {
            case 1: addTeacher(); break;
            case 2: listTeachers(); break;
            case 3: assignTeacher(); break;
            case 4: teacherCourses(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (ch != 0);
}

int main(void) {
    loadData();
    int ch;
    do {
        printf("\n===== UNIVERSITY MANAGEMENT SYSTEM =====\n"
               "1. Student / Admission panel\n2. Teacher panel\n0. Exit\n");
        ch = readInt("Choice: ");
        switch (ch) {
            case 1: studentMenu(); break;
            case 2: teacherMenu(); break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (ch != 0);
    saveData();
    return 0;
}