#include "Fauzan.h"
#include "Rafli.h"

Node* createNode() {
    Node *newNode = (Node*) malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("ERROR: Memori penuh!\n");
        exit(1);
    }
    newNode->data[0] = '\0';
    newNode->next    = NULL;
    newNode->prev    = NULL;
    return newNode;
}

Node* getNodeAt(int index) {
    Node *temp = head;
    int i;
    for (i = 0; i < index && temp != NULL; i++) {
        temp = temp->next;
    }
    return temp;
}

void freeAllNodes() {
    Node *temp = head;
    while (temp != NULL) {
        Node *nextNode = temp->next;
        free(temp);
        temp = nextNode;
    }
    head    = NULL;
    current = NULL;
}

int fileExists(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp != NULL) {
        fclose(fp);
        return 1;
    }
    return 0;
}

void getUniqueFilename(char *filename, char *result) {
    char name[100] = "";
    char ext[20]   = "";
    char *dot;
    int   counter = 1;

    if (!fileExists(filename)) {
        strcpy(result, filename);
        return;
    }

    dot = strrchr(filename, '.');
    if (dot != NULL) {
        strncpy(name, filename, dot - filename);
        name[dot - filename] = '\0';
        strcpy(ext, dot);
    } else {
        strcpy(name, filename);
        ext[0] = '\0';
    }

    do {
        sprintf(result, "%s(%d)%s", name, counter, ext);
        counter++;
    } while (fileExists(result));
}

void writeLinesToFile(FILE *fp) {
    Node *temp = head;
    while (temp != NULL) {
        fprintf(fp, "%s\n", temp->data);
        temp = temp->next;
    }
}

void openFile() {
    char  filename[100];
    FILE *fp;
    Node *newNode, *last;
    char  buffer[MAX_LENGTH];

    system("cls");
    // setCursorVisibility(1);

    printf("===== BUKA FILE =====\n");
    printf("Nama file: ");

    fflush(stdin);
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';

    // setCursorVisibility(0);

    if (strlen(filename) == 0) {
        printf("Nama file tidak boleh kosong!\n");
        system("pause");
        return;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File '%s' tidak ditemukan!\n", filename);
        system("pause");
        return;
    }

    freeAllNodes();
    line_count = 0;
    last = NULL;

    while (fgets(buffer, MAX_LENGTH, fp)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        newNode = createNode();
        strcpy(newNode->data, buffer);

        if (head == NULL) {
            head = newNode;
        } else {
            last->next    = newNode;
            newNode->prev = last;
        }
        last = newNode;
        line_count++;
    }

    if (line_count == 0) {
        head = createNode();
        line_count = 1;
    }

    fclose(fp);

    // Update status file saat ini
    strcpy(currentFile, filename);
    current    = head;
    cy         = 0;
    cx         = 0;
    row_offset = 0;

    printf("Berhasil membuka '%s' (%d baris)\n", filename, line_count);
    system("pause");
}

void saveFile() {
    FILE *fp;

    // Jika belum ada nama file, arahkan ke Save As
    if (strlen(currentFile) == 0) {
        printf("Belum ada nama file! Gunakan Save As [A].\n");
        system("pause");
        return;
    }

    fp = fopen(currentFile, "w");
    if (fp == NULL) {
        printf("Gagal menyimpan file '%s'!\n", currentFile);
        system("pause");
        return;
    }

    writeLinesToFile(fp);
    fclose(fp);

    printf("File disimpan: %s\n", currentFile);
    system("pause");
}

// ===== Fitur Save As =====
void saveAs() {
    char filename[100];
    char finalName[120];
    char uniqueName[140];
    char choice;
    FILE *fp;

    system("cls");
    // setCursorVisibility(1);

    printf("===== SIMPAN SEBAGAI =====\n");
    printf("Nama file: ");

    fflush(stdin);
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';

    // setCursorVisibility(0);

    if (strlen(filename) == 0) {
        printf("Nama file tidak boleh kosong!\n");
        system("pause");
        return;
    }

    printf("\nPilih format:\n");
    printf("  1. File teks (.txt)\n");
    printf("  2. File biasa (tanpa ekstensi)\n");
    printf("Pilihan [1/2]: ");

    choice = getch();
    printf("%c\n", choice);

    if (choice == '1') {
        if (strlen(filename) < 4 ||
            strcmp(filename + strlen(filename) - 4, ".txt") != 0) {
            sprintf(finalName, "%s.txt", filename);
        } else {
            strcpy(finalName, filename);
        }
    } else {
        strcpy(finalName, filename);
    }

    getUniqueFilename(finalName, uniqueName);

    if (strcmp(uniqueName, finalName) != 0) {
        printf("\nFile '%s' sudah ada!\n", finalName);
        printf("Disimpan sebagai: '%s'\n", uniqueName);
    }

    fp = fopen(uniqueName, "w");
    if (fp == NULL) {
        printf("Gagal membuat file!\n");
        system("pause");
        return;
    }

    writeLinesToFile(fp);
    fclose(fp);

    strcpy(currentFile, uniqueName);
    printf("Berhasil disimpan sebagai: %s\n", currentFile);
    system("pause");
}

// ===== Fitur Close File =====
void closeFile() {
    char choice;

    system("cls");

    printf("===== TUTUP FILE =====\n");
    printf("File aktif: %s\n\n",
           strlen(currentFile) ? currentFile : "(Tidak ada)");
    printf("Yakin ingin menutup file?\n\n");
    printf("  1. Yes        - Tutup TANPA simpan\n");
    printf("  2. Save First - Simpan DULU, lalu tutup\n");
    printf("  3. Cancel     - Batalkan\n");
    printf("\nPilihan [1/2/3]: ");

    choice = getch();
    printf("%c\n\n", choice);

    if (choice == '1') {
        freeAllNodes();
        head           = createNode();
        current        = head;
        line_count     = 1;
        currentFile[0] = '\0';
        cx = cy = row_offset = 0;
        printf("File ditutup.\n");
        system("pause");

    } else if (choice == '2') {
        saveFile();
        freeAllNodes();
        head           = createNode();
        current        = head;
        line_count     = 1;
        currentFile[0] = '\0';
        cx = cy = row_offset = 0;

    } else {
        printf("Dibatalkan.\n");
        system("pause");
    }
}