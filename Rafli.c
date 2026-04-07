#include "Rafli.h"


// char currentFile[100] = "";

// int cx = 0, cy = 0;
// int row_offset = 0;

// int mode = 0;
/*fitue file*/
// ================= FILE =================
/*open file*/
void openFile() {
    char filename[100];
    FILE *fp;

    clearScreen();
    printf("Open file: ");

    fgets(filename, sizeof(filename), stdin);
    if (filename[0] == '\n') fgets(filename, sizeof(filename), stdin);

    filename[strcspn(filename, "\n")] = 0;

    fp = fopen(filename, "r");

    if (!fp) {
        perror("Error");
        system("pause");
        return;
    }

    line_count = 0;

    while (fgets(text[line_count], MAX_LENGTH, fp)) {
        line_count++;
    }

    fclose(fp);
    strcpy(currentFile, filename);

    cy = 0;
    cx = 0;
    row_offset = 0;
}

/*save & save as*/
void saveFile() {
    int i;
    FILE *fp;

    if (strlen(currentFile) == 0) {
        printf("Gunakan Save As dulu!\n");
        system("pause");
        return;
    }

    fp = fopen(currentFile, "w");

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s", text[i]);
    }

    fclose(fp);
}

void saveAs() {
    int i;
    char filename[100];
    FILE *fp;

    clearScreen();
    printf("Save As: ");

    fgets(filename, sizeof(filename), stdin);
    if (filename[0] == '\n') fgets(filename, sizeof(filename), stdin);

    filename[strcspn(filename, "\n")] = 0;

    fp = fopen(filename, "w");

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s", text[i]);
    }

    fclose(fp);

    strcpy(currentFile, filename);
}
/*close file*/

void closeFile() {
    line_count = 1;
    text[0][0] = '\0';
    currentFile[0] = '\0';
    cx = cy = 0;
}