#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "edf.h"
#include "errors.h"
#include "output.h"
#include "parser.h"
#include "rate.h"
#include "sim.h"
#include "task.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "uso: %s <rate|edf> <arquivo_de_entrada>\n",
                argc > 0 ? argv[0] : "scheduler");
        return ERR_ARGS;
    }

    const char *algorithm = argv[1];
    PriorityKeyFn priority_key;

    if (strcmp(algorithm, "rate") == 0) {
        priority_key = rate_priority_key;
    } else if (strcmp(algorithm, "edf") == 0) {
        priority_key = edf_priority_key;
    } else {
        fprintf(stderr, "erro: algoritmo '%s' invalido (use 'rate' ou 'edf')\n",
                algorithm);
        return ERR_ALGORITHM;
    }

    Task *tasks;
    int task_count, total_time;
    int rc = parse_input_file(argv[2], &tasks, &task_count, &total_time);
    if (rc != ERR_OK) return rc;

    TaskState *states = NULL;
    if (task_count > 0) {
        states = malloc((size_t)task_count * sizeof(TaskState));
        if (states == NULL) {
            fprintf(stderr, "erro: memoria insuficiente\n");
            free(tasks);
            return ERR_INTERNAL;
        }
    }

    SimResult result;
    run_simulation(tasks, states, task_count, total_time, priority_key,
                   &result);

    if (write_output_file(algorithm, STUDENT_LOGIN, tasks, states, task_count,
                           &result) != 0) {
        fprintf(stderr, "erro: nao foi possivel escrever o arquivo de saida\n");
        free_sim_result(&result);
        free(states);
        free(tasks);
        return ERR_INTERNAL;
    }

    free_sim_result(&result);
    free(states);
    free(tasks);
    return ERR_OK;
}
