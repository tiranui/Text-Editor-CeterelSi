#include "Rafli.h"


Node *head        = NULL;
Node *current     = NULL;
int   line_count  = 1;
char  currentFile[100] = "";
int   cx = 0, cy = 0;
int   row_offset  = 0;
int   mode        = 0;


#define SCR_W  60
#define SCR_H  40


static CHAR_INFO g_screen[SCR_H * SCR_W];


#define CLR_NORMAL   0x07
#define CLR_BRIGHT   0x0F
#define CLR_DIM      0x08
#define CLR_CURSOR   0x70


static void scrClear(void) {
    int i;
    for (i = 0; i < SCR_H * SCR_W; i++) {
        g_screen[i].Char.AsciiChar = ' ';
        g_screen[i].Attributes     = CLR_NORMAL;
    }
}


static void scrPutStr(int row, int col, const char *s, WORD attr) {
    int idx;
    while (*s && col < SCR_W) {
        idx = row * SCR_W + col;
        g_screen[idx].Char.AsciiChar = *s;
        g_screen[idx].Attributes     = attr;
        s++;
        col++;
    }
}


static void scrPutChar(int row, int col, char c, WORD attr) {
    int idx;
    if (row < 0 || row >= SCR_H || col < 0 || col >= SCR_W) return;
    idx = row * SCR_W + col;
    g_screen[idx].Char.AsciiChar = c;
    g_screen[idx].Attributes     = attr;
}


 
static void scrFlush(void) {
    HANDLE     h   = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD      sz  = { SCR_W,      SCR_H      };  /* ukuran buffer kita   */
    COORD      org = { 0,          0          };  /* mulai baca dari [0,0]*/
    SMALL_RECT reg = { 0, 0, SCR_W-1, SCR_H-1 }; /* tulis ke seluruh layar */
    WriteConsoleOutputA(h, g_screen, sz, org, &reg);
}


enum keys {
    ARROW_UP    = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};

int readKey(void) {
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


void clearScreen(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD  written;
    COORD  home = {0, 0};
    GetConsoleScreenBufferInfo(hOut, &csbi);
    DWORD count = (DWORD)csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hOut, ' ', count, home, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, count, home, &written);
    SetConsoleCursorPosition(hOut, home);
}

void setCursorVisibility(int visible) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO ci;
    GetConsoleCursorInfo(hOut, &ci);
    ci.bVisible = (BOOL)visible;
    SetConsoleCursorInfo(hOut, &ci);
}


void setConsoleSize(void) {
    HANDLE     hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD      buf  = { SCR_W, SCR_H };
    SMALL_RECT win  = { 0, 0, SCR_W - 1, SCR_H - 1 };
    SetConsoleScreenBufferSize(hOut, buf);
    SetConsoleWindowInfo(hOut, TRUE, &win);
}


void editorRefreshScreen(void) {
    int  i;
    char tmp[SCR_W + 2];   /* buffer sprintf, +2 aman dari off-by-one */

    /* -- Kosongkan kanvas virtual -------------------------------- */
    scrClear();

    /* -- Baris 0-6: Header --------------------------------------- */
    scrPutStr(0, 0, "============================================================", CLR_BRIGHT);
    scrPutStr(1, 0, "                    MINI NOTEPAD                            ", CLR_BRIGHT);
    scrPutStr(2, 0, "============================================================", CLR_BRIGHT);

    /* Nama file: potong jika lebih dari 50 karakter */
    sprintf(tmp, " File : %.50s", strlen(currentFile) ? currentFile : "(Belum ada)");
    scrPutStr(3, 0, tmp, CLR_NORMAL);

    sprintf(tmp, " Mode : %s", mode == 0 ? "[EDIT]" : "[COMMAND]");
    scrPutStr(4, 0, tmp, CLR_NORMAL);

    sprintf(tmp, " Brs  : %-5d  |  Kol : %-5d", cy + 1, cx + 1);
    scrPutStr(5, 0, tmp, CLR_NORMAL);

    scrPutStr(6, 0, "============================================================", CLR_BRIGHT);
    /* Baris 7 sengaja kosong sebagai padding */

    /* -- Baris 8–27: Konten teks (VIEW_HEIGHT = 20 baris) -------- */
    for (i = 0; i < VIEW_HEIGHT; i++) {
        int   fileRow   = i + row_offset;
        int   screenRow = 8 + i;
        Node *node;
        int   len, col, j;

        if (fileRow >= line_count) {
            /* Baris melebihi isi file: tampilkan ~ */
            scrPutChar(screenRow, 0, '~', CLR_DIM);
            continue;
        }

        node = getNodeAt(fileRow);
        if (node == NULL) continue;

        len = (int)strlen(node->line);
        col = 0;

        /* Nomor baris: "  N | " dengan warna dim */
        sprintf(tmp, "%3d | ", fileRow + 1);
        scrPutStr(screenRow, 0, tmp, CLR_DIM);
        col = 6;

        /* Karakter teks satu per satu */
        for (j = 0; j <= len; j++) {
            if (col >= SCR_W) break;

            if (fileRow == cy && j == cx) {
                scrPutChar(screenRow, col, '|', CLR_CURSOR);
                col++;
                if (col >= SCR_W) break;
            }

            /* Karakter teks biasa */
            if (j < len) {
                scrPutChar(screenRow, col, node->line[j], CLR_NORMAL);
                col++;
            }
        }
    }

    
    if (mode == 0) {
        scrPutStr(29, 0, " [ESC] Masuk Command Mode", CLR_NORMAL);
    } else {
        scrPutStr(29, 0, "============================================================", CLR_BRIGHT);
        scrPutStr(30, 0, " [I] Edit       [O] Buka File      [S] Simpan", CLR_NORMAL);
        scrPutStr(31, 0, " [A] Simpan Sebagai                [C] Tutup File", CLR_NORMAL);
        scrPutStr(32, 0, " [Q] Keluar", CLR_NORMAL);
        scrPutStr(33, 0, "============================================================", CLR_BRIGHT);
    }

    
    scrFlush();

    
    if (mode == 0) {
        COORD pos = {
            (SHORT)(6 + cx),
            (SHORT)(8 + (cy - row_offset))
        };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    }
}

/* ================================================================
   PERGERAKAN KURSOR
   ================================================================ */
void moveCursor(int key) {
    switch (key) {
        case ARROW_UP:
            if (cy > 0) { cy--; current = current->prev; }
            break;

        case ARROW_DOWN:
            if (cy < line_count - 1) { cy++; current = current->next; }
            break;

        case ARROW_LEFT:
            if (cx > 0) cx--;
            break;

        case ARROW_RIGHT:
            if (cx < (int)strlen(current->line)) cx++;
            break;

        case PAGE_UP: {
            int steps = VIEW_HEIGHT;
            while (steps-- > 0 && cy > 0) {
                cy--;
                current = current->prev;
            }
            break;
        }

        case PAGE_DOWN: {
            int steps = VIEW_HEIGHT;
            while (steps-- > 0 && cy < line_count - 1) {
                cy++;
                current = current->next;
            }
            break;
        }
    }

    if (cx > (int)strlen(current->line))
        cx = (int)strlen(current->line);

    if (cy < row_offset)
        row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT)
        row_offset = cy - VIEW_HEIGHT + 1;
}

/* ================================================================
   EDIT TEKS
   ================================================================ */
void insertChar(char c) {
    int len = (int)strlen(current->line);
    int i;

    if (len >= MAX_LENGTH - 1) return;

    for (i = len; i >= cx; i--)
        current->line[i + 1] = current->line[i];

    current->line[cx] = c;
    cx++;
}

void insertNewLine(void) {
    Node *newNode = createNode();

    strcpy(newNode->line, current->line + cx);
    current->line[cx] = '\0';

    newNode->next = current->next;
    newNode->prev = current;

    if (current->next != NULL)
        current->next->prev = newNode;

    current->next = newNode;

    line_count++;
    cy++;
    current = newNode;
    cx = 0;

    if (cy >= row_offset + VIEW_HEIGHT)
        row_offset = cy - VIEW_HEIGHT + 1;
}

void deleteChar(void) {
    int len = (int)strlen(current->line);
    int i;

    if (cx > 0) {
        for (i = cx - 1; i < len; i++)
            current->line[i] = current->line[i + 1];
        cx--;

    } else if (cy > 0) {
        Node *prev    = current->prev;
        int   prevLen = (int)strlen(prev->line);

        if (prevLen + len < MAX_LENGTH - 1) {
            cx = prevLen;
            strcat(prev->line, current->line);

            prev->next = current->next;
            if (current->next != NULL)
                current->next->prev = prev;

            free(current);
            current = prev;
            cy--;
            line_count--;

            if (cy < row_offset) row_offset = cy;
        }
    }
}

/* ================================================================
   KONFIRMASI QUIT
   ================================================================ */
int confirmQuit(void) {
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

    if (choice == '1') return 1;
    if (choice == '2') { saveFile(); return 1; }

    setCursorVisibility(0);
    return 0;
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {
    int key;

    setConsoleSize();
    setCursorVisibility(0);

    head    = createNode();
    current = head;

    while (1) {
        editorRefreshScreen();
        key = readKey();

        if (mode == 0) {
            if      (key == 27)               mode = 1;
            else if (key == 13)               insertNewLine();
            else if (key == 8)                deleteChar();
            else if (key >= 32 && key <= 126) insertChar((char)key);
            else                              moveCursor(key);

        } else {
            switch (key) {
                case 'i': case 'I':  mode = 0;              break;
                case 'o': case 'O':  openFile(); mode = 0;  break;
                case 's': case 'S':  saveFile();             break;
                case 'a': case 'A':  saveAs();               break;
                case 'c': case 'C':  closeFile();            break;
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