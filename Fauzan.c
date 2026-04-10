#include "Rafli.h"
#include "Fauzan.h"

// ===== Fitur Find =====
void findText() {
    char keyword[100];
    int i, found = 0;

    printf("\nMasukkan kata yang dicari: ");
    scanf("%s", keyword);

    printf("\nHasil:\n");

    for (i = 0; i < line_count; i++) {
        if (strstr(text[i], keyword) != NULL) {
            printf("Baris %d: %s\n", i + 1, text[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("Tidak ditemukan.\n");
    }

    printf("\nTekan tombol apa saja...");
    getch();
}

// ===== Fitur Replace =====
void replaceText() {
    char oldWord[100], newWord[100];
    int i;

    printf("\nKata yang ingin diganti: ");
    scanf("%s", oldWord);

    printf("Ganti dengan: ");
    scanf("%s", newWord);

    for (i = 0; i < line_count; i++) {
        char buffer[MAX_LENGTH * 2];
        char *pos = strstr(text[i], oldWord);

        if (pos != NULL) {
            int index = pos - text[i];

            strncpy(buffer, text[i], index);
            buffer[index] = '\0';

            strcat(buffer, newWord);
            strcat(buffer, pos + strlen(oldWord));

            strcpy(text[i], buffer);
        }
    }

    printf("\nReplace selesai!\n");
    printf("Tekan tombol apa saja...");
    getch();
}