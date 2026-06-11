/*
 * grade_manager.c — Student Grade Management System
 *
 * Features:
 *   - Add / view / search / delete students
 *   - Auto-calculates GPA and letter grade
 *   - Sorts students by GPA (bubble sort)
 *   - Saves and loads data from a CSV file (persistent storage)
 *
 * Compile: gcc -o grade_manager grade_manager.c
 * Run:     ./grade_manager
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define NAME_LEN     64
#define DATA_FILE    "students.csv"

/* ── types ───────────────────────────────────────────────── */

typedef struct {
    int   id;
    char  name[NAME_LEN];
    float marks[5];   /* 5 subjects */
    float gpa;
    char  grade;
} Student;

static Student db[MAX_STUDENTS];
static int     count = 0;
static int     next_id = 1;

/* ── grade calculation ───────────────────────────────────── */

static float calc_gpa(float marks[5]) {
    float sum = 0;
    for (int i = 0; i < 5; i++) sum += marks[i];
    return sum / 5.0f;
}

static char calc_grade(float gpa) {
    if (gpa >= 90) return 'O';
    if (gpa >= 80) return 'A';
    if (gpa >= 70) return 'B';
    if (gpa >= 60) return 'C';
    if (gpa >= 50) return 'D';
    return 'F';
}

/* ── file I/O ────────────────────────────────────────────── */

static void save_to_file(void) {
    FILE *f = fopen(DATA_FILE, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "id,name,s1,s2,s3,s4,s5,gpa,grade\n");
    for (int i = 0; i < count; i++) {
        Student *s = &db[i];
        fprintf(f, "%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%c\n",
                s->id, s->name,
                s->marks[0], s->marks[1], s->marks[2],
                s->marks[3], s->marks[4],
                s->gpa, s->grade);
    }
    fclose(f);
    printf("Saved to %s\n", DATA_FILE);
}

static void load_from_file(void) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) return;   /* first run — no file yet */
    char line[256];
    fgets(line, sizeof(line), f);   /* skip header */
    count = 0;
    while (fgets(line, sizeof(line), f) && count < MAX_STUDENTS) {
        Student *s = &db[count];
        sscanf(line, "%d,%63[^,],%f,%f,%f,%f,%f,%f,%c",
               &s->id, s->name,
               &s->marks[0], &s->marks[1], &s->marks[2],
               &s->marks[3], &s->marks[4],
               &s->gpa, &s->grade);
        if (s->id >= next_id) next_id = s->id + 1;
        count++;
    }
    fclose(f);
    printf("Loaded %d students from %s\n", count, DATA_FILE);
}

/* ── CRUD ────────────────────────────────────────────────── */

static void add_student(void) {
    if (count >= MAX_STUDENTS) {
        printf("Database full.\n"); return;
    }
    Student *s = &db[count];
    s->id = next_id++;

    printf("Enter name: ");
    scanf(" %63[^\n]", s->name);

    printf("Enter marks for 5 subjects (0-100):\n");
    for (int i = 0; i < 5; i++) {
        printf("  Subject %d: ", i + 1);
        scanf("%f", &s->marks[i]);
        if (s->marks[i] < 0 || s->marks[i] > 100) {
            printf("Invalid mark. Setting to 0.\n");
            s->marks[i] = 0;
        }
    }
    s->gpa   = calc_gpa(s->marks);
    s->grade = calc_grade(s->gpa);
    count++;

    printf("Added student [ID=%d] GPA=%.2f Grade=%c\n",
           s->id, s->gpa, s->grade);
    save_to_file();
}

static void print_student(const Student *s) {
    printf("  ID:%-4d  Name:%-20s  GPA:%-6.2f  Grade:%c  "
           "Marks:[%.0f %.0f %.0f %.0f %.0f]\n",
           s->id, s->name, s->gpa, s->grade,
           s->marks[0], s->marks[1], s->marks[2],
           s->marks[3], s->marks[4]);
}

static void view_all(void) {
    if (count == 0) { printf("No students found.\n"); return; }
    printf("\n%-4s  %-20s  %-6s  %-5s  Marks\n",
           "ID", "Name", "GPA", "Grade");
    printf("%s\n", "-------------------------------------------------------");
    for (int i = 0; i < count; i++)
        print_student(&db[i]);
    printf("\nTotal: %d students\n\n", count);
}

static void search_student(void) {
    char query[NAME_LEN];
    printf("Enter name to search: ");
    scanf(" %63[^\n]", query);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strstr(db[i].name, query)) {
            print_student(&db[i]);
            found++;
        }
    }
    if (!found) printf("No student found with name containing '%s'\n", query);
}

static void delete_student(void) {
    int id;
    printf("Enter student ID to delete: ");
    scanf("%d", &id);
    for (int i = 0; i < count; i++) {
        if (db[i].id == id) {
            printf("Deleted: %s\n", db[i].name);
            db[i] = db[--count];   /* replace with last */
            save_to_file();
            return;
        }
    }
    printf("Student ID %d not found.\n", id);
}

/* ── sort by GPA (descending) ────────────────────────────── */

static void sort_by_gpa(void) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (db[j].gpa < db[j+1].gpa) {
                Student tmp = db[j];
                db[j]       = db[j+1];
                db[j+1]     = tmp;
            }
        }
    }
    printf("Sorted by GPA (highest first):\n");
    view_all();
}

/* ── statistics ──────────────────────────────────────────── */

static void show_stats(void) {
    if (count == 0) { printf("No data.\n"); return; }
    float sum = 0, highest = db[0].gpa, lowest = db[0].gpa;
    int   pass = 0;
    for (int i = 0; i < count; i++) {
        sum += db[i].gpa;
        if (db[i].gpa > highest) highest = db[i].gpa;
        if (db[i].gpa < lowest)  lowest  = db[i].gpa;
        if (db[i].grade != 'F')  pass++;
    }
    printf("\n=== Class Statistics ===\n");
    printf("  Total students : %d\n", count);
    printf("  Class average  : %.2f\n", sum / count);
    printf("  Highest GPA    : %.2f\n", highest);
    printf("  Lowest GPA     : %.2f\n", lowest);
    printf("  Pass rate      : %.1f%%\n", (float)pass / count * 100);
    printf("========================\n\n");
}

/* ── menu ────────────────────────────────────────────────── */

int main(void) {
    load_from_file();
    int choice;

    while (1) {
        printf("\n===== Grade Manager =====\n");
        printf("1. Add student\n");
        printf("2. View all\n");
        printf("3. Search by name\n");
        printf("4. Delete student\n");
        printf("5. Sort by GPA\n");
        printf("6. Class statistics\n");
        printf("0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_student();   break;
            case 2: view_all();      break;
            case 3: search_student(); break;
            case 4: delete_student(); break;
            case 5: sort_by_gpa();   break;
            case 6: show_stats();    break;
            case 0: printf("Goodbye!\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
