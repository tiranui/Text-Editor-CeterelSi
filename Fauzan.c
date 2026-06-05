#include "Fauzan.h"

void findText() {
    char  keyword[100];
    int   found  = 0;
    int   rowNum = 1;
    Line *curr;

    system("cls");
    printf("\nMasukkan kata yang dicari: ");
    scanf("%s", keyword);

    printf("\nHasil pencarian \"%s\":\n", keyword);
    printf("----------------------------------\n");

    curr = head;
    while (curr != NULL) {
        if (strstr(curr->data, keyword) != NULL) {
            printf("Baris %d: %s\n", rowNum, curr->data);
            found = 1;
        }
        curr = curr->next;
        rowNum++;
    }

    if (!found) {
        printf("Kata \"%s\" tidak ditemukan.\n", keyword);
    }

    printf("----------------------------------\n");
    printf("Tekan tombol apa saja...");
    getch();
}

static void replaceAllInLine(char *line, const char *oldWord, const char *newWord) {
    char  buffer[MAX_LENGTH];
    char *src    = line;
    char *pos;
    int   oldLen = (int)strlen(oldWord);
    int   partLen;

    buffer[0] = '\0';

    while ((pos = strstr(src, oldWord)) != NULL) {
        partLen = (int)(pos - src);
        strncat(buffer, src, partLen);
        strcat(buffer, newWord);
        src = pos + oldLen;
    }

    strcat(buffer, src);
    strcpy(line, buffer);
}

void replaceText() {
    char  oldWord[100], newWord[100];
    int   totalReplaced = 0;
    Line *curr;
    char *check;

    system("cls");
    printf("\nKata yang ingin diganti : ");
    scanf("%s", oldWord);

    printf("Ganti dengan            : ");
    scanf("%s", newWord);

    printf("\nProses replace...\n");

    curr = head;
    while (curr != NULL) {
        check = curr->data;
        while ((check = strstr(check, oldWord)) != NULL) {
            totalReplaced++;
            check += strlen(oldWord);
        }

        replaceAllInLine(curr->data, oldWord, newWord);
        curr = curr->next;
    }

    if (totalReplaced > 0) {
        printf("Replace selesai! Total %d kemunculan diganti.\n", totalReplaced);
    } else {
        printf("Kata \"%s\" tidak ditemukan.\n", oldWord);
    }

    printf("Tekan tombol apa saja...");
    getch();
}