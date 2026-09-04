#include "edf.h"

long edf_priority_key(const Task *task, const TaskState *state) {
    (void)task;
    return state->absolute_deadline;
}
