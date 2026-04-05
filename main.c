/*Library*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES 1000
#define MAX_LENGTH 256
#define VIEW_HEIGHT 20
/*Program*/
/* Data Teks*/
char text[MAX_LINES][MAX_LENGTH];
int line_count = 1;

char currentFile[100] = "";

int cx = 0, cy = 0;
int row_offset = 0;

int mode = 0; // 0 = EDIT, 1 = COMMAND
/*Page up & Page up*/
// ================= KEY =================
enum keys {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};
/*Input Keyboard*/
int readKey() {
    int c = getch();

    if (c == 0 || c == 224) {
        int c2 = getch();

        switch (c2) {
            case 72: return ARROW_UP;
            case 80: return ARROW_DOWN;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            case 73: return PAGE_UP;
            case 81: return PAGE_DOWN;
        }
    }

    return c;
}
/*Display Program*/
// ================= DISPLAY =================
void clearScreen() {
    system("cls");
}

void printLineWithCursor(int row) {
    int i;
    int len = strlen(text[row]);

    printf("%3d | ", row + 1);

    for (i = 0; i <= len; i++) {
        if (row == cy && i == cx) {
            printf("|");
        }
        if (i < len) {
            printf("%c", text[row][i]);
        }
    }

    printf("\n");
}

void editorRefreshScreen() {
    int i;

    clearScreen();

    printf("===== MINI NOTEPAD =====\n");
    printf("File: %s\n", strlen(currentFile) ? currentFile : "(None)");
    printf("MODE: %s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf("========================\n\n");

    for (i = 0; i < VIEW_HEIGHT; i++) {
        int fileRow = i + row_offset;

        if (fileRow >= line_count) {
            printf("~\n");
        } else {
            printLineWithCursor(fileRow);
        }
    }

    // COMMAND LIST DINAMIS
    if (mode == 0) {
        printf("\nESC = Command Mode\n");
    } else {
        printf("\n===== COMMAND LIST =====\n");
        printf("I  : Kembali ke Edit Mode\n");
        printf("O  : Open File\n");
        printf("S  : Save File\n");
        printf("A  : Save As\n");
        printf("C  : Close File\n");
        printf("Q  : Quit\n");
        printf("========================\n");
    }
}
