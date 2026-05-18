#include "keywords.h"

/* ═══════════════════════════════════════════════════════════════════
   BACA TOMBOL
   ═══════════════════════════════════════════════════════════════════ */

/*
 * readKey – pada Windows, _getch() mengembalikan 0 atau 224 sebagai
 * "prefix" untuk tombol khusus (arah, F1-F12, dsb.), diikuti oleh
 * kode scan kedua.  Kita petakan ke konstanta SpecialKey supaya
 * bagian logika tidak perlu tahu detail platform.
 */
int readKey(void) {
    int c = _getch();

    if (c == 0 || c == 224) {          /* prefix tombol khusus */
        int c2 = _getch();
        switch (c2) {
            case 72: return ARROW_UP;
            case 80: return ARROW_DOWN;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            default: return c2;        /* tombol khusus lain diabaikan */
        }
    }
    return c;
}

/* ═══════════════════════════════════════════════════════════════════
   MODE EDIT  (mode == 0)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * _clampCursor – pastikan cx tidak melebihi panjang baris cy.
 * Dipanggil setelah pergerakan vertikal agar kursor tidak "melayang"
 * di luar akhir baris yang lebih pendek.
 */
static void _clampCursor(void) {
    int len = (int)strlen(getLineNode(cy)->data);
    if (cx > len) cx = len;
}

/*
 * _adjustScroll – sesuaikan row_offset agar baris cy selalu
 * berada di dalam jendela tampilan.
 */
static void _adjustScroll(void) {
    if (cy < row_offset)
        row_offset = cy;
    if (cy >= row_offset + VIEW_HEIGHT)
        row_offset = cy - VIEW_HEIGHT + 1;
}

void processEditKey(int key) {
    /* ── Tombol kontrol ─────────────────────────────────────── */
    if (key == 27) {            /* ESC → masuk COMMAND mode */
        mode = 1;
        return;
    }
    if (key == 13) {            /* Enter */
        insertNewLine();
        return;
    }
    if (key == 8) {             /* Backspace */
        deleteChar();
        return;
    }

    /* ── Karakter printable ─────────────────────────────────── */
    if (key >= 32 && key <= 126) {
        insertChar((char)key);
        return;
    }

    /* ── Tombol arah ────────────────────────────────────────── */
    switch (key) {
        case ARROW_UP:
            if (cy > 0) { cy--; _clampCursor(); }
            break;

        case ARROW_DOWN:
            if (cy < line_count - 1) { cy++; _clampCursor(); }
            break;

        case ARROW_LEFT:
            if (cx > 0) {
                cx--;
            } else if (cy > 0) {
                /* Lompat ke ujung baris sebelumnya */
                cy--;
                cx = (int)strlen(getLineNode(cy)->data);
            }
            break;

        case ARROW_RIGHT: {
            int len = (int)strlen(getLineNode(cy)->data);
            if (cx < len) {
                cx++;
            } else if (cy < line_count - 1) {
                /* Lompat ke awal baris berikutnya */
                cy++;
                cx = 0;
            }
            break;
        }

        default:
            break;  /* tombol lain diabaikan */
    }

    /* Scroll layar mengikuti posisi kursor */
    _adjustScroll();
}

/* ═══════════════════════════════════════════════════════════════════
   MODE COMMAND  (mode == 1)
   ═══════════════════════════════════════════════════════════════════ */

void processCommandKey(int key) {
    switch (key) {
        case 'z': case 'Z':  undoAction();  break;
        case 'r': case 'R':  redoAction();  break;
        case 'y': case 'Y':  copyLine();    break;
        case 'p': case 'P':  pasteLine();   break;
        case 'n': case 'N':  newFile();     break;

        case 'q': case 'Q':
            printf("\x1b[?25h");    /* tampilkan kembali kursor terminal */
            system("cls");
            exit(0);
            break;                  /* tidak pernah dicapai, tapi baik untuk kejelasan */

        default:
            break;                  /* tombol tak dikenal → diam */
    }

    /* Setiap perintah mengembalikan editor ke EDIT mode */
    mode = 0;
}

/* ═══════════════════════════════════════════════════════════════════
   DISPATCHER UTAMA
   ═══════════════════════════════════════════════════════════════════ */

void processKey(int key) {
    if (mode == 0)
        processEditKey(key);
    else
        processCommandKey(key);
}