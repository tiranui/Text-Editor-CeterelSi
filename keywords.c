/*
 * keywords.c – Baca tombol dan routing ke mode EDIT / COMMAND.
 *
 * Mode EDIT    (mode == 0): mengetik teks biasa
 * Mode COMMAND (mode == 1): menjalankan perintah (Undo, Redo, dst.)
 *
 * PERUBAHAN DARI VERSI LAMA:
 *   - ESC di EDIT  → masuk COMMAND, tampilkan footer perintah
 *   - ESC di COMMAND → kembali ke EDIT, sembunyikan footer
 *   - Perintah lain di COMMAND TIDAK otomatis kembali ke EDIT
 */

#include "keywords.h"

/* ─── Baca Tombol ─────────────────────────────────────────── */

/*
 * readKey – baca satu tombol dari keyboard.
 *
 * Windows mengirim dua kode untuk tombol panah/fungsi:
 *   Kode 1: 0 atau 224 (prefix)
 *   Kode 2: kode scan (72=atas, 80=bawah, 75=kiri, 77=kanan)
 */
int readKey(void) {
    int c = _getch();
    if (c == 0 || c == 224) {
        int c2 = _getch();
        switch (c2) {
            case 72: return ARROW_UP;
            case 80: return ARROW_DOWN;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            default: return c2;
        }
    }
    return c;
}

/* ─── Pembantu Kursor ─────────────────────────────────────── */

/* Pastikan cx tidak melewati panjang baris saat ini */
static void clampCursor(void) {
    int len = getNode(cy)->len;
    if (cx > len) cx = len;
}

/* Sesuaikan row_offset agar baris kursor selalu terlihat */
static void adjustScroll(void) {
    if (cy < row_offset)
        row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT)
        row_offset = cy - VIEW_HEIGHT + 1;
}

/* ─── Mode EDIT ───────────────────────────────────────────── */

void processEditKey(int key) {

    if (key == 27) {          /* ESC → masuk mode COMMAND */
        mode = 1;
        return;
    }
    if (key == 13) {          /* Enter */
        insertNewLine();
        return;
    }
    if (key == 8) {           /* Backspace */
        deleteChar();
        return;
    }
    if (key >= 32 && key <= 126) { /* Karakter yang bisa diketik */
        insertChar((char)key);
        return;
    }

    /* Tombol panah */
    switch (key) {
        case ARROW_UP:
            if (cy > 0) { cy--; clampCursor(); }
            break;
        case ARROW_DOWN:
            if (cy < line_count - 1) { cy++; clampCursor(); }
            break;
        case ARROW_LEFT:
            if (cx > 0) {
                cx--;
            } else if (cy > 0) {
                cy--;
                cx = getNode(cy)->len;
            }
            break;
        case ARROW_RIGHT: {
            int len = getNode(cy)->len;
            if (cx < len) {
                cx++;
            } else if (cy < line_count - 1) {
                cy++;
                cx = 0;
            }
            break;
        }
        default: break;
    }

    adjustScroll();
}

/* ─── Mode COMMAND ────────────────────────────────────────── */

/*
 * processCommandKey – jalankan perintah.
 *
 * BERBEDA dari versi lama: mode TIDAK otomatis kembali ke EDIT.
 * User harus tekan ESC lagi untuk kembali ke EDIT.
 *
 * Ini membuat mode COMMAND seperti "panel perintah" yang bisa
 * digunakan berulang (misal: undo berkali-kali tanpa tekan ESC dulu).
 */
void processCommandKey(int key) {
    switch (key) {
        case 27:              /* ESC → kembali ke EDIT */
            mode = 0;
            break;

        case 'z': case 'Z':  undoAction();           break;
        case 'r': case 'R':  redoAction();           break;
        case 'c': case 'C':  copyLineToClipboard();  break;
        case 'v': case 'V':  pasteFromClipboard();   break;
        case 'n': case 'N':  newFile();              break;

        case 'q': case 'Q':
            printf("\x1b[?25h");
            system("cls");
            exit(0);
            break;

        default: break; /* Tombol lain diabaikan, tetap di COMMAND */
    }
}

/* ─── Dispatcher ──────────────────────────────────────────── */

void processKey(int key) {
    if (mode == 0)
        processEditKey(key);
    else
        processCommandKey(key);
}