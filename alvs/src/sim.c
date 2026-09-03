#include "sim.h"

#include <limits.h>
#include <stdlib.h>

#define LOG_INITIAL_CAPACITY 16

typedef struct {
    LogEntry *entries;
    int count;
    int capacity;
} LogBuffer;

static void log_buffer_init(LogBuffer *buf) {
    buf->capacity = LOG_INITIAL_CAPACITY;
    buf->count = 0;
    buf->entries = malloc((size_t)buf->capacity * sizeof(LogEntry));
}

static void log_push(LogBuffer *buf, LogEntryType type, int task_index,
                      int duration) {
    if (buf->count == buf->capacity) {
        buf->capacity *= 2;
        buf->entries =
            realloc(buf->entries, (size_t)buf->capacity * sizeof(LogEntry));
    }
    buf->entries[buf->count].type = type;
    buf->entries[buf->count].task_index = task_index;
    buf->entries[buf->count].duration = duration;
    buf->count++;
}

static int select_running_task(const Task *tasks, const TaskState *states,
                                int n, PriorityKeyFn priority_key) {
    int best = -1;
    long best_key = 0;

    for (int i = 0; i < n; i++) {
        if (!states[i].has_instance) continue;

        long key = priority_key(&tasks[i], &states[i]);
        if (best == -1 || key < best_key ||
            (key == best_key && tasks[i].order < tasks[best].order)) {
            best = i;
            best_key = key;
        }
    }
    return best;
}

void run_simulation(const Task *tasks, TaskState *states, int n,
                     int total_time, PriorityKeyFn priority_key,
                     SimResult *result) {
    LogBuffer buf;
    log_buffer_init(&buf);

    for (int i = 0; i < n; i++) {
        states[i].has_instance = 0;
        states[i].remaining_burst = 0;
        states[i].absolute_deadline = 0;
        states[i].next_arrival = 0;
        states[i].completed_count = 0;
        states[i].lost_count = 0;
        states[i].killed_count = 0;
    }

    int t = 0;
    while (t < total_time) {
        for (int i = 0; i < n; i++) {
            if (states[i].next_arrival == t) {
                states[i].has_instance = 1;
                states[i].remaining_burst = tasks[i].burst;
                states[i].absolute_deadline = t + tasks[i].deadline;
                states[i].next_arrival = t + tasks[i].period;
            }
        }

        for (int i = 0; i < n; i++) {
            if (states[i].has_instance && states[i].absolute_deadline <= t) {
                states[i].has_instance = 0;
                states[i].lost_count++;
            }
        }

        int running = select_running_task(tasks, states, n, priority_key);

        if (running == -1) {
            int next_event = total_time;
            for (int i = 0; i < n; i++) {
                if (states[i].next_arrival > t &&
                    states[i].next_arrival < next_event) {
                    next_event = states[i].next_arrival;
                }
            }
            log_push(&buf, LOG_IDLE, -1, next_event - t);
            t = next_event;
            continue;
        }

        int next_arrival_bound = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (states[i].next_arrival > t &&
                states[i].next_arrival < next_arrival_bound) {
                next_arrival_bound = states[i].next_arrival;
            }
        }

        int stretch = states[running].remaining_burst;
        if (states[running].absolute_deadline - t < stretch)
            stretch = states[running].absolute_deadline - t;
        if (next_arrival_bound - t < stretch) stretch = next_arrival_bound - t;
        if (total_time - t < stretch) stretch = total_time - t;

        int t_new = t + stretch;
        states[running].remaining_burst -= stretch;

        if (states[running].remaining_burst == 0) {
            log_push(&buf, LOG_FINISHED, running, stretch);
            states[running].has_instance = 0;
            states[running].completed_count++;
        } else if (t_new == states[running].absolute_deadline) {
            log_push(&buf, LOG_LOST, running, stretch);
            states[running].has_instance = 0;
            states[running].lost_count++;
        } else if (t_new == total_time) {
            log_push(&buf, LOG_KILLED, running, stretch);
            states[running].has_instance = 0;
            states[running].killed_count++;
        } else {
            log_push(&buf, LOG_PREEMPTED, running, stretch);
        }

        t = t_new;
    }

    for (int i = 0; i < n; i++) {
        if (states[i].has_instance) {
            states[i].has_instance = 0;
            states[i].killed_count++;
        }
    }

    result->entries = buf.entries;
    result->count = buf.count;
}

void free_sim_result(SimResult *result) {
    free(result->entries);
    result->entries = NULL;
    result->count = 0;
}
