#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "noel.h"

/* Kode tombol khusus (nilai > 127, tidak bentrok dengan ASCII) */
typedef enum {
    ARROW_UP    = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT
} SpecialKey;

int  readKey(void);
void processEditKey(int key);
void processCommandKey(int key);
void processKey(int key);

#endif