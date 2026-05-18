#include "noel.h"

Line           *head           = NULL;
Line           *tail           = NULL;
ClipboardNode  *clip_head      = NULL;
EditorSnapshot *history_cursor = NULL;
time_t          last_action_time = 0;

Line *getLineNode(int index) {
    Line *curr = head;
    for (int i = 0; i < index && curr != NULL; i++)
        curr = curr->next;
    return curr;
}

char *str_dup(const char *s) {
    if (s == NULL) return NULL;
    
    char *d = (char *)malloc(strlen(s) + 1);
    if (d != NULL) {
        strcpy(d, s);
    }
    return d;
}

void freeLines(void) {
    Line *curr = head;
    while (curr) {
        Line *nxt = curr->next;
        free(curr);
        curr = nxt;
    }
    head = NULL;
    tail = NULL;
}

void freeRedoChain(void) {
    if (!history_cursor || !history_cursor->next) return;

    EditorSnapshot *curr = history_cursor->next;
    while (curr) {
        EditorSnapshot *nxt = curr->next;
        for (int i = 0; i < curr->line_count; i++)
            free(curr->lines_data[i]);
        free(curr->lines_data);
        free(curr);
        curr = nxt;
    }
    history_cursor->next = NULL;
}

void pushUndo(void) {
    EditorSnapshot *snap = (EditorSnapshot *)malloc(sizeof(EditorSnapshot));
    if (!snap) return;

    snap->line_count  = line_count;
    snap->cx          = cx;
    snap->cy          = cy;
    snap->lines_data  = (char **)malloc(line_count * sizeof(char *));

    for (int i = 0; i < line_count; i++)
        snap->lines_data[i] = str_dup(getLineNode(i)->data);

    freeRedoChain();

    snap->prev = history_cursor;
    snap->next = NULL;
    if (history_cursor) history_cursor->next = snap;
    history_cursor = snap;
}

static void updateCurrentSnapshot(void) {
    if (!history_cursor) {
        pushUndo();
        return;
    }

    for (int i = 0; i < history_cursor->line_count; i++)
        free(history_cursor->lines_data[i]);
    free(history_cursor->lines_data);

    history_cursor->line_count = line_count;
    history_cursor->cx         = cx;
    history_cursor->cy         = cy;
    history_cursor->lines_data = (char **)malloc(line_count * sizeof(char *));
    for (int i = 0; i < line_count; i++)
        history_cursor->lines_data[i] = str_dup(getLineNode(i)->data);
}

void restoreSnapshot(EditorSnapshot *snap) {
    if (!snap) return;

    freeLines();

    Line *last = NULL;
    for (int i = 0; i < snap->line_count; i++) {
        Line *node = (Line *)malloc(sizeof(Line));
        strcpy(node->data, snap->lines_data[i]);
        node->next = NULL;
        node->prev = last;
        if (!head) head = node;
        else       last->next = node;
        last = node;
    }
    tail = last;

    line_count = snap->line_count;
    cx = snap->cx;
    cy = snap->cy;
}

void undoAction(void) {
    if (!history_cursor || !history_cursor->prev) return;

    updateCurrentSnapshot();

    history_cursor = history_cursor->prev;
    restoreSnapshot(history_cursor);
    last_action_time = 0;
}

void redoAction(void) {
    if (!history_cursor || !history_cursor->next) return;

    history_cursor = history_cursor->next;
    restoreSnapshot(history_cursor);
    last_action_time = 0;
}

void checkUndoCondition(char c) {
    time_t now = time(NULL);
    if (difftime(now, last_action_time) > UNDO_TIMEOUT || c == ' ')
        pushUndo();
    last_action_time = now;
}

void insertChar(char c) {
    checkUndoCondition(c);
    Line *curr = getLineNode(cy);
    int len = (int)strlen(curr->data);
    if (len < MAX_LENGTH - 1) {
        for (int i = len; i >= cx; i--)
            curr->data[i + 1] = curr->data[i];
        curr->data[cx] = c;
        cx++;
    }
}

void insertNewLine(void) {
    pushUndo();
    Line *curr = getLineNode(cy);

    Line *node = (Line *)malloc(sizeof(Line));
    strcpy(node->data, curr->data + cx);
    curr->data[cx] = '\0';

    node->next = curr->next;
    node->prev = curr;
    if (curr->next) curr->next->prev = node;
    else            tail = node;
    curr->next = node;

    line_count++;
    cy++;
    cx = 0;
    if (cy >= row_offset + VIEW_HEIGHT) row_offset++;
    last_action_time = time(NULL);
}

void deleteChar(void) {
    checkUndoCondition('\b');
    Line *curr = getLineNode(cy);

    if (cx > 0) {
        int len = (int)strlen(curr->data);
        for (int i = cx - 1; i < len; i++)
            curr->data[i] = curr->data[i + 1];
        cx--;
    } else if (cy > 0) {
        pushUndo();
        Line *prev_line = curr->prev;
        int   prev_len  = (int)strlen(prev_line->data);
        strcat(prev_line->data, curr->data);

        prev_line->next = curr->next;
        if (curr->next) curr->next->prev = prev_line;
        else            tail = prev_line;
        free(curr);

        cy--;
        cx = prev_len;
        line_count--;
        if (cy < row_offset) row_offset--;
    }
}

void copyLine(void) {
    Line *curr = getLineNode(cy);
    if (!curr) return;

    if (clip_head) free(clip_head);
    clip_head = (ClipboardNode *)malloc(sizeof(ClipboardNode));
    strcpy(clip_head->data, curr->data);
    clip_head->next = NULL;
}

void pasteLine(void) {
    if (!clip_head) return;
    pushUndo();

    Line *curr = getLineNode(cy);
    Line *node = (Line *)malloc(sizeof(Line));
    strcpy(node->data, clip_head->data);

    node->next = curr->next;
    node->prev = curr;
    if (curr->next) curr->next->prev = node;
    else            tail = node;
    curr->next = node;

    line_count++;
    cy++;
    last_action_time = 0;
}

void newFile(void) {
    pushUndo();
    freeLines();

    head = (Line *)malloc(sizeof(Line));
    head->data[0] = '\0';
    head->next    = NULL;
    head->prev    = NULL;
    tail = head;

    line_count = 1;
    cx = cy = row_offset = 0;
    currentFile[0] = '\0';
}