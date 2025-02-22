/* xstrrpl.c - replacement of substrings
   Copyright 1988-2017 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Author: Akim Demaille <demaille@inf.enst.fr> */

#include <config.h>

#include "system.h"
#include <assert.h>
#include "xstrrpl.h"

/* Perform subsitutions in string.  Result is malloc'd
   E.g., result = xstrrrpl ("1234", subst) gives result = "112333"
   where subst = { {"1", "11"}, {"3", "333"}, { "4", ""}}
   */
char *
xstrrpl (const char * string, const char * subst[][2])
{
  char * res, * cp;
  int i;

  size_t max = 0;
  for (i = 0 ; subst[i][0] ; i++)
    max = (max > strlen (subst[i][1])) ? max : strlen (subst[i][1]);

  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  res = XNMALLOC (strlen (string) * (max + 1) + 1, char);
  cp = res;

  /* Perform the substitutions */
  while (*string)
    {
      for (i = 0 ; subst[i][0] ; i++)
	if (!strncmp (subst[i][0], string, strlen (subst[i][0])))
	  {
	    cp = stpcpy (cp, subst[i][1]);
	    string += strlen (subst[i][0]);
	    goto __next_char;
	  }
      *cp++ = *string++;
    __next_char:
      /* nothing */;
    }

  *cp = '\0';
#if DEBUG
  assert (strlen (string) * (max + 1) < cp - res);
#endif

  res = xnrealloc (res, (size_t) (cp - res + 1), sizeof(char));

  return res;
}

/* Perform subsitution in string.  String is untouched, result is malloc'd
   E.g., result = xstrrrpl ("1234", "1", "11", "3", "333", "4", "", NULL)
   gives result = "112333"
   */
char *
xvstrrpl (const char * string, ...)
{
  va_list ap;
  const char * subst[100][2];	/* Hope this is enough :) */
  int i = 0;

  /* Copy arguments into `args'. */
  va_start (ap, string);
  for (i = 0 ; (subst[i / 2][i % 2] = va_arg (ap, char *)) ; i++)
    ;
  va_end (ap);
  if (i % 2)
    return NULL;
  return xstrrpl (string, subst);
}
