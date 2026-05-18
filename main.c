#include "noel.h"
#include "keywords.h"

/* ─── Definisi variabel global (main.c) ─────────────────────────── */
int  line_count = 1;
int  cx = 0, cy = 0;
int  row_offset = 0;
int  mode = 0;          /* 0 = EDIT, 1 = COMMAND */
char currentFile[100] = "";

/* ═══════════════════════════════════════════════════════════════════
   ENTRY POINT
   ═══════════════════════════════════════════════════════════════════ */
int main(void) {
    /* Sembunyikan kursor terminal agar render tidak berkedip */
    printf("\x1b[?25l");
    system("cls");

    /* ── Inisialisasi baris pertama (kosong) ────────────────── */
    head = (Line *)malloc(sizeof(Line));
    head->data[0] = '\0';
    head->next    = NULL;
    head->prev    = NULL;
    tail = head;

    /* Snapshot awal agar Undo tidak langsung "kehabisan" state */
    pushUndo();

    /* ── Event loop utama ───────────────────────────────────── */
    while (1) {
        editorRefreshScreen();
        int key = readKey();
        processKey(key);
    }

    /* Tidak akan pernah dicapai; program keluar melalui processKey → exit() */
    return 0;
}