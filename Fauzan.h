#ifndef FAUZAN_H
#define FAUZAN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

// ================= KONSTANTA =================
#define MAX_LENGTH  1024
#define MAX_LINES   10000
#define VIEW_HEIGHT 20

// ================= STRUKTUR NODE LINKED LIST =================
/*
 *  Setiap baris teks = satu node dalam doubly linked list.
 *
 *   head                                      tail
 *    |                                          |
 *    v                                          v
 *  [Line] <--> [Line] <--> [Line] <--> ... <--> [Line]
 *  "baris1"   "baris2"   "baris3"             "barisN"
 *
 *  cursor_line menunjuk ke node tempat kursor berada saat ini.
 */
typedef struct Line {
    char        data[MAX_LENGTH]; // Isi teks baris ini
    struct Line *prev;            // Pointer ke baris sebelumnya
    struct Line *next;            // Pointer ke baris berikutnya
} Line;

// ================= VARIABEL GLOBAL (didefinisikan di main.c) =================
extern Line *head;
extern Line *tail;
extern Line *cursor_line;

extern int  line_count;
extern int  cx, cy;
extern int  row_offset;
extern int  mode;
extern char currentFile[100];

// ================= HELPER LINKED LIST (didefinisikan di main.c) =================
Line* createLine();
Line* getLine(int n);

// ================= FUNGSI FAUZAN =================
void findText();
void replaceText();

#endif
#ifndef FAUZAN_H
#define FAUZAN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

// ================= KONSTANTA =================
#define MAX_LENGTH  1024
#define MAX_LINES   10000
#define VIEW_HEIGHT 20

// ================= STRUKTUR NODE LINKED LIST =================
/*
 *  Setiap baris teks = satu node dalam doubly linked list.
 *
 *   head                                      tail
 *    |                                          |
 *    v                                          v
 *  [Line] <--> [Line] <--> [Line] <--> ... <--> [Line]
 *  "baris1"   "baris2"   "baris3"             "barisN"
 *
 *  cursor_line menunjuk ke node tempat kursor berada saat ini.
 */
typedef struct Line {
    char        data[MAX_LENGTH]; // Isi teks baris ini
    struct Line *prev;            // Pointer ke baris sebelumnya
    struct Line *next;            // Pointer ke baris berikutnya
} Line;

// ================= VARIABEL GLOBAL (didefinisikan di main.c) =================
extern Line *head;
extern Line *tail;
extern Line *cursor_line;

extern int  line_count;
extern int  cx, cy;
extern int  row_offset;
extern int  mode;
extern char currentFile[100];

// ================= HELPER LINKED LIST (didefinisikan di main.c) =================
Line* createLine();
Line* getLine(int n);

// ================= FUNGSI FAUZAN =================
void findText();
void replaceText();

#endif