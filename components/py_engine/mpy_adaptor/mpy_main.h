/*
 * Copyright (C) 2018 Alibaba Group Holding Limited
 */

/*
DESCRIPTION
This file provides haascv entry.
*/

#ifndef HAASCV_MAIN_H
#define HAASCV_MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    int argc;
    char **argv;
    bool is_bootup;
} mpy_thread_args;

extern int mpy_run(int argc, char **argv);
extern int mpy_init(mpy_thread_args *args);
extern int mpy_deinit(void);
extern void mpy_add_path(char * path,int8_t length);


#ifdef __cplusplus
}
#endif

#endif /* HAASCV_MAIN_H */
