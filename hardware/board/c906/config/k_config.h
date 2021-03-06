/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#ifndef K_CONFIG_H
#define K_CONFIG_H

/* kernel feature conf */
#ifndef RHINO_CONFIG_SEM
#define RHINO_CONFIG_SEM                     1
#endif
#ifndef RHINO_CONFIG_QUEUE
#define RHINO_CONFIG_QUEUE                   1
#endif
#ifndef RHINO_CONFIG_TASK_SEM
#define RHINO_CONFIG_TASK_SEM                1
#endif
#ifndef RHINO_CONFIG_EVENT_FLAG
#define RHINO_CONFIG_EVENT_FLAG              1
#endif
#ifndef RHINO_CONFIG_TIMER
#define RHINO_CONFIG_TIMER                   1
#endif
#ifndef RHINO_CONFIG_BUF_QUEUE
#define RHINO_CONFIG_BUF_QUEUE               1
#endif
#ifndef RHINO_CONFIG_MM_BLK
#define RHINO_CONFIG_MM_BLK                  1
#endif
#ifndef RHINO_CONFIG_MM_DEBUG
#define RHINO_CONFIG_MM_DEBUG                0
#endif
#ifndef RHINO_CONFIG_MM_TLF
#define RHINO_CONFIG_MM_TLF                  1
#endif
#ifndef RHINO_CONFIG_MM_BLK_SIZE
#define RHINO_CONFIG_MM_BLK_SIZE             256
#endif
#ifndef RHINO_CONFIG_MM_MAXMSIZEBIT
#define RHINO_CONFIG_MM_MAXMSIZEBIT          28
#endif
#ifndef RHINO_CONFIG_MM_TLF_BLK_SIZE
#define RHINO_CONFIG_MM_TLF_BLK_SIZE         1024
#endif
#ifndef RHINO_CONFIG_MM_TRACE_LVL
#define RHINO_CONFIG_MM_TRACE_LVL            4//8
#endif

/* kernel task conf */
#ifndef RHINO_CONFIG_TASK_INFO
#define RHINO_CONFIG_TASK_INFO               1
#endif
#ifndef RHINO_CONFIG_TASK_DEL
#define RHINO_CONFIG_TASK_DEL                1
#endif

#ifndef RHINO_CONFIG_TASK_STACK_OVF_CHECK
#define RHINO_CONFIG_TASK_STACK_OVF_CHECK    1
#endif
#ifndef RHINO_CONFIG_SCHED_RR
#define RHINO_CONFIG_SCHED_RR                1
#endif
#ifndef RHINO_CONFIG_TIME_SLICE_DEFAULT
#define RHINO_CONFIG_TIME_SLICE_DEFAULT      50
#endif
#ifndef RHINO_CONFIG_PRI_MAX
#define RHINO_CONFIG_PRI_MAX                 62
#endif
#ifndef RHINO_CONFIG_USER_PRI_MAX
#define RHINO_CONFIG_USER_PRI_MAX            (RHINO_CONFIG_PRI_MAX - 2)
#endif

/* kernel workqueue conf */
#ifndef RHINO_CONFIG_WORKQUEUE
#define RHINO_CONFIG_WORKQUEUE               1
#endif
#ifndef RHINO_CONFIG_WORKQUEUE_STACK_SIZE
#define RHINO_CONFIG_WORKQUEUE_STACK_SIZE    512
#endif

/* kernel mm_region conf */
#ifndef RHINO_CONFIG_MM_REGION_MUTEX
#define RHINO_CONFIG_MM_REGION_MUTEX         1
#endif

/* kernel timer&tick conf */
#ifndef RHINO_CONFIG_HW_COUNT
#define RHINO_CONFIG_HW_COUNT                0
#endif

#ifndef RHINO_CONFIG_TICKS_PER_SECOND
#define RHINO_CONFIG_TICKS_PER_SECOND        100
#endif

/*must reserve enough stack size for timer cb will consume*/
#ifndef RHINO_CONFIG_TIMER_TASK_STACK_SIZE
#define RHINO_CONFIG_TIMER_TASK_STACK_SIZE   512
#endif
#ifndef RHINO_CONFIG_TIMER_TASK_PRI
#define RHINO_CONFIG_TIMER_TASK_PRI          5
#endif

/* kernel dyn alloc conf */
#ifndef RHINO_CONFIG_KOBJ_DYN_ALLOC
#define RHINO_CONFIG_KOBJ_DYN_ALLOC          1
#endif

#if (RHINO_CONFIG_KOBJ_DYN_ALLOC > 0)
#ifndef RHINO_CONFIG_K_DYN_TASK_STACK
#define RHINO_CONFIG_K_DYN_TASK_STACK        512
#endif
#ifndef RHINO_CONFIG_K_DYN_MEM_TASK_PRI
#define RHINO_CONFIG_K_DYN_MEM_TASK_PRI      6
#endif
#endif

/* kernel idle conf */
#ifndef RHINO_CONFIG_IDLE_TASK_STACK_SIZE
#define RHINO_CONFIG_IDLE_TASK_STACK_SIZE    512
#endif

/* kernel hook conf */
#ifndef RHINO_CONFIG_USER_HOOK
#define RHINO_CONFIG_USER_HOOK               0
#endif

#ifndef RHINO_CONFIG_CPU_NUM
#define RHINO_CONFIG_CPU_NUM                 1
#endif

#ifndef RHINO_CONFIG_SYSTEM_STACK_SIZE
#define RHINO_CONFIG_SYSTEM_STACK_SIZE       0x180
#endif

/*task user info index start*/
#ifndef RHINO_CONFIG_TASK_INFO_NUM
#define RHINO_CONFIG_TASK_INFO_NUM           5
#endif

#ifndef PTHREAD_CONFIG_USER_INFO_POS
#define PTHREAD_CONFIG_USER_INFO_POS         0
#endif

#ifndef RHINO_TASK_HOOK_USER_INFO_POS
#define RHINO_TASK_HOOK_USER_INFO_POS        1
#endif

#ifndef RHINO_CLI_CONSOLE_USER_INFO_POS
#define RHINO_CLI_CONSOLE_USER_INFO_POS      2
#endif

#ifndef RHINO_ERRNO_USER_INFO_POS
#define RHINO_ERRNO_USER_INFO_POS            3
#endif
/*task user info index end*/

#endif /* K_CONFIG_H */

