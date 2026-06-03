#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════
   KONSTANTA
   ═══════════════════════════════════════════════════════════════ */
#define VIEW_HEIGHT  20   /* Jumlah baris yang tampil di layar */
#define INIT_CAP     64   /* Kapasitas awal buffer tiap baris */

/* ═══════════════════════════════════════════════════════════════
   STRUKTUR: NODE BARIS  (Double Linked List)

   Setiap baris teks = satu node. Node mengenal tetangga atas
   (prev) dan bawah (next) sehingga kursor bisa bergerak dua arah.

   NULL <- [Baris 0] <-> [Baris 1] <-> [Baris 2] -> NULL
              ^head                        ^tail
   ═══════════════════════════════════════════════════════════════ */
typedef struct Line {
    char        *data;   /* Isi teks baris — buffer dinamis */
    int          len;    /* Panjang teks saat ini */
    int          cap;    /* Ukuran buffer yang dialokasi */
    struct Line *next;   /* Baris di bawah */
    struct Line *prev;   /* Baris di atas */
} Line;

/* ═══════════════════════════════════════════════════════════════
   STRUKTUR: SNAPSHOT  (Undo/Redo)

   Setiap snapshot = "foto" seluruh isi editor.
   Snapshot-snapshot tersambung sebagai linked list:

   [Snap0] <-> [Snap1] <-> [Snap2]
                               ^--- history_cursor

   Undo = geser cursor mundur | Redo = geser cursor maju
   ═══════════════════════════════════════════════════════════════ */
typedef struct Snapshot {
    char **lines_data;        /* Salinan isi setiap baris */
    int    line_count;        /* Jumlah baris */
    int    cx, cy;            /* Posisi kursor */
    struct Snapshot *prev;
    struct Snapshot *next;
} Snapshot;

/* ─── Variabel Global ──────────────────────────────────────── */
extern Line     *head;
extern Line     *tail;
extern Snapshot *history_cursor;

extern int  line_count, cx, cy, row_offset, mode;
extern char currentFile[100];

/* ─── list.c : Operasi Linked List ────────────────────────── */
Line *createList(void);                    /* Inisialisasi list kosong */
Line *createNode(void);                    /* Buat satu node baris */
void  insertList(int after_cy, char *txt); /* Sisipkan baris baru */
void  deleteList(int target_cy);           /* Hapus satu baris */
Line *getNode(int index);                  /* Ambil node ke-index */
void  freeList(void);                      /* Bebaskan seluruh list */
void  ensureCap(Line *ln, int needed);     /* Perbesar buffer jika perlu */

/* ─── noel.c : Fitur Editor ───────────────────────────────── */
void pushSnapshot(void);
void undoAction(void);
void redoAction(void);
void copyLineToClipboard(void);
void pasteFromClipboard(void);
void newFile(void);

/* ─── text.c : Operasi Teks ───────────────────────────────── */
void insertChar(char c);
void insertNewLine(void);
void deleteChar(void);

/* ─── main.c : Tampilan ───────────────────────────────────── */
void editorRefreshScreen(void);

/* ─── Utilitas ────────────────────────────────────────────── */
char *str_dup(const char *s);

#endif /* NOEL_H */