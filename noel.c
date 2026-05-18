#include "noel.h"

/* ─── Definisi variabel global (noel.c) ─────────────────────────── */
Line           *head           = NULL;
Line           *tail           = NULL;
ClipboardNode  *clip_head      = NULL;
EditorSnapshot *history_cursor = NULL;
time_t          last_action_time = 0;

/* ═══════════════════════════════════════════════════════════════════
   MANAJEMEN BARIS (Doubly Linked List)
   ═══════════════════════════════════════════════════════════════════ */

/* Kembalikan node baris ke-index (0-based). */
Line *getLineNode(int index) {
    Line *curr = head;
    for (int i = 0; i < index && curr != NULL; i++)
        curr = curr->next;
    return curr;
}

/* Bebaskan seluruh baris dan reset head/tail. */
void freeLines(void) {
    Line *curr = head;
    while (curr) {
        Line *nxt = curr->next;
        free(curr);
        curr = nxt;
    }
    head = NULL;
    tail = NULL;
}

/* ═══════════════════════════════════════════════════════════════════
   MANAJEMEN SNAPSHOT (Undo / Redo)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * freeRedoChain – hapus semua snapshot DI DEPAN history_cursor
 * (yaitu redo chain), termasuk membebaskan setiap string lines_data
 * dan struct EditorSnapshot-nya sendiri agar tidak ada memory leak.
 */
void freeRedoChain(void) {
    if (!history_cursor || !history_cursor->next) return;

    EditorSnapshot *curr = history_cursor->next;
    while (curr) {
        EditorSnapshot *nxt = curr->next;
        /* bebaskan setiap string baris */
        for (int i = 0; i < curr->line_count; i++)
            free(curr->lines_data[i]);
        free(curr->lines_data);
        free(curr);
        curr = nxt;
    }
    history_cursor->next = NULL;
}

/*
 * pushUndo – simpan keadaan editor saat ini sebagai snapshot baru.
 * Memanggil freeRedoChain sehingga redo chain lama dihapus terlebih
 * dahulu sebelum snapshot baru ditambahkan.
 */
void pushUndo(void) {
    EditorSnapshot *snap = (EditorSnapshot *)malloc(sizeof(EditorSnapshot));
    if (!snap) return;

    snap->line_count  = line_count;
    snap->cx          = cx;
    snap->cy          = cy;
    snap->lines_data  = (char **)malloc(line_count * sizeof(char *));

    for (int i = 0; i < line_count; i++)
        snap->lines_data[i] = str_dup(getLineNode(i)->data);

    /* Hapus redo chain SEBELUM menambahkan snapshot baru */
    freeRedoChain();

    snap->prev = history_cursor;
    snap->next = NULL;
    if (history_cursor) history_cursor->next = snap;
    history_cursor = snap;
}

/*
 * updateCurrentSnapshot – perbarui isi history_cursor dengan keadaan
 * editor SAAT INI tanpa memotong redo chain.
 *
 * Digunakan oleh undoAction agar state sebelum undo tetap bisa
 * dikembalikan via redo, meskipun ada perubahan tidak-tersimpan
 * setelah snapshot terakhir.
 */
static void updateCurrentSnapshot(void) {
    if (!history_cursor) {
        /* Belum ada snapshot sama sekali, buat fresh */
        pushUndo();
        return;
    }

    /* Bebaskan data lama di node ini */
    for (int i = 0; i < history_cursor->line_count; i++)
        free(history_cursor->lines_data[i]);
    free(history_cursor->lines_data);

    /* Tulis ulang dengan keadaan saat ini */
    history_cursor->line_count = line_count;
    history_cursor->cx         = cx;
    history_cursor->cy         = cy;
    history_cursor->lines_data = (char **)malloc(line_count * sizeof(char *));
    for (int i = 0; i < line_count; i++)
        history_cursor->lines_data[i] = str_dup(getLineNode(i)->data);
}

/*
 * restoreSnapshot – kembalikan seluruh baris teks DAN posisi kursor
 * persis seperti yang tersimpan di snapshot. Pointer prev/next pada
 * baris baru diatur dengan benar (doubly linked list).
 */
void restoreSnapshot(EditorSnapshot *snap) {
    if (!snap) return;

    freeLines();

    Line *last = NULL;
    for (int i = 0; i < snap->line_count; i++) {
        Line *node = (Line *)malloc(sizeof(Line));
        strcpy(node->data, snap->lines_data[i]);
        node->next = NULL;
        node->prev = last;           /* hubungkan ke belakang */
        if (!head) head = node;
        else       last->next = node;
        last = node;
    }
    tail = last;                     /* perbarui tail */

    line_count = snap->line_count;
    cx = snap->cx;
    cy = snap->cy;
}

/*
 * undoAction – simpan state saat ini ke history_cursor (sehingga
 * redo bisa kembali ke sini), lalu mundur satu langkah ke snapshot
 * sebelumnya.
 *
 * Perbaikan "state lost": updateCurrentSnapshot dipanggil SEBELUM
 * kursor digeser, menjamin perubahan yang belum di-push tersimpan.
 */
void undoAction(void) {
    if (!history_cursor || !history_cursor->prev) return;

    /* Pastikan state tak-tersimpan diabadikan di node saat ini */
    updateCurrentSnapshot();

    history_cursor = history_cursor->prev;
    restoreSnapshot(history_cursor);
    last_action_time = 0;
}

/*
 * redoAction – maju satu langkah ke snapshot berikutnya.
 * Tidak memanggil freeRedoChain sehingga redo chain utuh.
 */
void redoAction(void) {
    if (!history_cursor || !history_cursor->next) return;

    history_cursor = history_cursor->next;
    restoreSnapshot(history_cursor);
    last_action_time = 0;
}

/* ═══════════════════════════════════════════════════════════════════
   OPERASI EDIT
   ═══════════════════════════════════════════════════════════════════ */

/*
 * checkUndoCondition – panggil pushUndo jika:
 *   a) pengguna mengetik spasi (pemisah kata natural), atau
 *   b) jeda sejak aksi terakhir melebihi UNDO_TIMEOUT detik.
 */
void checkUndoCondition(char c) {
    time_t now = time(NULL);
    if (difftime(now, last_action_time) > UNDO_TIMEOUT || c == ' ')
        pushUndo();
    last_action_time = now;
}

/* Sisipkan karakter c di posisi cx baris cy. */
void insertChar(char c) {
    checkUndoCondition(c);
    Line *curr = getLineNode(cy);
    int len = (int)strlen(curr->data);
    if (len < MAX_LENGTH - 1) {
        for (int i = len; i >= cx; i--)
            curr->data[i + 1] = curr->data[i];
        curr->data[cx] = c;
        cx++;
    }
}

/*
 * insertNewLine – pecah baris cy di posisi cx menjadi dua baris.
 * Pointer doubly linked (prev/next) diperbarui sepenuhnya, termasuk
 * memperbarui tail jika baris baru berada di akhir.
 */
void insertNewLine(void) {
    pushUndo();
    Line *curr = getLineNode(cy);

    Line *node = (Line *)malloc(sizeof(Line));
    strcpy(node->data, curr->data + cx);   /* sisa teks setelah kursor */
    curr->data[cx] = '\0';                 /* potong baris saat ini */

    /* Sambungkan doubly linked list */
    node->next = curr->next;
    node->prev = curr;
    if (curr->next) curr->next->prev = node;
    else            tail = node;           /* node baru adalah tail baru */
    curr->next = node;

    line_count++;
    cy++;
    cx = 0;
    if (cy >= row_offset + VIEW_HEIGHT) row_offset++;
    last_action_time = time(NULL);
}

/*
 * deleteChar – hapus karakter sebelum kursor (Backspace).
 * Jika di awal baris (cx == 0), gabungkan dengan baris di atasnya.
 * Pointer doubly linked diperbarui saat penggabungan.
 */
void deleteChar(void) {
    checkUndoCondition('\b');
    Line *curr = getLineNode(cy);

    if (cx > 0) {
        /* Hapus satu karakter di belakang kursor */
        int len = (int)strlen(curr->data);
        for (int i = cx - 1; i < len; i++)
            curr->data[i] = curr->data[i + 1];
        cx--;
    } else if (cy > 0) {
        /* Gabungkan baris saat ini ke baris sebelumnya */
        pushUndo();
        Line *prev_line = curr->prev;       /* manfaatkan pointer prev */
        int   prev_len  = (int)strlen(prev_line->data);
        strcat(prev_line->data, curr->data);

        /* Lepaskan node curr dari list */
        prev_line->next = curr->next;
        if (curr->next) curr->next->prev = prev_line;
        else            tail = prev_line;   /* prev_line jadi tail baru */
        free(curr);

        cy--;
        cx = prev_len;
        line_count--;
        if (cy < row_offset) row_offset--;
    }
}

/* ═══════════════════════════════════════════════════════════════════
   CLIPBOARD
   ═══════════════════════════════════════════════════════════════════ */

/* Salin isi baris cy ke clipboard. */
void copyLine(void) {
    Line *curr = getLineNode(cy);
    if (!curr) return;

    if (clip_head) free(clip_head);
    clip_head = (ClipboardNode *)malloc(sizeof(ClipboardNode));
    strcpy(clip_head->data, curr->data);
    clip_head->next = NULL;
}

/*
 * pasteLine – tempelkan isi clipboard sebagai baris baru tepat di
 * bawah baris cy.  Pointer doubly linked diperbarui sepenuhnya.
 */
void pasteLine(void) {
    if (!clip_head) return;
    pushUndo();

    Line *curr = getLineNode(cy);
    Line *node = (Line *)malloc(sizeof(Line));
    strcpy(node->data, clip_head->data);

    node->next = curr->next;
    node->prev = curr;
    if (curr->next) curr->next->prev = node;
    else            tail = node;
    curr->next = node;

    line_count++;
    cy++;
    last_action_time = 0;
}

/* ═══════════════════════════════════════════════════════════════════
   FILE BARU
   ═══════════════════════════════════════════════════════════════════ */

/* Buat dokumen kosong (simpan state sebelumnya ke undo terlebih dulu). */
void newFile(void) {
    pushUndo();
    freeLines();

    head = (Line *)malloc(sizeof(Line));
    head->data[0] = '\0';
    head->next    = NULL;
    head->prev    = NULL;
    tail = head;

    line_count = 1;
    cx = cy = row_offset = 0;
    currentFile[0] = '\0';
}

/* ═══════════════════════════════════════════════════════════════════
   TAMPILAN (RENDER)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * editorRefreshScreen – tulis seluruh tampilan ke stdout sekali
 * (melalui satu buffer besar) untuk mengurangi flicker.
 */
void editorRefreshScreen(void) {
    static char screen_buf[16384];
    char        tmp[512];

    screen_buf[0] = '\0';

    /* Pindahkan kursor ke pojok kiri atas tanpa membersihkan layar */
    strcat(screen_buf, "\x1b[H");

    /* Header */
    sprintf(tmp, "=== NOTEPAD LINKED LIST === | MODE: %s\x1b[K\n",
            mode == 0 ? "EDIT" : "COMMAND");
    strcat(screen_buf, tmp);

    sprintf(tmp, "File: [%s] | Line: %d, Col: %d\x1b[K\n\n",
            currentFile[0] ? currentFile : "Untitled", cy + 1, cx + 1);
    strcat(screen_buf, tmp);

    /* Navigasi ke baris pertama yang terlihat */
    Line *curr = head;
    for (int i = 0; i < row_offset && curr; i++) curr = curr->next;

    /* Render VIEW_HEIGHT baris */
    for (int i = 0; i < VIEW_HEIGHT; i++) {
        int file_row = i + row_offset;
        if (file_row < line_count && curr) {
            sprintf(tmp, "%3d | ", file_row + 1);
            strcat(screen_buf, tmp);

            int len = (int)strlen(curr->data);
            for (int j = 0; j <= len; j++) {
                if (file_row == cy && j == cx)
                    strcat(screen_buf, "|");   /* penanda kursor */
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

    /* Footer shortcut hint */
    strcat(screen_buf,
           "\n[ESC] Command | [Z] Undo  [R] Redo  "
           "[Y] Copy  [P] Paste  [N] New  [Q] Quit\x1b[K");

    printf("%s", screen_buf);
    fflush(stdout);
}