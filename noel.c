#include "noel.h"

char clipboard[MAX_CLIPBOARD][MAX_LENGTH];
int  clipboard_line_count = 0;

EditorSnapshot undo_stack[MAX_UNDO];
int            undo_top  = -1;
EditorSnapshot redo_stack[MAX_UNDO];
int            redo_top  = -1;

void pushUndo() {
    int i, k;
    if (undo_top < MAX_UNDO - 1) {
        undo_top++;
    } else {
        for (k = 0; k < MAX_UNDO - 1; k++)
            undo_stack[k] = undo_stack[k + 1];
    }
    for (i = 0; i < MAX_LINES; i++)
        strcpy(undo_stack[undo_top].text[i], text[i]);
    undo_stack[undo_top].line_count = line_count;
    undo_stack[undo_top].cx        = cx;
    undo_stack[undo_top].cy        = cy;
    redo_top = -1;
}

void undoAction() {
    int i, k;
    if (undo_top < 0) {
        system("cls");
        printf("Tidak ada aksi yang bisa di-undo.\n");
        system("pause");
        return;
    }
    if (redo_top < MAX_UNDO - 1) {
        redo_top++;
    } else {
        for (k = 0; k < MAX_UNDO - 1; k++)
            redo_stack[k] = redo_stack[k + 1];
    }
    for (i = 0; i < MAX_LINES; i++)
        strcpy(redo_stack[redo_top].text[i], text[i]);
    redo_stack[redo_top].line_count = line_count;
    redo_stack[redo_top].cx        = cx;
    redo_stack[redo_top].cy        = cy;
    for (i = 0; i < MAX_LINES; i++)
        strcpy(text[i], undo_stack[undo_top].text[i]);
    line_count = undo_stack[undo_top].line_count;
    cx         = undo_stack[undo_top].cx;
    cy         = undo_stack[undo_top].cy;
    undo_top--;
}

void redoAction() {
    int i, k, saved;
    if (redo_top < 0) {
        system("cls");
        printf("Tidak ada aksi yang bisa di-redo.\n");
        system("pause");
        return;
    }
    saved = redo_top;
    if (undo_top < MAX_UNDO - 1) {
        undo_top++;
    } else {
        for (k = 0; k < MAX_UNDO - 1; k++)
            undo_stack[k] = undo_stack[k + 1];
    }
    for (i = 0; i < MAX_LINES; i++)
        strcpy(undo_stack[undo_top].text[i], text[i]);
    undo_stack[undo_top].line_count = line_count;
    undo_stack[undo_top].cx        = cx;
    undo_stack[undo_top].cy        = cy;
    for (i = 0; i < MAX_LINES; i++)
        strcpy(text[i], redo_stack[saved].text[i]);
    line_count = redo_stack[saved].line_count;
    cx         = redo_stack[saved].cx;
    cy         = redo_stack[saved].cy;
    redo_top--;
}

void newFile() {
    int i;
    char confirm[4];
    system("cls");
    printf("Buat file baru? Perubahan yang belum disimpan akan hilang. (y/n): ");
    fgets(confirm, sizeof(confirm), stdin);
    if (confirm[0] != 'y' && confirm[0] != 'Y') return;
    pushUndo();
    for (i = 0; i < MAX_LINES; i++) text[i][0] = '\0';
    line_count     = 1;
    cx             = 0;
    cy             = 0;
    row_offset     = 0;
    currentFile[0] = '\0';
}

void copyLine() {
    clipboard_line_count = 1;
    strcpy(clipboard[0], text[cy]);
    system("cls");
    printf("Baris %d disalin ke clipboard.\n", cy + 1);
    system("pause");
}

void pasteLine() {
    int i, j, insert_at;
    if (clipboard_line_count == 0) {
        system("cls");
        printf("Clipboard kosong. Copy dulu sebelum paste.\n");
        system("pause");
        return;
    }
    pushUndo();
    insert_at = cy + 1;
    for (i = line_count + clipboard_line_count - 1; i >= insert_at + clipboard_line_count; i--)
        strcpy(text[i], text[i - clipboard_line_count]);
    for (j = 0; j < clipboard_line_count; j++)
        strcpy(text[insert_at + j], clipboard[j]);
    line_count += clipboard_line_count;
    cy = insert_at;
    cx = 0;
}