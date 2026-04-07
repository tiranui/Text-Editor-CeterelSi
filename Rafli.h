/*Library*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES 1000
#define MAX_LENGTH 256
#define VIEW_HEIGHT 20

extern char text[MAX_LINES][MAX_LENGTH];
extern int line_count;
extern char currentFile[100];
extern int cx, cy;
extern int row_offset;
extern int mode;

//bagian open file
void openFile();

// bagian close file
void closeFile();

// bagian save file
void saveFile();

//bagian close file
void saveAs();

// Clear screen
void clearScreen();