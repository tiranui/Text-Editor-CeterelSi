#include "noel.h"
#include "keywords.h"

int  line_count = 1;
int  cx = 0, cy = 0;
int  row_offset = 0;
int  mode = 0;
char currentFile[100] = "";

void editorRefreshScreen(void) {
    static char screen_buf[16384];
    char        tmp[512];

    screen_buf[0] = '\0';

    strcat(screen_buf, "\x1b[H");

    sprintf(tmp, "=== NOTEPAD LINKED LIST === | MODE: %s\x1b[K\n",
            mode == 0 ? "EDIT" : "COMMAND");
    strcat(screen_buf, tmp);

    sprintf(tmp, "File: [%s] | Line: %d, Col: %d\x1b[K\n\n",
            currentFile[0] ? currentFile : "Untitled", cy + 1, cx + 1);
    strcat(screen_buf, tmp);

    Line *curr = head;
    for (int i = 0; i < row_offset && curr; i++) curr = curr->next;

    for (int i = 0; i < VIEW_HEIGHT; i++) {
        int file_row = i + row_offset;
        if (file_row < line_count && curr) {
            sprintf(tmp, "%3d | ", file_row + 1);
            strcat(screen_buf, tmp);

            int len = (int)strlen(curr->data);
            for (int j = 0; j <= len; j++) {
                if (file_row == cy && j == cx)
                    strcat(screen_buf, "|");
                if (j < len) {
                    tmp[0] = curr->data[j];
                    tmp[1] = '\0';
                    strcat(screen_buf, tmp);
                }
            }
            strcat(screen_buf, "\x1b[K\n");
            curr = curr->next;
        } else {
            strcat(screen_buf, "~\x1b[K\n");
        }
    }

    strcat(screen_buf,
           "\n[ESC] Command | [Z] Undo  [R] Redo  "
           "[Y] Copy  [P] Paste  [N] New  [Q] Quit\x1b[K");

    printf("%s", screen_buf);
    fflush(stdout);
}

int main(void) {
    printf("\x1b[?25l");
    system("cls");

    head = (Line *)malloc(sizeof(Line));
    head->data[0] = '\0';
    head->next    = NULL;
    head->prev    = NULL;
    tail = head;

    pushUndo();

    while (1) {
        editorRefreshScreen();
        int key = readKey();
        processKey(key);
    }

    return 0;
}