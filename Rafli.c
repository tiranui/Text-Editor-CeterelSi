#include "Rafli.h"

// ===== Fitur Open File =====
void openFile() {
    char filename[100];
    FILE *fp;

    // Tidak perlu clearScreen() di sini karena main.c sudah handle refresh
    // Kita langsung minta input di area yang sudah disiapkan gotoxy() di main
    printf("Buka file: "); 

    // Menggunakan fgets untuk mengambil nama file
    if (fgets(filename, sizeof(filename), stdin) != NULL) {
        // Hapus newline di akhir input
        filename[strcspn(filename, "\n")] = '\0';
    }

    // Jika user menekan enter tanpa ngetik apa-apa
    if (strlen(filename) == 0) return;

    fp = fopen(filename, "r");

    if (!fp) {
        // Jika error, tampilkan pesan di bawah agar tidak merusak UI
        printf("\nError: File tidak ditemukan!");
        system("pause");
        return;
    }

    line_count = 0;

    // Membaca file baris demi baris
    while (fgets(text[line_count], MAX_LENGTH, fp) && line_count < MAX_LINES) {
        // Hapus newline bawaan file agar tidak double newline di editor
        text[line_count][strcspn(text[line_count], "\r\n")] = '\0';
        line_count++;
    }

    fclose(fp);

    // Update status file saat ini
    strcpy(currentFile, filename);

    // Reset posisi kursor ke awal file
    cy = 0;
    cx = 0;
    row_offset = 0;

    // Jika file yang dibuka ternyata kosong
    if (line_count == 0) {
        line_count = 1;
        text[0][0] = '\0';
    }
}

// ===== Save File =====
void saveFile() {
    int i;
    FILE *fp;

    // Jika belum ada nama file, arahkan ke Save As
    if (strlen(currentFile) == 0) {
        gotoxy(1, VIEW_HEIGHT + 11);
        printf("Peringatan: Gunakan Save As dulu!");
        system("pause");
        return;
    }

    fp = fopen(currentFile, "w");

    if (!fp) {
        perror("Error saving file");
        system("pause");
        return;
    }

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s\n", text[i]); 
    }

    fclose(fp);
    
    gotoxy(1, VIEW_HEIGHT + 11);
    printf("File berhasil disimpan ke: %s", currentFile);
    system("pause");
}

// ===== Fitur Save As =====
void saveAs() {
    int i;
    char filename[100];
    FILE *fp;

    printf("Simpan sebagai: ");

    if (fgets(filename, sizeof(filename), stdin) != NULL) {
        filename[strcspn(filename, "\n")] = '\0';
    }

    if (strlen(filename) == 0) return;

    fp = fopen(filename, "w");

    if (!fp) {
        perror("Error saving file");
        system("pause");
        return;
    }

    for (i = 0; i < line_count; i++) {
        fprintf(fp, "%s\n", text[i]); 
    }

    fclose(fp);

    // Update nama file aktif
    strcpy(currentFile, filename);
    
    printf("\nBerhasil disimpan!");
    system("pause");
}

// ===== Fitur Close File =====
void closeFile() {
    int i;
    line_count = 1;
    
    // Bersihkan semua array text
    for (i = 0; i < MAX_LINES; i++) {
        text[i][0] = '\0';
    }
    
    currentFile[0] = '\0';
    cx = 0;
    cy = 0;
    row_offset = 0;
}