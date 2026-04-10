#include "Rafli.h"
#include "Fauzan.h"
#include "noel.h"

// ===== Data Teks =====
char text[MAX_LINES][MAX_LENGTH];
int line_count = 1;

char currentFile[100] = "";

int cx = 0, cy = 0;
int row_offset = 0;

int mode = 0; // 0 = EDIT, 1 = COMMAND

// ===== KEY =====
enum keys {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};

// ===== FUNGSI PEMBANTU UI =====
void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// ===== INPUT =====
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

// ===== DISPLAY =====
void clearScreen() {
    printf("\033[H");
}

void printLineWithCursor(int row) {
    int i;
    int len = strlen(text[row]);
    printf("\033[K  "); 

    for (i = 0; i < len; i++) {
        if (row == cy && i == cx) {
            printf("|");
        }
        printf("%c", text[row][i]);
    }
    if (row == cy && cx == len) {
        printf("|");
    }
    printf("\n");
}

void editorRefreshScreen() {
    int i;
    printf("\033[?25l"); 
    clearScreen();

    printf("\033[K===== MINI NOTEPAD =====\n");
    printf("\033[KFile: %s\n", strlen(currentFile) ? currentFile : "(None)");
    printf("\033[KMODE: %s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf("\033[K\n\033[K========================\n");

    if (mode == 0) {
        printf("\033[KESC = Command Mode\n");
    } else {
        printf("\033[KI: Edit  O: Open   S: Save  A: SaveAs  N: New\n");
        printf("\033[KF: Find  R: Replace   C: Close Q: Quit\n");
        printf("\033[KY: Copy  P: Paste  U: Undo  E: Redo\n");
    }

    printf("\033[K----------------------------------\n");

    for (i = 0; i < VIEW_HEIGHT; i++) {
        int fileRow = i + row_offset;
        if (fileRow >= line_count) {
            printf("\033[K\n");
        } else {
            printLineWithCursor(fileRow);
        }
    }
    
    printf("\033[K----------------------------------\n");
    printf("\033[J"); 
    fflush(stdout);
}

// ===== CURSOR =====
void moveCursor(int key) {
    int len = strlen(text[cy]);
    switch (key) {
        case ARROW_UP:    if (cy > 0) cy--; break;
        case ARROW_DOWN:  if (cy < line_count - 1) cy++; break;
        case ARROW_LEFT:  if (cx > 0) cx--; break;
        case ARROW_RIGHT: if (cx < len) cx++; break;
        case PAGE_UP:     cy = (cy - VIEW_HEIGHT < 0) ? 0 : cy - VIEW_HEIGHT; break;
        case PAGE_DOWN:   cy = (cy + VIEW_HEIGHT >= line_count) ? line_count - 1 : cy + VIEW_HEIGHT; break;
    }
    if (cx > (int)strlen(text[cy])) cx = strlen(text[cy]);
    if (cy < row_offset) row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT) row_offset = cy - VIEW_HEIGHT + 1;
}

// ===== EDIT =====
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
    text[cy][len + 1] = '\0';
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
        text[cy][len - 1] = '\0';
    }
}

// ===== MAIN =====
int main() {
    int key;
    int i;
    
    for (i = 0; i < MAX_LINES; i++) {
        text[i][0] = '\0';
    }

    while (1) {
        editorRefreshScreen();
        key = readKey();

        if (mode == 0) {
            if (key == 27) mode = 1;
            else if (key == 13) insertNewLine();
            else if (key == 8) deleteChar();
            else if (key >= 32 && key <= 126) insertChar(key);
            else moveCursor(key);
        } 
        else {
            switch (key) {
                case 'i': case 'I': mode = 0; break;
                
                case 'o': case 'O': 
                    gotoxy(1, VIEW_HEIGHT + 10);
                    printf("\033[?25h");
                    openFile();
                    mode = 0; 
                    break;

                case 's': case 'S': saveFile(); break;

                case 'a': case 'A': 
                    gotoxy(1, VIEW_HEIGHT + 10);
                    printf("\033[?25h");
                    saveAs(); 
                    break;

                case 'f': case 'F': 
                    gotoxy(1, VIEW_HEIGHT + 10);
                    printf("\033[?25h");
                    findText(); 
                    break;

                case 'r': case 'R': 
                    gotoxy(1, VIEW_HEIGHT + 10);
                    printf("\033[?25h");
                    replaceText(); 
                    break;

                case 'c': case 'C': closeFile(); break;
                case 'n': case 'N': newFile(); break;
                case 'y': case 'Y': copyLine(); break;
                case 'p': case 'P': pasteLine(); break;
                case 'u': case 'U': undoAction(); break;
                case 'e': case 'E': redoAction(); break;
                
                case 'q': case 'Q': 
                    printf("\033[?25h\033[2J\033[H");
                    return 0;
            }
        }
    }
    return 0;
}