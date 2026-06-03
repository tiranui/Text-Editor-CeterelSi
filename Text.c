/*
 * text.c – Operasi teks: insertChar, insertNewLine, deleteChar.
 *
 * Tiga fungsi ini adalah "jembatan" antara penekanan tombol
 * dan perubahan pada linked list baris.
 */

#include "noel.h"

/*
 * insertChar – sisipkan karakter c di posisi kursor (cx, cy).
 *
 * Snapshot dibuat hanya saat c == ' ' (spasi = batas kata).
 *
 * Cara geser karakter ke kanan (contoh: sisip 'X' di cx=2):
 *   Sebelum: H e l l o \0
 *   Indeks:  0 1 2 3 4
 *   Geser:   H e _ l l o \0   (_ = slot kosong yang tersedia)
 *   Isi:     H e X l l o \0
 */
void insertChar(char c) {
    if (c == ' ') pushSnapshot(); /* Snapshot di setiap awal kata */

    Line *ln = getNode(cy);
    ensureCap(ln, ln->len + 2); /* Pastikan ada ruang */

    /* Geser semua karakter setelah kursor ke kanan */
    for (int i = ln->len; i >= cx; i--)
        ln->data[i + 1] = ln->data[i];

    ln->data[cx] = c;
    ln->len++;
    cx++;
}

/*
 * insertNewLine – pecah baris di posisi kursor (tekan Enter).
 *
 * Contoh: baris "Hello World", cx=5
 *   Baris lama    → "Hello"
 *   Baris baru    → " World"
 *
 * Di linked list:
 *   Sebelum: [Hello World]
 *   Sesudah: [Hello] <-> [ World]
 *
 * Gunakan insertList dari list.c untuk sisipkan node baru,
 * lalu potong teks baris lama.
 */
void insertNewLine(void) {
    pushSnapshot();

    Line *curr = getNode(cy);
    char *after = curr->data + cx; /* Teks setelah kursor */

    /* Sisipkan baris baru dengan teks setelah kursor */
    insertList(cy, after);

    /* Potong baris lama sampai posisi kursor */
    curr->data[cx] = '\0';
    curr->len = cx;

    /* Pindah kursor ke awal baris baru */
    cy++;
    cx = 0;
    if (cy >= row_offset + VIEW_HEIGHT) row_offset++;
}

/*
 * deleteChar – hapus karakter sebelum kursor (Backspace).
 *
 * Setiap penghapusan simpan snapshot → undo per karakter.
 *
 * Dua kasus:
 *   Kasus 1 (cx > 0): hapus karakter di sebelah kiri kursor
 *     Contoh: "Hello|" cx=5 → hapus 'o' → "Hell|" cx=4
 *
 *   Kasus 2 (cx == 0, cy > 0): gabungkan dengan baris di atasnya
 *     Baris atas: "Hello"
 *     Baris ini:  " World"   ← kursor di awal
 *     Hasil:      "Hello World"   (baris ini dihapus dari list)
 */
void deleteChar(void) {
    pushSnapshot();

    Line *curr = getNode(cy);

    if (cx > 0) {
        /* Kasus 1: geser karakter ke kiri untuk "menutup" slot */
        for (int i = cx - 1; i < curr->len; i++)
            curr->data[i] = curr->data[i + 1];
        curr->len--;
        cx--;
    } else if (cy > 0) {
        /* Kasus 2: gabungkan baris ini ke baris sebelumnya */
        Line *prev_line = curr->prev;
        int   prev_len  = prev_line->len;

        /* Tambahkan teks baris ini ke akhir baris sebelumnya */
        ensureCap(prev_line, prev_len + curr->len + 1);
        memcpy(prev_line->data + prev_len, curr->data, curr->len + 1);
        prev_line->len += curr->len;

        /* Hapus node baris ini dari linked list */
        deleteList(cy);

        /* Pindah kursor ke posisi sambungan */
        cy--;
        cx = prev_len;
        if (cy < row_offset) row_offset--;
    }
}