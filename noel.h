#ifndef NOEL_H
#define NOEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES   1000
#define MAX_LENGTH  256
#define VIEW_HEIGHT 20

#define MAX_CLIPBOARD 100
#define MAX_UNDO      50

extern char text[MAX_LINES][MAX_LENGTH];
extern int  line_count;
extern char currentFile[100];
extern int  cx, cy;
extern int  row_offset;
extern int  mode;

/* Clipboard */
extern char clipboard[MAX_CLIPBOARD][MAX_LENGTH];
extern int  clipboard_line_count;

/* Fungsi */
void newFile();
void copyLine();
void pasteLine();
void pushUndo();
void undoAction();
void redoAction();

typedef struct {
    char text[MAX_LINES][MAX_LENGTH];
    int  line_count;
    int  cx, cy;
} EditorSnapshot;

extern EditorSnapshot undo_stack[MAX_UNDO];
extern int            undo_top;
extern EditorSnapshot redo_stack[MAX_UNDO];
extern int            redo_top;

#endif