/* Based on example code from Wikipedia */
/* https://en.wikipedia.org/wiki/Gcov */
#include <stdio.h>
#include "../code/gcov_public.h"

int
main (void)
{
  int i;

  // Because we are running this example under unix,
  // do not need __gcov_call_constructors() here,
  // unix application startup will accomplish that.

  for (i = 1; i < 10; i++)
    {
      if (i % 3 == 0)
        printf ("%d is divisible by 3\n", i);
      if (i % 11 == 0)
        printf ("%d is divisible by 11\n", i);
    }

  // Because we are running this example under unix,
  // the unix application shutdown will call
  // __gcov_exit() once per source file,
  // so several copies of the gcov output
  // will be produced.
  // In most embedded systems, that would not happen,
  // instead you would insert your own single
  // call to __gcov_exit() here,
  // to produce one copy of the gcov output.

  return 0;
}
