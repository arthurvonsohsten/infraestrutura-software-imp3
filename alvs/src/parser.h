#ifndef PARSER_H
#define PARSER_H

#include "task.h"

int parse_input_file(const char *path, Task **out_tasks, int *out_count,
                      int *out_total_time);

#endif /* PARSER_H */
