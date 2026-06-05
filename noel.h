#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

#define MAX_LENGTH  1024
#define MAX_LINES   10000
#define VIEW_HEIGHT 20

typedef struct Line {
    char        data[MAX_LENGTH];
    struct Line *prev;
    struct Line *next;
} Line;

typedef struct UndoNode {
    Line            *head_state;
    int             cx, cy;
    struct UndoNode *prev;
    struct UndoNode *next;
} UndoNode;

extern Line *head;
extern Line *tail;
extern Line *cursor_line;

extern int  line_count;
extern int  cx, cy;
extern int  row_offset;
extern int  mode;
extern char currentFile[100];

Line* createLine();
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