#include "Rafli.h"

// ===== Fitur Open File =====
void openFile() {
    char filename[100];
    FILE *fp;

    clearScreen();
    printf("Open file: ");

    fgets(filename, sizeof(filename), stdin);

    // handle newline sisa
    if (filename[0] == '\n') {
        fgets(filename, sizeof(filename), stdin);
    }

    // hapus newline
    filename[strcspn(filename, "\n")] = '\0';

    fp = fopen(filename, "r");

    if (!fp) {
        perror("Error opening file");
        system("pause");
        return;
    }

    line_count = 0;

    while (fgets(text[line_count], MAX_LENGTH, fp)) {

        // hapus newline dari file
        text[line_count][strcspn(text[line_count], "\n")] = '\0';

        if (line_count < MAX_LINES - 1) {
            line_count++;
        } else {
            break;
        }
    }

    fclose(fp);

    strcpy(currentFile, filename);

    // reset cursor
    cy = 0;
    cx = 0;
    row_offset = 0;

    // kalau file kosong
    if (line_count == 0) {
        line_count = 1;
        text[0][0] = '\0';
    }
}

// ===== Save File =====
void saveFile() {
    int i;
    FILE *fp;

    if (strlen(currentFile) == 0) {
        printf("Gunakan Save As dulu!\n");
        system("pause");
        return;
    }

    fp = fopen(currentFile, "w");

    if (!fp) {
        perror("Error saving file");
        return;
    }

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s\n", text[i]); // ✅ tambahin newline biar rapi
    }

    fclose(fp);
}

// ===== Fitur Save As =====
void saveAs() {
    int i;
    char filename[100];
    FILE *fp;

    clearScreen();
    printf("Save As: ");

    fgets(filename, sizeof(filename), stdin);

    if (filename[0] == '\n') {
        fgets(filename, sizeof(filename), stdin);
    }

    filename[strcspn(filename, "\n")] = '\0';

    fp = fopen(filename, "w");

    if (!fp) {
        perror("Error saving file");
        return;
    }

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s\n", text[i]); // ✅ biar tiap baris turun
    }

    fclose(fp);

    strcpy(currentFile, filename);
}

// ===== Fitur Close File =====
void closeFile() {
    line_count = 1;
    text[0][0] = '\0';
    currentFile[0] = '\0';
    cx = 0;
    cy = 0;
    row_offset = 0;
}