#ifndef TASK_H
#define TASK_H

#define MAX_TASK_NAME 32

typedef struct {
    char name[MAX_TASK_NAME];
    int period;
    int deadline;
    int burst;
    int order; /* posicao no arquivo; criterio de desempate */
} Task;

typedef struct {
    int has_instance;
    int remaining_burst;
    int absolute_deadline;
    int next_arrival;

    long completed_count;
    long lost_count;
    long killed_count;
} TaskState;

#endif /* TASK_H */
