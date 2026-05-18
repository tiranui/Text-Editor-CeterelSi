#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>

/* ─── Konstanta ──────────────────────────────────────────────────── */
#define MAX_LENGTH   256
#define VIEW_HEIGHT  20
#define UNDO_TIMEOUT 2.0

/* Kompatibilitas _strdup antara MSVC dan GCC/MinGW */
#ifdef _MSC_VER
#  define str_dup _strdup
#else
#  define str_dup strdup
#endif

/* ─── Struktur Data ──────────────────────────────────────────────── */

/* Baris teks: Doubly Linked List (ada prev DAN next) */
typedef struct Line {
    char         data[MAX_LENGTH];
    struct Line *next;
    struct Line *prev;          /* <-- BARU: pointer mundur */
} Line;

/* Node snapshot untuk Undo/Redo: Doubly Linked List */
typedef struct EditorSnapshot {
    char **lines_data;          /* array string per-baris */
    int    line_count;
    int    cx, cy;
    struct EditorSnapshot *prev;
    struct EditorSnapshot *next;
} EditorSnapshot;

/* Papan klip (clipboard) */
typedef struct ClipboardNode {
    char              data[MAX_LENGTH];
    struct ClipboardNode *next;
} ClipboardNode;

/* ─── Variabel Global (extern – didefinisikan di masing-masing .c) ─ */

/* noel.c */
extern Line           *head;
extern Line           *tail;            /* <-- BARU: ujung list baris */
extern ClipboardNode  *clip_head;
extern EditorSnapshot *history_cursor;
extern time_t          last_action_time;

/* main.c */
extern int  line_count, cx, cy, row_offset, mode;
extern char currentFile[100];

/* ─── Prototipe Fungsi (noel.c) ──────────────────────────────────── */
Line *getLineNode(int index);

void  pushUndo(void);
void  undoAction(void);
void  redoAction(void);

void  copyLine(void);
void  pasteLine(void);
void  newFile(void);

void  freeLines(void);
void  freeRedoChain(void);
void  restoreSnapshot(EditorSnapshot *snap);

void  checkUndoCondition(char c);
void  insertChar(char c);
void  insertNewLine(void);
void  deleteChar(void);

void  editorRefreshScreen(void);

#endif /* NOEL_H */