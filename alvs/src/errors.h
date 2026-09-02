#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERR_OK           = 0,
    ERR_ARGS         = 1,
    ERR_ALGORITHM    = 2,
    ERR_FILE_OPEN    = 3,
    ERR_FILE_FORMAT  = 4,
    ERR_TASK_SPEC    = 5,
} ErrorCode;

#endif /* ERRORS_H */
