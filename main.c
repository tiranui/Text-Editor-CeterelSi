/*Library*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES 1000
#define MAX_LENGTH 256
#define VIEW_HEIGHT 20
/*Program*/
/* Data Teks*/
char text[MAX_LINES][MAX_LENGTH];
int line_count = 1;

char currentFile[100] = "";

int cx = 0, cy = 0;
int row_offset = 0;

int mode = 0; // 0 = EDIT, 1 = COMMAND
/*Page up & Page up*/
// ================= KEY =================
enum keys {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN
};