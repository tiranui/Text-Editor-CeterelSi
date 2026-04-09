#include "noel.h"

/*Program*/
/* Data Teks*/
char text[MAX_LINES][MAX_LENGTH];
int line_count = 1;

char currentFile[100] = "";

int cx = 0, cy = 0;
int row_offset = 0;

int mode = 0; // 0 = EDIT, 1 = COMMAND

/*Page up & Page down*/
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
        printf("N  : New File\n");
        printf("Y  : Copy Baris (Menyesuaikan dengan Kursor)\n");
        printf("P  : Paste\n");
        printf("Z  : Undo\n");
        printf("R  : Redo\n");
        printf("Q  : Quit\n");
        printf("========================\n");
    }
}

/*Program edit dan move cursor*/
// ================= CURSOR =================
void moveCursor(int key) {
    int len = strlen(text[cy]);

    switch (key) {
        case ARROW_UP:
            if (cy > 0) cy--;
            break;

        case ARROW_DOWN:
            if (cy < line_count - 1) cy++;
            break;

        case ARROW_LEFT:
            if (cx > 0) cx--;
            break;

        case ARROW_RIGHT:
            if (cx < len) cx++;
            break;

        case PAGE_UP:
            cy -= VIEW_HEIGHT;
            if (cy < 0) cy = 0;
            break;

        case PAGE_DOWN:
            cy += VIEW_HEIGHT;
            if (cy >= line_count) cy = line_count - 1;
            break;
    }

    if (cx > strlen(text[cy])) {
        cx = strlen(text[cy]);
    }

    if (cy < row_offset) row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT) {
        row_offset = cy - VIEW_HEIGHT + 1;
    }
}

// ================= EDIT =================
void insertChar(char c) {
    int len = strlen(text[cy]);
    int i;

    if (len >= MAX_LENGTH - 1) return;

    pushUndo();

    for (i = len; i >= cx; i--) {
        text[cy][i + 1] = text[cy][i];
    }

    text[cy][cx] = c;
    cx++;
}

void insertNewLine() {
    int i;

    if (line_count >= MAX_LINES) return;

    pushUndo();

    for (i = line_count; i > cy + 1; i--) {
        strcpy(text[i], text[i - 1]);
    }

    strcpy(text[cy + 1], text[cy] + cx);
    text[cy][cx] = '\0';

    line_count++;
    cy++;
    cx = 0;
}

void deleteChar() {
    int len = strlen(text[cy]);
    int i;

    if (cx > 0) {
        pushUndo();

        for (i = cx - 1; i < len; i++) {
            text[cy][i] = text[cy][i + 1];
        }
        cx--;
    }
}

/*main program*/
/*================= MAIN =================*/
int main() {
    int key;

    text[0][0] = '\0';
    int i;
    for (i = 0; i < MAX_LINES; i++) {
        text[i][0] = '\0';
    }

    while (1) {
        editorRefreshScreen();
        key = readKey();

        if (mode == 0) {
            if (key == 27) {
                mode = 1;
            }
            else if (key == 13) {
                insertNewLine();
            }
            else if (key == 8) {
                deleteChar();
            }
            else if (key >= 32 && key <= 126) {
                insertChar(key);
            }
            else {
                moveCursor(key);
            }
        } else {
            switch (key) {
                case 'i':
                case 'I':
                    mode = 0;
                    break;

                case 'n':
                case 'N':
                    newFile();
                    mode = 0;
                    break;

                case 'y':
                case 'Y':
                    copyLine();
                    break;


                case 'p':
                case 'P':
                    pasteLine();
                    break;

                case 'z':
                case 'Z':
                    undoAction();
                    break;

                case 'r':
                case 'R':
                    redoAction();
                    break;

                case 'q':
                case 'Q':
                    return 0;
            }
        }
    }

    return 0;
}