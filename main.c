#include "Rafli.h"

Node *head        = NULL;
Node *current     = NULL;
int   line_count  = 1;
char  currentFile[100] = "";
int   cx = 0, cy = 0;
int   row_offset  = 0;
int   mode        = 0;

enum keys {
    ARROW_UP    = 1000,
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

/*
 * FIX BUG 1 - FLICKERING:
 * clearScreen() versi lama hanya MEMINDAHKAN kursor ke (0,0) tanpa
 * benar-benar menghapus layar. Akibatnya teks lama masih ada di buffer
 * dan tampilan berkedip saat teks baru ditulis di atasnya.
 *
 * Solusi: gunakan FillConsoleOutputCharacter + FillConsoleOutputAttribute
 * untuk menghapus SELURUH isi buffer layar secara sempurna sebelum
 * menggambar ulang, lalu posisikan kursor ke (0,0).
 */
void clearScreen() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD jumlahKarakter, jumlahDitulis;
    COORD posisiAwal = {0, 0};

    GetConsoleScreenBufferInfo(hOut, &csbi);
    jumlahKarakter = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hOut, ' ', jumlahKarakter, posisiAwal, &jumlahDitulis);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, jumlahKarakter, posisiAwal, &jumlahDitulis);
    SetConsoleCursorPosition(hOut, posisiAwal);
}

void setCursorVisibility(int visible) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

/*
 * FIX BUG 3 - UKURAN CONSOLE:
 * Set ukuran console buffer agar tampilan tidak terpotong
 * dan scroll bar tidak muncul yang bisa menggeser koordinat.
 */
void setConsoleSize() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = {0, 0, 59, 39};
    COORD bufferSize      = {60, 40};
    SetConsoleScreenBufferSize(hOut, bufferSize);
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
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
    printf("\n");
}

void editorRefreshScreen() {
    int i;

    clearScreen();

    printf("========================================\n");
    printf("           MINI NOTEPAD                 \n");
    printf("========================================\n");
    printf(" File : %-36s\n", strlen(currentFile) ? currentFile : "(Belum ada)");
    printf(" Mode : %-36s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf(" Brs  : %-5d  |  Kol : %-5d\n", cy + 1, cx + 1);
    printf("========================================\n\n");

    for (i = 0; i < VIEW_HEIGHT; i++) {
        int fileRow = i + row_offset;
        if (fileRow >= line_count) {
            printf("~\n");
        } else {
            printLineWithCursor(fileRow);
        }
    }

    printf("\n");
    if (mode == 0) {
        printf("[ESC] Masuk Command Mode\n");
    } else {
        printf("========================================\n");
        printf(" [I]Edit  [O]Buka  [S]Simpan\n");
        printf(" [A]SimpanSebagai [C]Tutup\n");
        printf(" [Q]Keluar\n");
        printf("========================================\n");
    }

    /*
     * FIX BUG 2 - POSISI KURSOR:
     * Hitung posisi kursor berdasarkan jumlah baris header yang tetap:
     * Baris 0-2  : header (3 baris ===)
     * Baris 3-5  : info File/Mode/Brs
     * Baris 6    : === bawah
     * Baris 7    : baris kosong (dari \n)
     * Baris 8+   : konten teks (VIEW_HEIGHT baris)
     *
     * cx + 6 karena "  1 | " = 6 karakter di depan teks
     */
    if (mode == 0) {
        COORD posKursor = {6 + cx, 8 + (cy - row_offset)};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posKursor);
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

/*
 * FIX BUG 4 - CONFIRMQUIT MENIMPA LAYAR EDITOR:
 * Sebelumnya confirmQuit() langsung printf tanpa membersihkan layar,
 * sehingga teks menu quit menimpa teks editor yang ada.
 * Solusi: panggil system("cls") dulu sebelum menampilkan menu.
 */
int confirmQuit() {
    char choice;

    system("cls");
    setCursorVisibility(1);

    printf("===== KELUAR PROGRAM =====\n\n");
    printf("Yakin ingin keluar?\n\n");
    printf("  1. Yes        - Keluar TANPA simpan\n");
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

    setCursorVisibility(0);
    return 0;
}

int main() {
    setConsoleSize();
    setCursorVisibility(0);

    head    = createNode();
    current = head;

    while (1) {
        editorRefreshScreen();
        int key = readKey();

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
                    break;

                case 'o': case 'O':
                    openFile();
                    mode = 0;
                    break;

                case 's': case 'S':
                    saveFile();
                    break;

                case 'a': case 'A':
                    saveAs();
                    break;

                case 'c': case 'C':
                    closeFile();
                    break;

                case 'q': case 'Q':
                    if (confirmQuit()) {
                        setCursorVisibility(1);
                        return 0;
                    }
                    break;
            }
        }
    }

    return 0;
}