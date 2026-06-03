/*
 * noel.c – Fitur utama editor: Undo/Redo, Copy, Paste, New File.
 *
 * File ini TIDAK mengurus linked list secara langsung.
 * Semua operasi list dilakukan lewat fungsi di list.c.
 */

#include "noel.h"

/* Variabel global — didefinisikan di sini, extern di noel.h */
Line     *head           = NULL;
Line     *tail           = NULL;
Snapshot *history_cursor = NULL;

/* ═══════════════════════════════════════════════════════════════
   UNDO / REDO
   ═══════════════════════════════════════════════════════════════ */

/*
 * freeRedoChain – hapus semua snapshot di depan history_cursor.
 *
 * Dipanggil saat user mengetik sesuatu setelah undo —
 * "masa depan" lama tidak berlaku lagi, jadi kita buang.
 *
 * Sebelum: [S0] <-> [S1] <-> [S2] <-> [S3]
 *                      ^cursor
 * Sesudah: [S0] <-> [S1]
 *                      ^cursor  (S2 dan S3 dibebaskan)
 */
static void freeRedoChain(void) {
    if (!history_cursor || !history_cursor->next) return;

    Snapshot *curr = history_cursor->next;
    while (curr != NULL) {
        Snapshot *next = curr->next;
        for (int i = 0; i < curr->line_count; i++)
            free(curr->lines_data[i]);
        free(curr->lines_data);
        free(curr);
        curr = next;
    }
    history_cursor->next = NULL;
}

/*
 * pushSnapshot – simpan "foto" kondisi editor sekarang.
 *
 * Kapan dipanggil:
 *   - Saat user menekan SPASI (setiap kata baru = unit undo)
 *   - Saat user menekan ENTER
 *   - Saat user menekan BACKSPACE
 *   - Sebelum Paste dan New File
 */
void pushSnapshot(void) {
    Snapshot *snap = (Snapshot *)malloc(sizeof(Snapshot));
    if (!snap) return;

    /* Simpan posisi kursor dan jumlah baris */
    snap->line_count = line_count;
    snap->cx         = cx;
    snap->cy         = cy;

    /* Salin isi setiap baris ke dalam snapshot */
    snap->lines_data = (char **)malloc(line_count * sizeof(char *));
    for (int i = 0; i < line_count; i++)
        snap->lines_data[i] = str_dup(getNode(i)->data);

    /* Buang redo chain (tidak bisa redo setelah aksi baru) */
    freeRedoChain();

    /* Sambungkan snapshot baru ke rantai */
    snap->prev = history_cursor;
    snap->next = NULL;
    if (history_cursor)
        history_cursor->next = snap;
    history_cursor = snap;
}

/*
 * restoreSnapshot – terapkan isi snapshot ke editor.
 *
 * Cara kerja:
 *   1. Hapus semua baris yang ada sekarang
 *   2. Buat ulang linked list dari data snapshot
 *   3. Restore posisi kursor
 */
static void restoreSnapshot(Snapshot *snap) {
    if (!snap) return;

    /* Hapus semua baris */
    freeList();

    /* Buat ulang linked list dari data snapshot */
    Line *last = NULL;
    for (int i = 0; i < snap->line_count; i++) {
        Line *node = createNode();
        int slen = (int)strlen(snap->lines_data[i]);
        ensureCap(node, slen + 1);
        strcpy(node->data, snap->lines_data[i]);
        node->len  = slen;
        node->prev = last;

        if (head == NULL) head = node;
        else              last->next = node;
        last = node;
    }
    tail = last;

    /* Restore posisi kursor */
    line_count = snap->line_count;
    cx = snap->cx;
    cy = snap->cy;
}

/* undoAction – mundur satu langkah di rantai snapshot */
void undoAction(void) {
    if (!history_cursor || !history_cursor->prev) return;
    history_cursor = history_cursor->prev;
    restoreSnapshot(history_cursor);
}

/* redoAction – maju satu langkah di rantai snapshot */
void redoAction(void) {
    if (!history_cursor || !history_cursor->next) return;
    history_cursor = history_cursor->next;
    restoreSnapshot(history_cursor);
}

/* ═══════════════════════════════════════════════════════════════
   COPY  (ke clipboard Windows)
   ═══════════════════════════════════════════════════════════════ */

/*
 * copyLineToClipboard – salin isi baris saat ini ke clipboard Windows.
 *
 * Setelah ini user bisa Ctrl+V di Notepad, Word, dll.
 *
 * Cara kerja Windows Clipboard API:
 *   1. GlobalAlloc  – alokasi memori global untuk data clipboard
 *   2. GlobalLock   – kunci memori agar bisa ditulis
 *   3. OpenClipboard / EmptyClipboard / SetClipboardData
 *   4. CloseClipboard
 */
void copyLineToClipboard(void) {
    Line *curr = getNode(cy);
    if (!curr) return;

    int len = curr->len + 1; /* +1 untuk null terminator */

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem) return;

    char *buf = (char *)GlobalLock(hMem);
    memcpy(buf, curr->data, len);
    GlobalUnlock(hMem);

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    } else {
        GlobalFree(hMem); /* Gagal buka clipboard — bebaskan memori */
    }
}

/* ═══════════════════════════════════════════════════════════════
   PASTE  (dari clipboard Windows)
   ═══════════════════════════════════════════════════════════════ */

/*
 * pasteFromClipboard – ambil teks dari clipboard dan sisipkan.
 *
 * Karakter '\r' (Windows line ending) dilewati.
 * Karakter '\n' memicu insertNewLine (baris baru).
 * Karakter lain disisipkan langsung tanpa snapshot spasi.
 */
void pasteFromClipboard(void) {
    if (!IsClipboardFormatAvailable(CF_TEXT)) return;
    if (!OpenClipboard(NULL)) return;

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData) {
        char *text = (char *)GlobalLock(hData);
        if (text) {
            pushSnapshot(); /* Simpan kondisi sebelum paste */

            for (int i = 0; text[i] != '\0'; i++) {
                if (text[i] == '\r') {
                    continue;          /* Skip carriage return */
                } else if (text[i] == '\n') {
                    insertNewLine();   /* Buat baris baru */
                } else {
                    /* Sisipkan karakter langsung ke baris aktif */
                    Line *ln = getNode(cy);
                    ensureCap(ln, ln->len + 2);
                    for (int j = ln->len; j >= cx; j--)
                        ln->data[j + 1] = ln->data[j];
                    ln->data[cx] = text[i];
                    ln->len++;
                    cx++;
                }
            }
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
}

/* ═══════════════════════════════════════════════════════════════
   NEW FILE
   ═══════════════════════════════════════════════════════════════ */

/*
 * newFile – bersihkan editor untuk memulai file baru.
 *
 * Kondisi sebelum: editor berisi teks apapun.
 * Kondisi sesudah: satu baris kosong, semua posisi di-reset.
 */
void newFile(void) {
    pushSnapshot(); /* Simpan kondisi sekarang (bisa di-undo) */
    freeList();     /* Hapus semua baris */
    createList();   /* Buat list baru dengan satu baris kosong */

    cx = cy = row_offset = 0;
    currentFile[0] = '\0';
}