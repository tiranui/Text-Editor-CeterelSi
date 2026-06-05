#ifndef RAFLI_H
#define RAFLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "Fauzan.h"

// Alias: Rafli.c pakai nama "Node" dan "createNode",
// keduanya di-redirect ke Line dan createLine milik Fauzan
#define Node Line

// "current" di Rafli.c = cursor_line di Fauzan.h
#define current    cursor_line

extern Line *head;
extern Line *cursor_line;   // ini yang actual
extern Line *tail;
extern int   line_count;
extern char  currentFile[100];
extern int   cx, cy;
extern int   row_offset;
extern int   mode;

Line* getNodeAt(int index);
Node* createNode();
void  freeAllNodes();
void  openFile();
void  closeFile();
void  saveFile();
void  saveAs();
void  clearScreen();
void  setCursorVisibility(int visible);

#endif