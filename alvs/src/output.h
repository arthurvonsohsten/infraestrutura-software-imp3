#ifndef OUTPUT_H
#define OUTPUT_H

#include "sim.h"
#include "task.h"

int write_output_file(const char *algorithm_name, const char *login,
                       const Task *tasks, const TaskState *states, int n,
                       const SimResult *result);

#endif /* OUTPUT_H */
