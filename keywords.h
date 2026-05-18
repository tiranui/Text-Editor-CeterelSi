#ifndef KEYSWORDS_H
#define KEYSWORDS_H

#include "noel.h"

/* ─── Kode Tombol Khusus ─────────────────────────────────────────── */
/*
 * Nilai di atas 127 tidak bentrok dengan karakter ASCII biasa sehingga
 * aman digunakan sebagai sentinel untuk tombol arah pada Windows.
 */
typedef enum {
    ARROW_UP    = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT
} SpecialKey;

/* ─── Prototipe ──────────────────────────────────────────────────── */

/*
 * readKey – baca satu penekanan tombol dari keyboard.
 * Mengembalikan nilai ASCII untuk karakter biasa, atau salah satu
 * konstanta SpecialKey untuk tombol arah / fungsi.
 */
int readKey(void);

/*
 * processEditKey – tangani tombol dalam MODE EDIT (mode == 0).
 *   - Karakter printable  → insertChar
 *   - Enter               → insertNewLine
 *   - Backspace           → deleteChar
 *   - Arrow keys          → gerak kursor + scroll otomatis
 *   - ESC                 → switch ke COMMAND mode
 */
void processEditKey(int key);

/*
 * processCommandKey – tangani tombol dalam MODE COMMAND (mode == 1).
 *   - z/Z  Undo          - r/R  Redo
 *   - y/Y  Copy line     - p/P  Paste line
 *   - n/N  New file      - q/Q  Quit
 *   - Tombol lain        → kembali ke EDIT mode (no-op)
 */
void processCommandKey(int key);

/*
 * processKey – dispatcher utama; memanggil processEditKey atau
 * processCommandKey sesuai nilai `mode` saat ini.
 */
void processKey(int key);

#endif /* SHORTCUTS_H */