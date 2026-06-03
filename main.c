/*
 * main.c – Tampilan editor dan entry point program.
 *
 * Layar dibagi tiga zona:
 * ┌────────────────────────┐
 * │ HEADER (2 baris)       │  judul + info file/kursor
 * ├────────────────────────┤
 * │ AREA TEKS (20 baris)   │  isi editor
 * ├────────────────────────┤
 * │ FOOTER (muncul di      │  hanya tampil saat mode COMMAND
 * │ COMMAND mode saja)     │
 * └────────────────────────┘
 */

#include "noel.h"
#include "keywords.h"

/* ─── Variabel Global ─────────────────────────────────────── */
int  line_count    = 1;
int  cx            = 0;
int  cy            = 0;
int  row_offset    = 0;
int  mode          = 0;
char currentFile[100] = "";

/* Posisi baris footer di terminal (header + teks + 1 spasi) */
#define HEADER_ROWS  3
#define FOOTER_ROW   (HEADER_ROWS + VIEW_HEIGHT + 1)

/* ─── Tampilan ────────────────────────────────────────────── */

/*
 * editorRefreshScreen – gambar ulang seluruh layar.
 *
 * Anti-flicker: kumpulkan semua output ke screen_buf,
 * cetak sekaligus dengan SATU printf.
 *
 * Footer HANYA ditampilkan saat mode == 1 (COMMAND).
 * Saat mode EDIT, baris footer dikosongkan ("\x1b[K").
 */
void editorRefreshScreen(void) {
    static char screen_buf[65536];
    char tmp[1024];

    screen_buf[0] = '\0';

    /* Sembunyikan kursor terminal selama render (cegah kedip) */
    strcat(screen_buf, "\x1b[?25l");

    /* Pindah ke pojok kiri atas */
    strcat(screen_buf, "\x1b[H");

    /* ── Header ── */
    sprintf(tmp, "=== NOTEPAD LINKED LIST === | MODE: %s\x1b[K\n",
            mode == 0 ? "EDIT" : "COMMAND");
    strcat(screen_buf, tmp);

    sprintf(tmp, "File: [%s] | Baris: %d, Kolom: %d\x1b[K\n\n",
            currentFile[0] ? currentFile : "Untitled",
            cy + 1, cx + 1);
    strcat(screen_buf, tmp);

    /* ── Area teks ── */
    Line *curr = head;
    for (int i = 0; i < row_offset && curr; i++)
        curr = curr->next;

    for (int i = 0; i < VIEW_HEIGHT; i++) {
        int file_row = i + row_offset;

        if (file_row < line_count && curr) {
            sprintf(tmp, "%3d | ", file_row + 1);
            strcat(screen_buf, tmp);

            /* Cetak karakter + sisipkan kursor '|' di posisi cx */
            int len = curr->len;
            for (int j = 0; j <= len; j++) {
                if (file_row == cy && j == cx)
                    strcat(screen_buf, "|");
                if (j < len) {
                    tmp[0] = curr->data[j];
                    tmp[1] = '\0';
                    strcat(screen_buf, tmp);
                }
            }
            strcat(screen_buf, "\x1b[K\n");
            curr = curr->next;
        } else {
            strcat(screen_buf, "~\x1b[K\n");
        }
    }

    /* ── Footer – hanya tampil di mode COMMAND ──────────────
     * Pindah ke baris tetap dengan "\x1b[N;1H".
     * Saat EDIT: baris ini dikosongkan saja.
     * ──────────────────────────────────────────────────────── */
    sprintf(tmp, "\x1b[%d;1H", FOOTER_ROW);
    strcat(screen_buf, tmp);

    if (mode == 1) {
        /* Tampilkan panel perintah dengan highlight */
        strcat(screen_buf,
               "\x1b[7m"
               " [ESC] Kembali Edit | [Z] Undo  [R] Redo  "
               "[C] Copy  [V] Paste  [N] Baru  [Q] Keluar "
               "\x1b[0m\x1b[K");
    } else {
        /* EDIT mode: kosongkan baris footer */
        strcat(screen_buf, "\x1b[K");
    }

    printf("%s", screen_buf);
    printf("\x1b[?25h"); /* Tampilkan kembali kursor terminal */
    fflush(stdout);
}

/* ─── Main ────────────────────────────────────────────────── */

int main(void) {
    /* Aktifkan VT escape codes di Windows terminal */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    printf("\x1b[?25l");
    system("cls");

    /* Inisialisasi: buat linked list dengan satu baris kosong */
    createList();

    /* Simpan snapshot awal */
    pushSnapshot();

    /* Loop utama: gambar → baca tombol → proses */
    while (1) {
        editorRefreshScreen();
        int key = readKey();
        processKey(key);
    }

    return 0;
}