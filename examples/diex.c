/* this code is in the public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <di.h>

enum {
  HAVE_SPACE = 0,
  NO_SPACE = 1,
};

int
check_space (const char *fn, double space_wanted) {
  void        *di_data;
  int         targc;
  const char  *targv [10];
  int         exitflag;
  const di_pub_disk_info_t  *pub;
  int         rval = NO_SPACE;
  int         count;

  targc = 1;
  targv [0] = fn;
  targv [1] = NULL;

  di_data = di_initialize ();

  exitflag = di_process_options (di_data, targc, targv, 0);
  if (exitflag != DI_EXIT_NORM) {
    di_cleanup (di_data);
    exit (exitflag);
  }

  exitflag = di_get_all_disk_info (di_data);
  if (exitflag != DI_EXIT_NORM) {
    di_cleanup (di_data);
    exit (exitflag);
  }

  count = di_iterate_init (di_data, DI_ITER_PRINTABLE);
  while ((pub = di_iterate (di_data)) != NULL) {
    double    dval;

    /* compare terabytes available */
    dval = di_get_scaled (di_data, pub->index, DI_SCALE_TERA,
        DI_SPACE_AVAIL, DI_VALUE_NONE, DI_VALUE_NONE);
    if (dval >= space_wanted) {
      rval = HAVE_SPACE;
    }
  }
  di_cleanup (di_data);

  return rval;
}

int
main (int argc, char *argv []) {
  const char  *fn = NULL;
  double      spwant = 0.2;   /* terabytes */
  bool        rval;

  if (argc > 1) {
    fn = argv [1];
  }
  if (fn == NULL) {
    fprintf (stderr, "Usage: %s <path> [<space-wanted>]\n", argv [0]);
    exit (1);
  }

  if (argc > 2) {
    spwant = atof (argv [2]);
  }

  rval = check_space (fn, spwant);
  if (rval == NO_SPACE) {
    fprintf (stdout, "Not enough disk space\n");
  }
  if (rval == HAVE_SPACE) {
    fprintf (stdout, "Enough disk space\n");
  }

  return rval;
}

