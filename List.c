/*
 * list.c – Semua operasi Double Linked List untuk baris teks.
 *
 * File ini HANYA mengurus struktur data linked list:
 *   createList  – inisialisasi list dengan satu baris kosong
 *   createNode  – buat satu node baris kosong
 *   insertList  – sisipkan baris baru setelah baris tertentu
 *   deleteList  – hapus satu baris dari list
 *   getNode     – ambil node ke-index (traversal dari head)
 *   freeList    – bebaskan seluruh memori list
 *   ensureCap   – perbesar buffer teks jika tidak muat
 */

#include "noel.h"

/* ═══════════════════════════════════════════════════════════════
   UTILITAS
   ═══════════════════════════════════════════════════════════════ */

char *str_dup(const char *s) {
    if (!s) return NULL;
    char *copy = (char *)malloc(strlen(s) + 1);
    if (copy) strcpy(copy, s);
    return copy;
}

/*
 * ensureCap – pastikan buffer node punya cukup ruang.
 *
 * Jika kapasitas kurang, gandakan terus sampai cukup.
 * Contoh: cap=64, butuh 100 → cap jadi 128.
 */
void ensureCap(Line *ln, int needed) {
    while (ln->cap <= needed)
        ln->cap *= 2;
    ln->data = (char *)realloc(ln->data, ln->cap);
}

/* ═══════════════════════════════════════════════════════════════
   OPERASI LIST
   ═══════════════════════════════════════════════════════════════ */

/*
 * createNode – alokasi satu node baris kosong.
 *
 * Setiap node punya:
 *   data → buffer teks (mulai INIT_CAP = 64 byte)
 *   len  → 0 (kosong)
 *   cap  → INIT_CAP
 *   next, prev → NULL (belum tersambung)
 */
Line *createNode(void) {
    Line *node = (Line *)malloc(sizeof(Line));
    node->cap    = INIT_CAP;
    node->len    = 0;
    node->data   = (char *)malloc(node->cap);
    node->data[0] = '\0';
    node->next   = NULL;
    node->prev   = NULL;
    return node;
}

/*
 * createList – buat list baru dengan satu baris kosong.
 *
 * Setelah fungsi ini:
 *   head = tail = [node kosong]
 *   line_count = 1
 *
 * Editor tidak boleh benar-benar kosong (0 node),
 * jadi kita mulai dengan satu baris kosong.
 */
Line *createList(void) {
    Line *first = createNode();
    head = first;
    tail = first;
    line_count = 1;
    return first;
}

/*
 * getNode – ambil node ke-index dari linked list.
 *
 * Cara kerja: mulai dari head, ikuti ->next sebanyak 'index' kali.
 *
 * Contoh (index = 2):
 *   head → [Baris 0] → [Baris 1] → [Baris 2]  ← dikembalikan
 */
Line *getNode(int index) {
    Line *curr = head;
    for (int i = 0; i < index && curr != NULL; i++)
        curr = curr->next;
    return curr;
}

/*
 * insertList – sisipkan baris baru SETELAH baris ke-after_cy.
 *
 * Parameter:
 *   after_cy → indeks baris yang akan menjadi tetangga atas
 *   txt      → isi teks untuk baris baru (boleh "")
 *
 * Sebelum (after_cy=1):
 *   [Baris 0] <-> [Baris 1] <-> [Baris 2]
 *
 * Sesudah:
 *   [Baris 0] <-> [Baris 1] <-> [NODE BARU] <-> [Baris 2]
 *
 * Empat pointer yang harus diupdate:
 *   node->prev  = curr        (atas)
 *   node->next  = curr->next  (bawah)
 *   curr->next->prev = node   (bawah tunjuk balik)
 *   curr->next  = node        (atas tunjuk ke baru)
 */
void insertList(int after_cy, char *txt) {
    Line *curr = getNode(after_cy);
    Line *node = createNode();

    /* Isi teks jika ada */
    if (txt && txt[0] != '\0') {
        int slen = (int)strlen(txt);
        ensureCap(node, slen + 1);
        strcpy(node->data, txt);
        node->len = slen;
    }

    /* Sambungkan node baru ke list */
    node->prev = curr;
    node->next = curr->next;

    if (curr->next != NULL)
        curr->next->prev = node;  /* Baris bawah tunjuk balik ke node */
    else
        tail = node;              /* Node baru jadi tail */

    curr->next = node;
    line_count++;
}

/*
 * deleteList – hapus node baris ke-target_cy dari list.
 *
 * Tidak boleh hapus jika hanya ada 1 baris tersisa.
 *
 * Sebelum (hapus Baris 1):
 *   [Baris 0] <-> [Baris 1] <-> [Baris 2]
 *
 * Sesudah:
 *   [Baris 0] <-> [Baris 2]
 *
 * Dua pointer yang diupdate:
 *   prev->next = node->next
 *   next->prev = node->prev
 */
void deleteList(int target_cy) {
    if (line_count <= 1) return; /* Tidak boleh kosong */

    Line *node = getNode(target_cy);
    if (!node) return;

    if (node->prev) node->prev->next = node->next;
    else            head = node->next;  /* Yang dihapus adalah head */

    if (node->next) node->next->prev = node->prev;
    else            tail = node->prev;  /* Yang dihapus adalah tail */

    free(node->data);
    free(node);
    line_count--;
}

/*
 * freeList – bebaskan seluruh memori linked list.
 *
 * Berjalan dari head ke tail, bebaskan satu per satu.
 * Setelah ini head = tail = NULL, line_count = 0.
 */
void freeList(void) {
    Line *curr = head;
    while (curr != NULL) {
        Line *next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
    head = NULL;
    tail = NULL;
}