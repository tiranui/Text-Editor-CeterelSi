#include "noel.h"

UndoNode *history_head = NULL;
UndoNode *history_curr = NULL;

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

int areBuffersEqual(Line *h1, Line *h2) {
    while (h1 != NULL && h2 != NULL) {
        if (strcmp(h1->data, h2->data) != 0) return 0;
        h1 = h1->next;
        h2 = h2->next;
    }
    return (h1 == NULL && h2 == NULL);
}

void saveState() {
    if (head == NULL) return;

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

    if (history_curr != NULL) {
        Line *temp_dup = duplicateBuffer(head);
        if (temp_dup != NULL && areBuffersEqual(temp_dup, history_curr->head_state)) {
            history_curr->cx = cx;
            history_curr->cy = cy;
            freeTextBuffer(temp_dup);
            return; 
        }
        if (temp_dup != NULL) freeTextBuffer(temp_dup);
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
        return;
    }
    history_curr = history_curr->prev;

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

void redo() {
    if (history_curr == NULL || history_curr->next == NULL) {
        return;
    }
    history_curr = history_curr->next;

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
                saveState();
                
                char remainder[MAX_LENGTH];
                strcpy(remainder, cursor_line->data + cx);
                cursor_line->data[cx] = '\0'; 

                int len = (int)strlen(pchData);
                int i; 
                for (i = 0; i < len; i++) {
                    // Proteksi: cegah luapan buffer jika char melampaui MAX_LENGTH
                    if ((int)strlen(cursor_line->data) >= MAX_LENGTH - 1 && pchData[i] != '\n' && pchData[i] != '\r') {
                        break; 
                    }

                    if (pchData[i] == '\r') {
                        if (line_count >= MAX_LINES) break;
                        insertNewLine();
                        if (pchData[i+1] == '\n') i++;
                    } else if (pchData[i] == '\n') {
                        if (line_count >= MAX_LINES) break;
                        insertNewLine();
                    } else {
                        insertChar(pchData[i]);
                    }
                }
                
                // Menggabungkan kembali sisa baris teks dengan aman
                if ((int)(strlen(cursor_line->data) + strlen(remainder)) < MAX_LENGTH) {
                    strcat(cursor_line->data, remainder);
                } else {
                    int current_len = (int)strlen(cursor_line->data);
                    int available = MAX_LENGTH - current_len - 1;
                    if (available > 0) {
                        strncat(cursor_line->data, remainder, available);
                    }
                }
                
                GlobalUnlock(hClipboardData);
            }
        }
        CloseClipboard();
    }
}

void clearEditorToNewFile() {
    freeTextBuffer(head);
    freeUndoRedoHistory();

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
    mode        = 0;
    strcpy(currentFile, "");

    saveState();
}