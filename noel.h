#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h> // Diperlukan untuk Clipboard Global Windows

#define MAX_LENGTH  1024
#define MAX_LINES   10000
#define VIEW_HEIGHT 20

// ==========================================
// KAMUS DATA / STRUKTUR DATA
// ==========================================

// Node untuk Text Buffer (Double Linked List)
typedef struct Line {
    char        data[MAX_LENGTH];
    struct Line *prev;
    struct Line *next;
} Line;

// Node untuk Editor State (History Undo/Redo menggunakan Double Linked List)
typedef struct UndoNode {
    Line            *head_state; // Salinan utuh text buffer pada state tersebut
    int             cx, cy;      // Posisi kursor pada state tersebut
    struct UndoNode *prev;
    struct UndoNode *next;
} UndoNode;

// Variabel Global (Extern)
extern Line *head;
extern Line *tail;
extern Line *cursor_line;

extern int  line_count;
extern int  cx, cy;
extern int  row_offset;
extern int  mode;
extern char currentFile[100];

// ==========================================
// DAFTAR FUNGSI OPERASI TEXT EDITOR
// ==========================================

// 1. Alokasi & Manajemen Memori
Line* createLine();
Line* duplicateBuffer(Line *src_head);
void freeTextBuffer(Line *target_head);

// 2. Operasi Dasar Linked List (Insert & Deletion)
void insertChar(char c);
void insertNewLine();
void mergeWithPrevLine();
void deleteChar();

// 3. Traversal / Navigasi & Print
Line* getLine(int n);
void moveCursor(int key);
void printLineWithCursor(Line *line, int fileRow);
void editorRefreshScreen();

// 4. Fitur Utama: Copy, Paste, Undo, Redo, New File
void copyToClipboard();
void pasteFromClipboard();
void saveState();
void undo();
void redo();
void clearEditorToNewFile();
void freeUndoRedoHistory();

#endif