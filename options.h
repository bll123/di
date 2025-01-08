/*
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_OPTIONS_H
#define DI_INC_OPTIONS_H

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#define DI_VAL_512              512.0
#define DI_VAL_1000             1000.0
#define DI_DISP_1000_IDX        0
#define DI_VAL_1024             1024.0
#define DI_DISP_1024_IDX        1

#define DI_DISP_HR        -20.0
#define DI_DISP_HR_2      -21.0

extern int getDIOptions (int , char * argv [], di_data_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_OPTIONS_H */
