#include "Rafli.h"
#include "Fauzan.h"
#include "noel.h"

/* Data Teks */
char text[MAX_LINES][MAX_LENGTH];
int line_count = 1;

char currentFile[100] = "";

int cx = 0, cy = 0;
int row_offset = 0;

int mode = 0; // 0 = EDIT, 1 = COMMAND

// ================= KEY =================
enum keys {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};

// ================= INPUT =================
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

// ================= DISPLAY (DIPERBAIKI) =================
void clearScreen() {
    // Pindahkan kursor ke pojok kiri atas tanpa menghapus seluruh frame (mencegah flicker)
    printf("\033[H");
}

void printLineWithCursor(int row) {
    int i;
    int len = strlen(text[row]);

    // \033[K membersihkan sisa teks di kanan kursor jika panjang baris berkurang
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

    // \033[?25l menyembunyikan kursor asli terminal agar tidak ada kursor ganda
    printf("\033[?25l"); 
    clearScreen();

    // Tambahkan \033[K di setiap baris UI agar tidak ada sisa karakter dari frame sebelumnya
    printf("\033[K===== MINI NOTEPAD =====\n");
    printf("\033[KFile: %s\n", strlen(currentFile) ? currentFile : "(None)");
    printf("\033[KMODE: %s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf("\033[K\n\033[K========================\n");

    if (mode == 0) {
        printf("\033[KESC = Command Mode\n");
    } else {
        // UI diupdate untuk menampilkan opsi baru dari noel.c
        printf("\033[KI: Edit  O: Open   S: Save  A: SaveAs  N: New\n");
        printf("\033[KF: Find  R: Rplc   C: Close Q: Quit\n");
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

    // \033[J membersihkan sisa baris terminal di bawah area teks editor
    printf("\033[J"); 

    fflush(stdout);
}

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

    if (cx > (int)strlen(text[cy])) {
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

    // Simpan status sebelum diubah untuk fitur UNDO
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

    // Simpan status sebelum membuat baris baru
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
        // Simpan status sebelum menghapus karakter
        pushUndo();

        for (i = cx - 1; i < len; i++) {
            text[cy][i] = text[cy][i + 1];
        }
        cx--;

        text[cy][len - 1] = '\0';
    }
}

// ================= MAIN =================
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
                case 'i': case 'I':
                    mode = 0;
                    break;

                case 'o': case 'O':
                    openFile();     // Pastikan ini ada di header lain
                    mode = 0;
                    break;

                case 's': case 'S':
                    saveFile();     // Pastikan ini ada di header lain
                    break;

                case 'a': case 'A':
                    saveAs();       // Pastikan ini ada di header lain
                    break;

                case 'f': case 'F':
                    findText();     // Pastikan ini ada di header lain
                    break;

                case 'r': case 'R':
                    replaceText();  // Pastikan ini ada di header lain
                    break;

                case 'c': case 'C':
                    closeFile();    // Pastikan ini ada di header lain
                    break;

                // --- FITUR TAMBAHAN DARI NOEL.C ---
                case 'n': case 'N':
                    newFile();
                    break;

                case 'y': case 'Y': // Menggunakan Y (Yank) untuk Copy, karena C dipakai Close
                    copyLine();
                    break;

                case 'p': case 'P':
                    pasteLine();
                    break;

                case 'u': case 'U':
                    undoAction();
                    break;

                case 'e': case 'E': // Menggunakan E untuk Redo
                    redoAction();
                    break;
                // ----------------------------------

                case 'q': case 'Q':
                    // \033[?25h memunculkan kembali kursor asli terminal sebelum keluar
                    printf("\033[?25h"); 
                    return 0;
            }
        }
    }

    return 0;
}