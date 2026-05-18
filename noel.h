#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>

#define MAX_LENGTH   256
#define VIEW_HEIGHT  20
#define UNDO_TIMEOUT 2.0

char *str_dup(const char *s);

typedef struct Line {
    char         data[MAX_LENGTH];
    struct Line *next;
    struct Line *prev;
} Line;

typedef struct EditorSnapshot {
    char **lines_data;
    int    line_count;
    int    cx, cy;
    struct EditorSnapshot *prev;
    struct EditorSnapshot *next;
} EditorSnapshot;

typedef struct ClipboardNode {
    char              data[MAX_LENGTH];
    struct ClipboardNode *next;
} ClipboardNode;

extern Line           *head;
extern Line           *tail;
extern ClipboardNode  *clip_head;
extern EditorSnapshot *history_cursor;
extern time_t          last_action_time;

extern int  line_count, cx, cy, row_offset, mode;
extern char currentFile[100];

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

#endif