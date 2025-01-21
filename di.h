/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef INC_DI_H
#define INC_DI_H

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* scaled values */
#define DI_SCALE_BYTE     0
#define DI_SCALE_KILO     1
#define DI_SCALE_MEGA     2
#define DI_SCALE_GIGA     3
#define DI_SCALE_TERA     4
#define DI_SCALE_PETA     5
#define DI_SCALE_EXA      6
#define DI_SCALE_ZETTA    7
#define DI_SCALE_YOTTA    8
#define DI_SCALE_RONNA    9
#define DI_SCALE_QUETTA   10
#define DI_SCALE_MAX      11
/* human readable */
#define DI_SCALE_HR       -20
#define DI_SCALE_HR_ALT   -21

/* print flags */
#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2
#define DI_PRNT_OUTOFZONE   3
#define DI_PRNT_EXCLUDE     4
#define DI_PRNT_FORCE       5
#define DI_PRNT_SKIP        6

/* string identifiers */
#define DI_DISP_MOUNTPT     0
#define DI_DISP_FILESYSTEM  1
#define DI_DISP_FSTYPE      2
#define DI_DISP_MOUNTOPT    3
#define DI_DISP_MAX         4

/* value identifiers */
#define DI_SPACE_TOTAL      0
#define DI_SPACE_FREE       1
#define DI_SPACE_AVAIL      2
#define DI_INODE_TOTAL      3
#define DI_INODE_FREE       4
#define DI_INODE_AVAIL      5
#define DI_VALUE_MAX        6
#define DI_VALUE_NONE       -1

/* options return values */
#define DI_EXIT_NORM      0
#define DI_EXIT_HELP      1
#define DI_EXIT_VERS      2
#define DI_EXIT_WARN      3
#define DI_EXIT_FAIL      4

/* iterator options */
/* these match the true/false value of the DI_OPT_DISP_ALL value */
#define DI_ITER_PRINTABLE 0
#define DI_ITER_ALL       1

/* di options */
#define DI_OPT_POSIX_COMPAT     0
#define DI_OPT_QUOTA_CHECK      1
#define DI_OPT_DISP_CSV         2
#define DI_OPT_DISP_CSV_TAB     3
#define DI_OPT_EXCL_LOOPBACK    4
#define DI_OPT_DISP_JSON        5
#define DI_OPT_DISP_TOTALS      6
#define DI_OPT_DISP_DBG_HEADER  7
#define DI_OPT_DISP_HEADER      8
#define DI_OPT_DISP_ALL         9
#define DI_OPT_LOCAL_ONLY       10
#define DI_OPT_NO_SYMLINK       11
#define DI_OPT_MAX              12
#define DI_OPT_FMT_STR_LEN      13
#define DI_OPT_SCALE            14
#define DI_OPT_BLOCK_SZ         15

#define DI_FMT_ITER_STOP        -1

/* format string characters */
/* strings */
#define DI_FMT_MOUNT           'm'
#define DI_FMT_FILESYSTEM      's'
#define DI_FMT_FSTYPE          't'
#define DI_FMT_MOUNT_OPTIONS   'O'
/* these will be processed (forever probably), but are no longer used */
#define DI_FMT_MOUNT_FULL      'M'
#define DI_FMT_FILESYSTEM_FULL 'S'
#define DI_FMT_FSTYPE_FULL     'T'
/* space */
#define DI_FMT_BTOT            'b'
#define DI_FMT_BTOT_AVAIL      'B'
#define DI_FMT_BUSED           'u'
#define DI_FMT_BCUSED          'c'
#define DI_FMT_BFREE           'f'
#define DI_FMT_BAVAIL          'v'
/* percentages */
#define DI_FMT_BPERC_NAVAIL    'p'
#define DI_FMT_BPERC_USED      '1'
#define DI_FMT_BPERC_BSD       '2'
#define DI_FMT_BPERC_AVAIL     'a'
#define DI_FMT_BPERC_FREE      '3'
/* inodes */
#define DI_FMT_ITOT            'i'
#define DI_FMT_IUSED           'U'
#define DI_FMT_IFREE           'F'
#define DI_FMT_IPERC           'P'

#define DI_FMT_MAX            19

typedef struct
{
  const char    *strdata [DI_DISP_MAX];     /* mount point           */
                                            /* special device name   */
                                            /* type of file system   */
                                            /* mount options         */
  int           index;                      /* the index for this entry */
  int           doPrint;                    /* should this entry        */
                                            /*   be printed?            */
  int           printFlag;                  /* print flags              */
  int           isLocal;                    /* is this mount point      */
                                            /*   local?                 */
  int           isReadOnly;                 /* is this mount point      */
                                            /*   read-only?             */
  int           isLoopback;                 /* lofs or none fs type?    */
} di_pub_disk_info_t;

/* dilib.c */
extern void * di_initialize (void);
extern int di_process_options (void *, int, char * []);
extern void di_get_all_disk_info (void *);
extern void di_cleanup (void *);
extern int di_iterate_init (void *, int);
extern di_pub_disk_info_t *di_iterate (void *);
extern int di_check_option (void *, int);
extern void di_format_iter_init (void *);
extern int di_format_iterate (void *);
extern int di_get_scale_max (void *, int, int, int, int);
extern double di_get_scaled (void *, int, int, int, int, int);
extern void di_disp_scaled (void *, char *, long, int, int, int, int, int);
extern double di_get_perc (void *, int, int, int, int, int, int);
extern void di_disp_perc (void *, char *, long, int, int, int, int, int, int);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DI_H */
