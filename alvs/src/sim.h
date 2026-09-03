#ifndef SIM_H
#define SIM_H

#include "task.h"

typedef enum {
    LOG_FINISHED,  /* F */
    LOG_PREEMPTED, /* H */
    LOG_LOST,      /* L */
    LOG_KILLED,    /* K */
    LOG_IDLE,
} LogEntryType;

typedef struct {
    LogEntryType type;
    int task_index; /* -1 quando type == LOG_IDLE */
    int duration;
} LogEntry;

typedef struct {
    LogEntry *entries;
    int count;
} SimResult;

/* Menor valor = maior prioridade. Empate e resolvido pelo campo
 * task->order fora desta funcao. */
typedef long (*PriorityKeyFn)(const Task *task, const TaskState *state);

void run_simulation(const Task *tasks, TaskState *states, int n,
                     int total_time, PriorityKeyFn priority_key,
                     SimResult *result);

void free_sim_result(SimResult *result);

#endif /* SIM_H */
