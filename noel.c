#include "noel.h"

// Pointer untuk Riwayat Undo Redo (Double Linked List)
UndoNode *history_head = NULL;
UndoNode *history_curr = NULL;

// ==========================================
// ALOKASI & MANAJEMEN MEMORI UNDO/REDO
// ==========================================

Line* duplicateBuffer(Line *src_head) {
    if (src_head == NULL) return NULL;
    
    Line *new_head = (Line*)malloc(sizeof(Line));
    strcpy(new_head->data, src_head->data);
    new_head->prev = NULL;
    new_head->next = NULL;
    
    Line *curr_src = src_head->next;
    Line *curr_dest = new_head;
    
    while(curr_src != NULL) {
        Line *node = (Line*)malloc(sizeof(Line));
        strcpy(node->data, curr_src->data);
        node->prev = curr_dest;
        node->next = NULL;
        curr_dest->next = node;
        
        curr_dest = node;
        curr_src = curr_src->next;
    }
    return new_head;
}

void freeTextBuffer(Line *target_head) {
    Line *curr = target_head;
    while(curr != NULL) {
        Line *next = curr->next;
        free(curr);
        curr = next;
    }
}

void freeUndoRedoHistory() {
    UndoNode *curr = history_head;
    while(curr != NULL) {
        UndoNode *next = curr->next;
        freeTextBuffer(curr->head_state);
        free(curr);
        curr = next;
    }
    history_head = NULL;
    history_curr = NULL;
}

// ==========================================
// MEKANISME UNDO & REDO ENGINE
// ==========================================

void saveState() {
    // Jika kursor berada di tengah riwayat akibat undo, hapus sisa redo ke depan
    if (history_curr != NULL && history_curr->next != NULL) {
        UndoNode *del = history_curr->next;
        while(del != NULL) {
            UndoNode *next_del = del->next;
            freeTextBuffer(del->head_state);
            free(del);
            del = next_del;
        }
        history_curr->next = NULL;
    }

    UndoNode *node = (UndoNode*)malloc(sizeof(UndoNode));
    node->head_state = duplicateBuffer(head);
    node->cx = cx;
    node->cy = cy;
    node->prev = history_curr;
    node->next = NULL;

    if (history_curr != NULL) {
        history_curr->next = node;
    } else {
        history_head = node;
    }
    history_curr = node;
}

void undo() {
    if (history_curr == NULL || history_curr->prev == NULL) {
        return; // Tidak ada aksi untuk di-undo
    }
    history_curr = history_curr->prev;

    // Restore text buffer ke state sebelumnya
    freeTextBuffer(head);
    head = duplicateBuffer(history_curr->head_state);
    
    // Sinkronisasi tail & cursor_line kembali ke posisi semula
    Line *curr = head;
    int current_row = 0;
    line_count = 1;
    cursor_line = head;
    
    while (curr->next != NULL) {
        if(current_row == history_curr->cy) {
            cursor_line = curr;
        }
        curr = curr->next;
        line_count++;
        current_row++;
    }
    if(current_row == history_curr->cy) {
        cursor_line = curr;
    }
    tail = curr;

    cx = history_curr->cx;
    cy = history_curr->cy;
}

void redo() {
    if (history_curr == NULL || history_curr->next == NULL) {
        return; // Tidak ada aksi untuk di-redo
    }
    history_curr = history_curr->next;

    // Restore text buffer ke state setelahnya
    freeTextBuffer(head);
    head = duplicateBuffer(history_curr->head_state);
    
    Line *curr = head;
    int current_row = 0;
    line_count = 1;
    cursor_line = head;
    
    while (curr->next != NULL) {
        if(current_row == history_curr->cy) {
            cursor_line = curr;
        }
        curr = curr->next;
        line_count++;
        current_row++;
    }
    if(current_row == history_curr->cy) {
        cursor_line = curr;
    }
    tail = curr;

    cx = history_curr->cx;
    cy = history_curr->cy;
}

// ==========================================
// FITUR GLOBAL COPY PASTE (WINDOWS API)
// ==========================================

void copyToClipboard() {
    if (cursor_line == NULL) return;

    int len = (int)strlen(cursor_line->data);
    if (len == 0) return;

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, len + 1);
        char *pchData;
        pchData = (char*)GlobalLock(hClipboardData);
        strcpy(pchData, cursor_line->data);
        GlobalUnlock(hClipboardData);
        SetClipboardData(CF_TEXT, hClipboardData);
        CloseClipboard();
    }
}

void pasteFromClipboard() {
    if (OpenClipboard(NULL)) {
        HANDLE hClipboardData = GetClipboardData(CF_TEXT);
        if (hClipboardData != NULL) {
            char *pchData = (char*)GlobalLock(hClipboardData);
            if (pchData != NULL) {
                saveState(); // Simpan state sebelum menempel teks baru
                
                int len = (int)strlen(pchData);
                for (int i = 0; i < len; i++) {
                    if (pchData[i] == '\n' || pchData[i] == '\r') {
                        insertNewLine();
                        if (pchData[i] == '\r' && pchData[i+1] == '\n') i++; 
                    } else {
                        insertChar(pchData[i]);
                    }
                }
                GlobalUnlock(hClipboardData);
            }
        }
        CloseClipboard();
    }
}

// ==========================================
// FITUR NEW FILE
// ==========================================

void clearEditorToNewFile() {
    // Bersihkan buffer teks lama dan riwayat undo-redo
    freeTextBuffer(head);
    freeUndoRedoHistory();

    // Reset total ke kondisi awal kosong
    head          = (Line*)malloc(sizeof(Line));
    head->data[0] = '\0';
    head->prev    = NULL;
    head->next    = NULL;
    
    tail        = head;
    cursor_line = head;
    line_count  = 1;
    cx          = 0;
    cy          = 0;
    row_offset  = 0;
    mode        = 0; // Pembuatan file baru akan mengembalikan ke mode edit secara default
    strcpy(currentFile, "");

    // Simpan inisiasi awal ke dalam state history baru
    saveState();
}