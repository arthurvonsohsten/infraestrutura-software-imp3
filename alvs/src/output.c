#include "output.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char status_letter(LogEntryType type) {
    switch (type) {
        case LOG_FINISHED: return 'F';
        case LOG_PREEMPTED: return 'H';
        case LOG_LOST: return 'L';
        case LOG_KILLED: return 'K';
        default: return '?';
    }
}

int write_output_file(const char *algorithm_name, const char *login,
                       const Task *tasks, const TaskState *states, int n,
                       const SimResult *result) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s_%s.out", algorithm_name, login);

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) return -1;

    char upper_algo[32];
    size_t len = strlen(algorithm_name);
    if (len >= sizeof(upper_algo)) len = sizeof(upper_algo) - 1;
    for (size_t i = 0; i < len; i++) {
        upper_algo[i] = (char)toupper((unsigned char)algorithm_name[i]);
    }
    upper_algo[len] = '\0';

    fprintf(fp, "EXECUTION BY %s\n", upper_algo);
    for (int i = 0; i < result->count; i++) {
        const LogEntry *e = &result->entries[i];
        if (e->type == LOG_IDLE) {
            fprintf(fp, "idle for %d units\n", e->duration);
        } else {
            fprintf(fp, "[%s] for %d units - %c\n", tasks[e->task_index].name,
                    e->duration, status_letter(e->type));
        }
    }

    fprintf(fp, "LOST DEADLINES\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "[%s] %ld\n", tasks[i].name, states[i].lost_count);
    }

    fprintf(fp, "COMPLETE EXECUTION\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "[%s] %ld\n", tasks[i].name, states[i].completed_count);
    }

    fprintf(fp, "KILLED\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "[%s] %ld\n", tasks[i].name, states[i].killed_count);
    }

    fclose(fp);
    return 0;
}
