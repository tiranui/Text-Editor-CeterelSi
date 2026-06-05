#ifndef RAFLI_H
#define RAFLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "Fauzan.h"

#ifdef FAUZAN_H
    #define Node       Line
    #define createNode createLine

    // deklarasi fungsi pakai Line langsung
    extern Line *head;
    extern Line *current;
    extern int   line_count;
    extern char  currentFile[100];
    extern int   cx, cy;
    extern int   row_offset;
    extern int   mode;

    Line* createNode();
    Line* getNodeAt(int index);
    void  freeAllNodes();
    void  openFile();
    void  closeFile();
    void  saveFile();
    void  saveAs();
    void  clearScreen();
    void  setCursorVisibility(int visible);

#else
    #ifndef MAX_LENGTH
    #define MAX_LENGTH 256
    #endif

    #ifndef VIEW_HEIGHT
    #define VIEW_HEIGHT 20
    #endif

    typedef struct Node {
        char line[MAX_LENGTH];
        struct Node *next;
        struct Node *prev;
    } Node;

    extern Node *head;
    extern Node *current;
    extern int   line_count;
    extern char  currentFile[100];
    extern int   cx, cy;
    extern int   row_offset;
    extern int   mode;

    Node* createNode();
    Node* getNodeAt(int index);
    void  freeAllNodes();
    void  openFile();
    void  closeFile();
    void  saveFile();
    void  saveAs();
    void  clearScreen();
    void  setCursorVisibility(int visible);

#endif

#endif // RAFLI_H