#ifndef FAUZAN_H
#define FAUZAN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>


#define MAX_LENGTH  1024
#define MAX_LINES   10000
#define VIEW_HEIGHT 20

typedef struct Line {
    char        data[MAX_LENGTH];
    struct Line *prev;            
    struct Line *next;            
} Line;

extern Line *head;
extern Line *tail;
extern Line *cursor_line;

extern int  line_count;
extern int  cx, cy;
extern int  row_offset;
extern int  mode;
extern char currentFile[100];

Line* createLine();
Line* getLine(int n);

void findText();
void replaceText();

#endif