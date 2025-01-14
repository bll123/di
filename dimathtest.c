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

int
main (int argc, char *argv [])
{
  dinum_t   a;
  dinum_t   b;
  dinum_t   r;
  double    dval;
  char      buff [200];
  int       tcount = 0;
  int       ecount = 0;


#if _use_math == DI_GMP
  fprintf (stdout, "GMP:\n");
#elif _use_math == DI_TOMMATH
  fprintf (stdout, "TOMMATH:\n");
#else
  fprintf (stdout, "UINT: ld:%d d:%d u64:%d ll:%d l:%d\n", _siz_long_double, _siz_double, _siz_uint64_t, _siz_long, _siz_long_long);
#endif

  dinum_init (&a);
  dinum_init (&b);
  dinum_init (&r);

  /* set */
  dinum_set_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 1) != 0) {
    fprintf (stderr, "%d: set-u 1 != 0 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  ++tcount;
  if (dinum_cmp_s (&a, 0) <= 0) {
    fprintf (stderr, "%d: set-u 1 <= 0 fail %d\n", tcount, dinum_cmp_s (&a, 0));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", tcount, buff);
    ++ecount;
  }

  /* set negative, must use _s suffix */
  dinum_set_s (&a, -1);
  ++tcount;
  if (dinum_cmp_s (&a, -1) != 0) {
    fprintf (stderr, "%d: set-s -1 != 0 fail %d\n", tcount, dinum_cmp_s (&a, -1));
    ++ecount;
  }
  ++tcount;
  if (dinum_cmp_s (&a, 0) >= 0) {
    fprintf (stderr, "%d: set-s -1 >= 0 fail %d\n", tcount, dinum_cmp_s (&a, 0));
    ++ecount;
  }

  /* add */
  dinum_set_u (&a, 0);
  dinum_add_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 1) != 0) {
    fprintf (stderr, "%d: add-u 1 != 0 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  ++tcount;
  if (dinum_cmp_s (&a, 0) <= 0) {
    fprintf (stderr, "%d: add-u 1 <= 0 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_add_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 2) != 0) {
    fprintf (stderr, "%d: add-u 0 fail %d\n", tcount, dinum_cmp_s (&a, 0));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "2") != 0) {
    fprintf (stderr, "%d: str 2 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_add_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 3) != 0) {
    fprintf (stderr, "%d: add-u 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "3") != 0) {
    fprintf (stderr, "%d: str 3 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_set_u (&b, 1);
  dinum_add (&a, &b);
  ++tcount;
  if (dinum_cmp_s (&a, 4) != 0) {
    fprintf (stderr, "%d: add 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "4") != 0) {
    fprintf (stderr, "%d: str 4 fail %s\n", tcount, buff);
    ++ecount;
  }

  /* subtract */
  dinum_sub_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 3) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "3") != 0) {
    fprintf (stderr, "%d: str 3 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_sub (&a, &b);
  ++tcount;
  if (dinum_cmp_s (&a, 2) != 0) {
    fprintf (stderr, "%d: sub 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "2") != 0) {
    fprintf (stderr, "%d: str 2 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_sub_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 1) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1") != 0) {
    fprintf (stderr, "%d: str 1 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_sub_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, 0) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "0") != 0) {
    fprintf (stderr, "%d: str 0 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_sub_u (&a, 1);
  ++tcount;
  if (dinum_cmp_s (&a, -1) != 0) {
    fprintf (stderr, "%d: sub-u 1 fail %d\n", tcount, dinum_cmp_s (&a, 1));
    ++ecount;
  }

  /* multiply */
  dinum_mul_uu (&a, 1, 1024);
  ++tcount;
  if (dinum_cmp_s (&a, 1024) != 0) {
    fprintf (stderr, "%d: mul-uu 1024 fail %d\n", tcount, dinum_cmp_s (&a, 1024));
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1024") != 0) {
    fprintf (stderr, "%d: str 1024 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_set_u (&b, 1024);
  dinum_mul (&a, &b);
  ++tcount;
  if (dinum_cmp_s (&a, 1024 * 1024) != 0) {
    fprintf (stderr, "%d: mul 1024*1024 fail %d\n", tcount, dinum_cmp_s (&a, 1024 * 1024) );
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1048576") != 0) {
    fprintf (stderr, "%d: str 1048576 fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_mul_u (&a, 1024);
  ++tcount;
  if (dinum_cmp_s (&a, 1024 * 1024 * 1024) != 0) {
    fprintf (stderr, "%d: mul-u 1024*1024*1024 fail %d\n", tcount, dinum_cmp_s (&a, 1024*1024*1024) );
    ++ecount;
  }
  dinum_str (&a, buff, sizeof (buff));
  ++tcount;
  if (strcmp (buff, "1073741824") != 0) {
    fprintf (stderr, "%d: str 1073741824 fail %s\n", tcount, buff);
    ++ecount;
  }

  /* scaled value tests */
  dinum_set_u (&a, 1024 * 1024);
  dinum_set_u (&b, 1024);
  dinum_scale (&r, &a, &b);
  ++tcount;
  if (dinum_cmp_s (&r, 1024) != 0) {
    dinum_str (&r, buff, sizeof (buff));
    fprintf (stderr, "%d: scale 1024 a fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_set_u (&a, 1024 * 1024 + 510);
  dinum_set_u (&b, 1024);
  dinum_scale (&r, &a, &b);
  ++tcount;
  if (dinum_cmp_s (&r, 1025) != 0) {
    dinum_str (&r, buff, sizeof (buff));
    fprintf (stderr, "%d: scale 1024 b fail %s\n", tcount, buff);
    ++ecount;
  }

  dinum_set_u (&a, 1024 * 1024 + 512);
  dinum_set_u (&b, 1024);
  dinum_scale (&r, &a, &b);
  ++tcount;
  if (dinum_cmp_s (&r, 1025) != 0) {
    dinum_str (&r, buff, sizeof (buff));
    fprintf (stderr, "%d: scale 1024 c fail %s\n", tcount, buff);
    ++ecount;
  }

  /* percentage tests */
  dval = 1024 * 1024 * 0.01;
  dinum_set_u (&a, (di_unum_t) dval);
  dinum_set_u (&b, 1024 * 1024);
  dval = dinum_perc (&a, &b);
  ++tcount;
  if (fabs (dval - 1.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 a fail %.4f\n", tcount, dval);
    ++ecount;
  }

  dval = 1024 * 1024 * 0.5;
  dinum_set_u (&a, (di_unum_t) dval);
  dinum_set_u (&b, 1024 * 1024);
  dval = dinum_perc (&a, &b);
  ++tcount;
  if (fabs (dval - 50.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 b fail %.4f\n", tcount, dval);
    ++ecount;
  }

  dinum_set_u (&a, 1024 * 1024);
  dinum_set_u (&b, 1024 * 1024);
  dval = dinum_perc (&a, &b);
  ++tcount;
  if (fabs (dval - 100.0) > 0.0001) {
    fprintf (stderr, "%d: perc 1024 c fail %.4f\n", tcount, dval);
    ++ecount;
  }

  dinum_clear (&a);
  dinum_clear (&b);
  dinum_clear (&r);
  fprintf (stdout, "tests: %d errors: %d\n", tcount, ecount);
  return ecount;
}
