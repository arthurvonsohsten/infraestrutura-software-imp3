#include "rate.h"

long rate_priority_key(const Task *task, const TaskState *state) {
    (void)state;
    return task->period;
}
