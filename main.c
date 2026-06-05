#include "Fauzan.h"
#include "Rafli.h"
#include "noel.h"

Line *head        = NULL;
Line *tail        = NULL;
Line *cursor_line = NULL;

int   line_count = 1;
int   cx = 0, cy = 0;
int   row_offset = 0;
int   mode       = 0;

char currentFile[100] = "";

enum keys {
    ARROW_UP    = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};

Line* createLine() {
    Line *node = (Line*)malloc(sizeof(Line));
    if (node == NULL) {
        printf("ERROR: Gagal alokasi memori!\n");
        exit(1);
    }
    node->data[0] = '\0';
    node->prev    = NULL;
    node->next    = NULL;
    return node;
}

Line* getLine(int n) {
    Line *curr = head;
    int   i;
    for (i = 0; i < n && curr != NULL; i++) {
        curr = curr->next;
    }
    return curr;
}

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
    printf("\033[H");
}

void printLineWithCursor(Line *line, int fileRow) {
    int i;
    int len = (int)strlen(line->data);

    printf("\033[K  ");

    for (i = 0; i < len; i++) {
        if (fileRow == cy && i == cx) {
            printf("|");
        }
        printf("%c", line->data[i]);
    }

    if (fileRow == cy && cx == len) {
        printf("|");
    }

    printf("\n");
}

void editorRefreshScreen() {
    int   i;
    Line *line;

    printf("\033[?25l");
    clearScreen();

    printf("\033[K===== MINI NOTEPAD =====\n");
    printf("\033[KFile: %s\n", strlen(currentFile) ? currentFile : "(None)");
    printf("\033[KMODE: %s\n", mode == 0 ? "[EDIT]" : "[COMMAND]");
    printf("\033[K\n\033[K========================\n");

    if (mode == 0) {
        printf("\033[KESC = Command Mode\n");
    } else {
        /* Memperbarui petunjuk menu agar mencakup semua fitur baru */
        printf("\033[K[I]:Edit   [O]:Open   [S]:Save   [A]:SaveAs   [C]:Close\n");
        printf("\033[K[F]:Find   [R]:Repl   [U]:Undo   [Y]:Redo     [N]:New\n");
        printf("\033[K[B]:Copy   [P]:Paste  [Q]:Quit\n");
    }

    printf("\033[K----------------------------------\n");

    line = getLine(row_offset);

    for (i = 0; i < VIEW_HEIGHT; i++) {
        int fileRow = i + row_offset;

        if (fileRow >= line_count || line == NULL) {
            printf("\033[K\n");
        } else {
            printLineWithCursor(line, fileRow);
            line = line->next;
        }
    }

    printf("\033[J");
    fflush(stdout);
}

void moveCursor(int key) {
    int i, steps;

    switch (key) {
        case ARROW_UP:
            if (cursor_line->prev != NULL) {
                cursor_line = cursor_line->prev;
                cy--;
            }
            break;

        case ARROW_DOWN:
            if (cursor_line->next != NULL) {
                cursor_line = cursor_line->next;
                cy++;
            }
            break;

        case ARROW_LEFT:
            if (cx > 0) cx--;
            break;

        case ARROW_RIGHT:
            if (cx < (int)strlen(cursor_line->data)) cx++;
            break;

        case PAGE_UP:
            steps = VIEW_HEIGHT;
            for (i = 0; i < steps; i++) {
                if (cursor_line->prev != NULL) {
                    cursor_line = cursor_line->prev;
                    cy--;
                } else break;
            }
            if (cy < 0) cy = 0;
            break;

        case PAGE_DOWN:
            steps = VIEW_HEIGHT;
            for (i = 0; i < steps; i++) {
                if (cursor_line->next != NULL) {
                    cursor_line = cursor_line->next;
                    cy++;
                } else break;
            }
            if (cy >= line_count) cy = line_count - 1;
            break;
    }

    if (cx > (int)strlen(cursor_line->data)) {
        cx = (int)strlen(cursor_line->data);
    }

    if (cy < row_offset) row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT) {
        row_offset = cy - VIEW_HEIGHT + 1;
    }
}

void insertChar(char c) {
    int len = (int)strlen(cursor_line->data);
    int i;

    if (len >= MAX_LENGTH - 1) return;

    for (i = len; i >= cx; i--) {
        cursor_line->data[i + 1] = cursor_line->data[i];
    }

    cursor_line->data[cx] = c;
    cx++;
    cursor_line->data[len + 1] = '\0';
}

void insertNewLine() {
    Line *newNode;
    Line *nextNode;

    if (line_count >= MAX_LINES) return;

    newNode  = createLine();
    nextNode = cursor_line->next;
    strcpy(newNode->data, cursor_line->data + cx);
    cursor_line->data[cx] = '\0';
    newNode->prev     = cursor_line;
    newNode->next     = nextNode;
    cursor_line->next = newNode;

    if (nextNode != NULL) {
        nextNode->prev = newNode;
    } else {
        tail = newNode;
    }

    line_count++;
    cursor_line = newNode;
    cy++;
    cx = 0;
}

void mergeWithPrevLine() {
    Line *prevNode = cursor_line->prev;
    Line *nextNode = cursor_line->next;
    int   prevLen;

    if (prevNode == NULL) return;
    prevLen = (int)strlen(prevNode->data);

    if (prevLen + (int)strlen(cursor_line->data) >= MAX_LENGTH) return;

    strcat(prevNode->data, cursor_line->data);
    prevNode->next = nextNode;
    if (nextNode != NULL) {
        nextNode->prev = prevNode;
    } else {
        tail = prevNode;
    }
    free(cursor_line);

    cursor_line = prevNode;
    cx          = prevLen;
    cy--;
    line_count--;

    if (cy < row_offset) row_offset = cy;
}

void deleteChar() {
    int len = (int)strlen(cursor_line->data);
    int i;

    if (cx == 0) {
        mergeWithPrevLine();
    } else {
        for (i = cx - 1; i < len; i++) {
            cursor_line->data[i] = cursor_line->data[i + 1];
        }
        cx--;
        cursor_line->data[len - 1] = '\0';
    }
}

int main() {
    int key;
    head        = createLine();
    tail        = head;
    cursor_line = head;
    line_count  = 1;

    // Simpan state awal kosong agar bisa undo sampai bersih kembali
    saveState();

    while (1) {
        editorRefreshScreen();
        key = readKey();

        if (mode == 0) {
            /* ------------------ EDIT MODE ------------------ */
            if (key == 27) {
                mode = 1;
            }
            else if (key == 13) {
                saveState(); // Simpan riwayat sebelum membuat baris baru
                insertNewLine();
            }
            else if (key == 8) {
                saveState(); // Simpan riwayat sebelum menghapus
                deleteChar();
            }
            else if (key >= 32 && key <= 126) {
                saveState(); // Simpan riwayat sebelum menyisipkan karakter
                insertChar((char)key);
            }
            else {
                moveCursor(key);
            }

        } else {
            /* ----------------- COMMAND MODE ----------------- */
            switch (key) {
                case 'i': case 'I':
                    mode = 0;
                    break;

                /* Fitur File Tambahan dari Rafli.h */
                case 'o': case 'O':
                    openFile();
                    mode = 0; // Otomatis kembali ke Edit Mode setelah buka file
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

                /* Fitur Pencarian dari Fauzan.h */
                case 'f': case 'F':
                    findText();
                    break;

                case 'r': case 'R':
                    saveState(); // Simpan state sebelum replace teks massal
                    replaceText();
                    break;

                /* Fitur Riwayat & Clipboard dari noel.h */
                case 'n': case 'N':
                    clearEditorToNewFile();
                    break;

                case 'u': case 'U':
                    undo();
                    break;

                case 'y': case 'Y':
                    redo();
                    break;

                case 'b': case 'B': // Tombol B dipilih untuk fungsi Copy Clipboard
                    copyToClipboard();
                    break;

                case 'p': case 'P':
                    saveState(); // Simpan state sebelum menempelkan teks baru
                    pasteFromClipboard();
                    break;

                case 'q': case 'Q':
                    printf("\033[?25h");
                    return 0;
                    break;
            }
        }
    }
    return 0;
}