#ifndef DI_INC_OPTIONS_H
#define DI_INC_OPTIONS_H

/*
 * Copyright 2023-2024 Brad Lanam, Pleasant Hill, CA
 */

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

    /* these are indexes into the dispTable array... */
#define DI_ONE_K                0
#define DI_ONE_MEG              1
#define DI_ONE_GIG              2
#define DI_ONE_TERA             3
#define DI_ONE_PETA             4
#define DI_ONE_EXA              5
#define DI_ONE_ZETTA            6
#define DI_ONE_YOTTA            7
#define DI_DISP_HR              -20.0
#define DI_DISP_HR_2            -21.0

extern int getDIOptions (int , const char * const [], diData_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_OPTIONS_H */
