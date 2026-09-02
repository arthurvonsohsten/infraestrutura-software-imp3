#include "parser.h"
#include "errors.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 512
#define INITIAL_CAPACITY 8

static int parse_positive_int(const char *token, int *out) {
    if (token == NULL || *token == '\0') return 0;

    errno = 0;
    char *end;
    long value = strtol(token, &end, 10);

    if (*end != '\0' || end == token) return 0;
    if (errno == ERANGE || value <= 0 || value > INT_MAX) return 0;

    *out = (int)value;
    return 1;
}

static int is_blank_line(const char *line) {
    for (const char *p = line; *p != '\0'; p++) {
        if (!isspace((unsigned char)*p)) return 0;
    }
    return 1;
}

int parse_input_file(const char *path, Task **out_tasks, int *out_count,
                      int *out_total_time) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "erro: nao foi possivel abrir '%s': %s\n", path,
                strerror(errno));
        return ERR_FILE_OPEN;
    }

    char line[LINE_BUF_SIZE];
    int line_no = 0;
    int total_time = 0;
    int have_total_time = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        if (is_blank_line(line)) continue;

        char *first = strtok(line, " \t\r\n");
        char *extra = strtok(NULL, " \t\r\n");
        if (!parse_positive_int(first, &total_time) || extra != NULL) {
            fprintf(stderr,
                    "erro: linha %d: tempo total de simulacao invalido "
                    "(esperado um unico inteiro positivo)\n",
                    line_no);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }
        have_total_time = 1;
        break;
    }

    if (!have_total_time) {
        fprintf(stderr,
                "erro: arquivo vazio ou sem tempo total de simulacao\n");
        fclose(fp);
        return ERR_FILE_FORMAT;
    }

    int capacity = INITIAL_CAPACITY;
    int count = 0;
    Task *tasks = malloc((size_t)capacity * sizeof(Task));
    if (tasks == NULL) {
        fprintf(stderr, "erro: memoria insuficiente\n");
        fclose(fp);
        return ERR_INTERNAL;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        if (is_blank_line(line)) continue;

        char *tokens[5];
        int ntok = 0;
        char *tok = strtok(line, " \t\r\n");
        while (tok != NULL && ntok < 5) {
            tokens[ntok++] = tok;
            tok = strtok(NULL, " \t\r\n");
        }

        if (ntok != 4) {
            fprintf(stderr,
                    "erro: linha %d: esperado 'NOME PERIODO DEADLINE "
                    "BURST', encontrado(s) %d campo(s)\n",
                    line_no, ntok);
            free(tasks);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }

        if (strlen(tokens[0]) >= MAX_TASK_NAME) {
            fprintf(stderr,
                    "erro: linha %d: nome de tarefa '%s' excede %d "
                    "caracteres\n",
                    line_no, tokens[0], MAX_TASK_NAME - 1);
            free(tasks);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }

        int period, deadline, burst;
        if (!parse_positive_int(tokens[1], &period)) {
            fprintf(stderr,
                    "erro: linha %d: periodo invalido '%s' (esperado "
                    "inteiro positivo)\n",
                    line_no, tokens[1]);
            free(tasks);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }
        if (!parse_positive_int(tokens[2], &deadline)) {
            fprintf(stderr,
                    "erro: linha %d: deadline invalido '%s' (esperado "
                    "inteiro positivo)\n",
                    line_no, tokens[2]);
            free(tasks);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }
        if (!parse_positive_int(tokens[3], &burst)) {
            fprintf(stderr,
                    "erro: linha %d: burst invalido '%s' (esperado "
                    "inteiro positivo)\n",
                    line_no, tokens[3]);
            free(tasks);
            fclose(fp);
            return ERR_FILE_FORMAT;
        }

        if (burst > deadline || deadline > period) {
            fprintf(stderr,
                    "erro: linha %d: tarefa '%s' viola C <= D <= P "
                    "(C=%d, D=%d, P=%d)\n",
                    line_no, tokens[0], burst, deadline, period);
            free(tasks);
            fclose(fp);
            return ERR_TASK_SPEC;
        }

        if (count == capacity) {
            capacity *= 2;
            Task *grown = realloc(tasks, (size_t)capacity * sizeof(Task));
            if (grown == NULL) {
                fprintf(stderr, "erro: memoria insuficiente\n");
                free(tasks);
                fclose(fp);
                return ERR_INTERNAL;
            }
            tasks = grown;
        }

        Task *t = &tasks[count];
        strcpy(t->name, tokens[0]);
        t->period = period;
        t->deadline = deadline;
        t->burst = burst;
        t->order = count;
        count++;
    }

    fclose(fp);

    *out_tasks = tasks;
    *out_count = count;
    *out_total_time = total_time;
    return ERR_OK;
}
