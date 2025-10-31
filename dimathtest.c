/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#include "dimath.h"
#include "dimath_mp.h"

int
main (int argc, char *argv [])
{
  dinum_t   a;
  dinum_t   b;
  dinum_t   r;
  double    dval;
  di_ui_t   ival;
  char      buff [200];
  int       testcount = 0;
  int       errcount = 0;

#if _use_math == DI_GMP
  fprintf (stdout, "GMP:\n");
#elif _use_math == DI_TOMMATH
  fprintf (stdout, "TOMMATH:\n");
#elif _use_math == DI_MPDECIMAL
  fprintf (stdout, "MPDECIMAL:\n");
#else
  fprintf (stdout, "INTERNAL: ld:%d d:%d u64:%d ll:%d l:%d\n", _siz_long_double, _siz_double, _siz_uint64_t, _siz_long_long, _siz_long);
#endif

  dimath_initialize ();

  dinum_init (&a);
  dinum_init (&b);
  dinum_init (&r);

  /* set_u, cmp_s */
  dinum_set_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 1) != 0) {
    fprintf (stderr, "%d: cmp-s 1 != 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 0) <= 0) {
    fprintf (stderr, "%d: cmp-s 1 > 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 0));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 2) >= 0) {
    fprintf (stderr, "%d: cmp-s 1 < 2 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 2));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* set */
  dinum_set (&b, &a);
  ++testcount;
  if (dinum_cmp_s (&b, (di_si_t) 1) != 0) {
    fprintf (stderr, "%d: cmp-s 1 != 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&b, (di_si_t) 0) <= 0) {
    fprintf (stderr, "%d: cmp-s 1 > 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 0));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&b, (di_si_t) 2) >= 0) {
    fprintf (stderr, "%d: cmp-s 1 < 2 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 2));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp (&a, &b) != 0) {
    fprintf (stderr, "%d: cmp a/b fail %d\n", testcount, dinum_cmp (&a, &b));
    ++errcount;
  }
  dinum_str (&b, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* set negative, must use _s suffix */
  /* set_s */
  dinum_set_s (&a, (di_si_t) -1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) -1) != 0) {
    fprintf (stderr, "%d: set-s -1 != 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) -1));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 0) >= 0) {
    fprintf (stderr, "%d: set-s -1 >= 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 0));
    ++errcount;
  }

  /* set with neg */
  dinum_set (&b, &a);
  ++testcount;
  if (dinum_cmp_s (&b, (di_si_t) -1) != 0) {
    fprintf (stderr, "%d: set -1 != 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) -1));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&b, (di_si_t) 0) >= 0) {
    fprintf (stderr, "%d: set -1 >= 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 0));
    ++errcount;
  }

  /* add_u */
  dinum_set_u (&a, (di_ui_t) 0);
  dinum_add_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 1) != 0) {
    fprintf (stderr, "%d: add-u 1 != 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 0) <= 0) {
    fprintf (stderr, "%d: add-u 1 <= 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_add_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 2) != 0) {
    fprintf (stderr, "%d: add-u 0 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 0));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "2") != 0) {
    fprintf (stderr, "%d: str 2 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_add_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 3) != 0) {
    fprintf (stderr, "%d: add-u 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "3") != 0) {
    fprintf (stderr, "%d: str 3 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* add */
  dinum_set_u (&b, (di_ui_t) 1);
  dinum_add (&a, &b);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 4) != 0) {
    fprintf (stderr, "%d: add 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "4") != 0) {
    fprintf (stderr, "%d: str 4 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* sub_u */
  dinum_sub_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 3) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "3") != 0) {
    fprintf (stderr, "%d: str 3 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_sub (&a, &b);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 2) != 0) {
    fprintf (stderr, "%d: sub 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "2") != 0) {
    fprintf (stderr, "%d: str 2 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_sub_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 1) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_sub_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 0) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "0") != 0) {
    fprintf (stderr, "%d: str 0 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_sub_u (&a, (di_ui_t) 1);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) -1) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1));
    ++errcount;
  }

  /* mul_uu */
  dinum_mul_uu (&a, (di_ui_t) 1, (di_ui_t) 1024);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 1024) != 0) {
    fprintf (stderr, "%d: mul-uu 1024 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 1024));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1024") != 0) {
    fprintf (stderr, "%d: str 1024 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* mul */
  dinum_set_u (&b, (di_ui_t) 1024);
  dinum_mul (&a, &b);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) (1024 * 1024)) != 0) {
    fprintf (stderr, "%d: mul 1024*1024 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) (1024 * 1024)) );
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1048576") != 0) {
    fprintf (stderr, "%d: str 1048576 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_mul_u (&a, (di_ui_t) 1024);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) (1024 * 1024 * 1024)) != 0) {
    fprintf (stderr, "%d: mul-u 1024*1024*1024 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) (1024*1024*1024)) );
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "1073741824") != 0) {
    fprintf (stderr, "%d: str 1073741824 fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_mul_uu (&a, (di_ui_t) 3843826, (di_ui_t) 4096);
  ++testcount;
  if (dinum_cmp_s (&a, (di_si_t) 15744311296) != 0) {
    fprintf (stderr, "%d: mul-uu 15744311296 fail %d\n", testcount, dinum_cmp_s (&a, (di_si_t) 15744311296));
    ++errcount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++testcount;
  if (strcmp (buff, "15744311296") != 0) {
    fprintf (stderr, "%d: str 15744311296 fail %s\n", testcount, buff);
    ++errcount;
  }

  /* scaled value tests */
  dinum_set_u (&a, (di_ui_t) (1024 * 1024));
  dinum_set_u (&b, (di_ui_t) 1024);
  dval = dinum_scale (&a, &b);
  ++testcount;
  if (dval != 1024.0) {
    Snprintf1 (buff, sizeof (buff), "%.3f", dval);
    fprintf (stderr, "%d: scale 1024 a fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_set_u (&a, (di_ui_t) (1024 * 1024 + 2));
  dinum_set_u (&b, (di_ui_t) 1024);
  dval = dinum_scale (&a, &b);
  ++testcount;
  if (dval > 1025.0) {
    Snprintf1 (buff, sizeof (buff), "%.3f", dval);
    fprintf (stderr, "%d: scale 1025 b fail %s\n", testcount, buff);
    ++errcount;
  }

  dinum_set_u (&a, (di_ui_t) (1024 * 1024 + 512));
  dinum_set_u (&b, (di_ui_t) 1024);
  dval = dinum_scale (&a, &b);
  ++testcount;
  if (dval > 1025.0) {
    Snprintf1 (buff, sizeof (buff), "%.3f", dval);
    fprintf (stderr, "%d: scale 1025 c fail %s\n", testcount, buff);
    ++errcount;
  }

  /* percentage tests */
  ival = 1024 * 1024 / 100;
  dinum_set_u (&a, (di_ui_t) ival);
  dinum_set_u (&b, (di_ui_t) (1024 * 1024));
  dval = dinum_perc (&a, &b);
  ++testcount;
  if (fabs (dval - 1.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 a fail %.4f\n", testcount, dval);
    ++errcount;
  }

  ival = 1024 * 1024 / 2;
  dinum_set_u (&a, (di_ui_t) ival);
  dinum_set_u (&b, (di_ui_t) (1024 * 1024));
  dval = dinum_perc (&a, &b);
  ++testcount;
  if (fabs (dval - 50.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 b fail %.4f\n", testcount, dval);
    ++errcount;
  }

  dinum_set_u (&a, (di_ui_t) (1024 * 1024));
  dinum_set_u (&b, (di_ui_t) (1024 * 1024));
  dval = dinum_perc (&a, &b);
  ++testcount;
  if (fabs (dval - 100.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 c fail %.4f\n", testcount, dval);
    ++errcount;
  }

  dinum_clear (&a);
  dinum_clear (&b);
  dinum_clear (&r);
  fprintf (stdout, "tests: %d errors: %d\n", testcount, errcount);

  dimath_cleanup ();

  return errcount;
}
