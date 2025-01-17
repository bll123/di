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
#define DI_KILO           0
#define DI_MEGA           1
#define DI_GIGA           2
#define DI_TERA           3
#define DI_PETA           4
#define DI_EXA            5
#define DI_ZETTA          6
#define DI_YOTTA          7
#define DI_RONNA          8
#define DI_QUETTA         9

/* human readable */
#define DI_HR             -20

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
#define DI_DISP_DEVNAME     1
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
#define DI_OPT_OUT_CSV          2
#define DI_OPT_OUT_CSV_TAB      3
#define DI_OPT_EXCL_LOOPBACK    4
#define DI_OPT_OUT_JSON         5
#define DI_OPT_OUT_TOTALS       6
#define DI_OPT_OUT_DBG_HEADER   7
#define DI_OPT_OUT_HEADER       8
#define DI_OPT_DISP_ALL         9
#define DI_OPT_LOCAL_ONLY       10
#define DI_OPT_NO_SYMLINK       11
#define DI_OPT_MAX              12
#define DI_OPT_FMT_STR_LEN      13

#define DI_FMT_ITER_STOP        -1

/* string formats */
#define DI_FMT_MOUNT           'm'
#define DI_FMT_DEVNAME         'd'
#define DI_FMT_FSTYPE          't'
#define DI_FMT_MOUNT_OPTIONS   'O'
/* these will be processed (forever probably), but are no longer used */
#define DI_FMT_MOUNT_OLD       'M'
#define DI_FMT_DEVNAME_OLD     's'
#define DI_FMT_DEVNAME_OLD_B   'S'
#define DI_FMT_FSTYPE_OLD      'T'

/* disk information */
#define DI_FMT_BTOT            'b'
#define DI_FMT_BTOT_AVAIL      'B'
#define DI_FMT_BUSED           'u'
#define DI_FMT_BCUSED          'c'
#define DI_FMT_BFREE           'f'
#define DI_FMT_BAVAIL          'v'
#define DI_FMT_BPERC_NAVAIL    'p'
#define DI_FMT_BPERC_USED      '1'
#define DI_FMT_BPERC_BSD       '2'
#define DI_FMT_BPERC_AVAIL     'a'
#define DI_FMT_BPERC_FREE      '3'
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
extern int di_process_options (void *di_data, int argc, char * argv []);
extern void di_get_data (void *di_data);
extern void di_cleanup (void *di_data);
extern int di_iterate_init (void *di_data, int iteropt);
extern di_pub_disk_info_t *di_iterate (void *di_data);
extern int di_check_option (void *di_data, int optidx);
extern void di_format_iter_init (void *di_data);
extern int di_format_iterate (void *di_data);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DI_H */
