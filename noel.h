#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

// PANGGIL FAUZAN.H AGAR BISA BERBAGI STRUCT LINE YANG SAMA
#include "Fauzan.h"

typedef struct UndoNode {
    Line            *head_state;
    int             cx, cy;
    struct UndoNode *prev;
    struct UndoNode *next;
} UndoNode;

/* Prototipe Fungsi */
Line* duplicateBuffer(Line *src_head);
void freeTextBuffer(Line *target_head);
int areBuffersEqual(Line *h1, Line *h2);

void insertChar(char c);
void insertNewLine();
void mergeWithPrevLine();
void deleteChar();

Line* getLine(int n);
void moveCursor(int key);
void printLineWithCursor(Line *line, int fileRow);
void editorRefreshScreen();

void copyToClipboard();
void pasteFromClipboard();
void saveState();
void undo();
void redo();
void clearEditorToNewFile();
void freeUndoRedoHistory();

#endif