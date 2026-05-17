#include "Rafli.h"

Node *head    = NULL;
Node *current = NULL;
int   line_count  = 1;
char  currentFile[100] = "";
int   cx = 0, cy = 0;
int   row_offset = 0;
int   mode = 0;

enum keys {
    ARROW_UP   = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};

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

void clearScreen() {
    COORD posisi = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posisi);
}

void setCursorVisibility(int visible) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = visible; // 0 (false) untuk sembunyi, 1 (true) untuk muncul
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void printLineWithCursor(int row) {
    Node *node = getNodeAt(row);
    int i, len;

    if (node == NULL) return;

    len = strlen(node->line);
    printf("%3d | ", row + 1);

    for (i = 0; i <= len; i++) {
        if (row == cy && i == cx) {
            printf("|");
        }
        if (i < len) {
            printf("%c", node->line[i]);
        }
    }
    printf("                                                    \n");
}

void editorRefreshScreen() {
    int i;

    clearScreen();
    printf("========================================\n");
    printf("         MINI NOTEPAD                   \n");
    printf("========================================\n");
    printf(" File : %-40s\n", strlen(currentFile) ? currentFile : "(Belum ada)");
    printf(" Mode : %-40s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf(" Brs  : %-5d  |  Kol : %-5d\n", cy + 1, cx + 1);
    printf("========================================\n\n");

    for (i = 0; i < VIEW_HEIGHT; i++) {
        int fileRow = i + row_offset;
        if (fileRow >= line_count) {
            printf("~                                                       \n");
        } else {
            printLineWithCursor(fileRow);
        }
    }

    printf("\n");
    if (mode == 0) {
        printf("[ESC] Masuk Command Mode                \n");
        printf("                                        \n");
        printf("                                        \n");
        printf("                                        \n");
        printf("========================================\n");
    } else {
        printf("========================================\n");
        printf("[I]Edit  [O]Buka  [S]Simpan             \n");
        printf("[A]SimpanSebagai [C]Tutup               \n");
        printf("[Q]Keluar                               \n");
        printf("========================================\n");
    }
    printf("                                                                \n");
    printf("                                                                \n");
    printf("                                                                \n");

    if (mode == 0) {
        COORD kursorTeks = {6 + cx, 8 + (cy - row_offset)};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), kursorTeks);
    } else {
        COORD kursorMenu = {0, 33};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), kursorMenu);
    }
}

void moveCursor(int key) {
    switch (key) {
        case ARROW_UP:
            if (cy > 0) {
                cy--;
                current = current->prev;
            }
            break;

        case ARROW_DOWN:
            if (cy < line_count - 1) {
                cy++;
                current = current->next;
            }
            break;

        case ARROW_LEFT:
            if (cx > 0) cx--;
            break;

        case ARROW_RIGHT:
            if (cx < (int)strlen(current->line)) cx++;
            break;

        case PAGE_UP: {
            int steps = VIEW_HEIGHT;
            while (steps > 0 && cy > 0) {
                cy--;
                current = current->prev;
                steps--;
            }
            break;
        }

        case PAGE_DOWN: {
            int steps = VIEW_HEIGHT;
            while (steps > 0 && cy < line_count - 1) {
                cy++;
                current = current->next;
                steps--;
            }
            break;
        }
    }

    if (cx > (int)strlen(current->line)) {
        cx = strlen(current->line);
    }

    if (cy < row_offset)
        row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT)
        row_offset = cy - VIEW_HEIGHT + 1;
}

void insertChar(char c) {
    int len = strlen(current->line);
    int i;

    if (len >= MAX_LENGTH - 1) return;

    for (i = len; i >= cx; i--) {
        current->line[i + 1] = current->line[i];
    }

    current->line[cx] = c;
    cx++;
}

void insertNewLine() {
    Node *newNode = createNode();

    strcpy(newNode->line, current->line + cx);
    current->line[cx] = '\0';

    newNode->next = current->next;
    newNode->prev = current;

    if (current->next != NULL) {
        current->next->prev = newNode;
    }
    current->next = newNode;

    line_count++;
    cy++;
    current = newNode;
    cx = 0;

    if (cy >= row_offset + VIEW_HEIGHT) {
        row_offset = cy - VIEW_HEIGHT + 1;
    }
}

void deleteChar() {
    int len = strlen(current->line);
    int i;

    if (cx > 0) {
        for (i = cx - 1; i < len; i++) {
            current->line[i] = current->line[i + 1];
        }
        cx--;

    } else if (cy > 0) {
        Node *prevNode = current->prev;
        int   prevLen  = strlen(prevNode->line);

        if (prevLen + len < MAX_LENGTH - 1) {
            cx = prevLen;
            strcat(prevNode->line, current->line);

            prevNode->next = current->next;
            if (current->next != NULL) {
                current->next->prev = prevNode;
            }

            free(current);
            current = prevNode;
            cy--;
            line_count--;

            if (cy < row_offset) row_offset = cy;
        }
    }
}

int confirmQuit() {
    char choice;

    printf("\n===== KELUAR PROGRAM =====\n\n");
    printf("Yakin ingin keluar?\n\n");
    printf("  1. Yes         - Keluar TANPA simpan\n");
    printf("  2. Save First - Simpan DULU, lalu keluar\n");
    printf("  3. Cancel     - Batalkan\n");
    printf("\nPilihan [1/2/3]: ");

    choice = getch();
    printf("%c\n", choice);

    if (choice == '1') {
        return 1;
    } else if (choice == '2') {
        saveFile();
        return 1;
    }

    return 0;
}

int main() {
    int key;

    setCursorVisibility(0);

    head    = createNode();
    current = head;

    while (1) {
        editorRefreshScreen();
        key = readKey();

        if (mode == 0) {
            if (key == 27) {
                mode = 1;
            } else if (key == 13) {
                insertNewLine();
            } else if (key == 8) {
                deleteChar();
            } else if (key >= 32 && key <= 126) {
                insertChar((char)key);
            } else {
                moveCursor(key);
            }

        } else {
            switch (key) {
                case 'i': case 'I': 
                    mode = 0; 
                    system("cls"); // Reset layar sekali agar koordinat kembali (0,0) bersih
                    break;
                case 'o': case 'O': 
                    openFile(); 
                    mode = 0;  
                    system("cls"); // Reset layar total setelah sukses open file
                    break;
                case 's': case 'S': 
                    saveFile(); 
                    system("cls"); // Reset layar total setelah sukses save file
                    break;
                case 'a': case 'A': 
                    saveAs();   
                    system("cls"); // Reset layar total setelah sukses save as
                    break;
                case 'c': case 'C': 
                    closeFile();
                    system("cls"); // Reset layar total setelah sukses close file
                    break;
                case 'q': case 'Q':
                    if (confirmQuit()) {
                        // Munculkan kembali kursor asli sebelum benar-benar keluar ke CMD/Terminal
                        setCursorVisibility(1); 
                        return 0;
                    }
                    system("cls"); // Reset layar jika user membatalkan quit
                    break;
            }
        }
    }

    return 0;
}