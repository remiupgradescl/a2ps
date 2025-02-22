/* xbackupfile.c - backup helping routines
   Copyright 1990-2017 Free Software Foundation, Inc.

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

/* Written by Akim Demaille <demaille@inf.enst.fr> */

#include <config.h>

#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include "xbackupfile.h"
#include "routines.h"
#include "error.h"
#include "quotearg.h"


/* Non failing call to find_backup_file_name */
char *
xfind_backup_file_name (const char * file, enum backup_type backup_type)
{
  char * res;

  res = find_backup_file_name (AT_FDCWD, file, backup_type);
  if (!res)
    error (1, 0, _("memory exhausted"));

  return res;
}

/*
  The following approach has been suggested by Miles Bader and
  Paul Eggert.

From: Paul Eggert <eggert@twinsun.com>
Date: Wed, 16 Sep 1998 14:07:47 -0700 (PDT)

   From: Miles Bader <miles@ccs.mt.nec.co.jp>
   Date: Wed, 16 Sep 1998 10:15:34 +0900 (JST)

   In general, if you can get away with it, it's better *not* to stat files
   and then abort based on information you find; instead, try to simply
   structure the algorithm so that the actual operation which makes the
   change fails in an abortable way, and report *that* failure.  This
   avoids race conditions.

This is a good suggestion.  Here's an idea for doing this.  When you
first open the file F for writing, use O_EXCL.  This will work
correctly if F is a directory, since the initial open call will fail
with errno==EISDIR in that case.  Some older systems don't have
O_EXCL; on those systems, you'll have to approximate it with `stat',
but that's the best that you can do.

I believe that demaille wants to back up only regular files (e.g. he
doesn't want to back up /dev/null or /dev/tty, or back up
directories).  In that case, there's an unavoidable race condition,
since there's no way to say `rename this file only if it's a regular
file' atomically.  But it's still better to use O_EXCL; this will
avoid some of the race conditions.

  The following code was written by Paul Eggert.
*/

/* Create FILE for output, using open flags OFLAG and default mode MODE.
   But before creating FILE, back up its previous contents if necessary.
   OFLAG must include the O_CREAT flag.  */
int
create_file_for_backup (char const *file, int oflag, mode_t mode,
			enum backup_type backup_type)
{
  int fd;
  struct stat st;

  assert (oflag & O_CREAT);

  if (backup_type == no_backups)
    return open (file, oflag, mode);

  fd = open (file, oflag | O_EXCL, mode);

  if (fd < 0 && errno == EEXIST)
    {
      /* We want to back up only regular files, even though this
         allows races between `stat' and `open'.  */
      if (stat (file, &st) == 0 && S_ISREG (st.st_mode))
	{
	  char *backup = find_backup_file_name (AT_FDCWD, file, backup_type);
	  if (rename (file, backup) != 0)
	    return -1;
	}

      fd = open (file, oflag, mode);
    }

  return fd;
}

/* Since there were two different approaches, and one may fear
 * portability problems, the old version is here provided by disabled.
 * Follows the solution based on Miles Bader and Paul Eggert's
 * approach.  */

/*======================
 * I could not make it behave the way I want (it works for
 * a mode 000, which I don't want), so this code the old code will
 * be used instead
 ========================*/
#define USE_OLD_FOPEN_BACKUP 1

#ifdef USE_OLD_FOPEN_BACKUP
/* Open a stream for writing on the file FILENAME, making, if
   necessary and adequate, a backup according the policy BACKUP_TYPE */
FILE *
fopen_backup (const char * filename, enum backup_type backup_type)
{
  char * backup_name = NULL;
  struct stat filestat;
  FILE * res;

  /* No backup upon non existing files */
  if (stat (filename, &filestat))
    {
      if ((errno == ENOENT)
	  || (errno == ENOTDIR))
	/* the file does not exist: return */
	backup_type = no_backups ;
      else
	/* Another kind of error occured: exit */
	error (1, errno, _("cannot get informations on file `%s'"),
	       quotearg (filename));
    }

  /* If the file is special (/dev/null etc.), don't backup.
     Or if we don't have the rights to open the file for writing, don't
     backup, so that the forthcoming fopen does complain on the rights*/
  if (!S_ISREG (filestat.st_mode)
      || access (filename, W_OK))
    backup_type = no_backups ;

  /* Definitely, make a backup */
  if (backup_type != no_backups)
    {
      backup_name = xfind_backup_file_name (filename, backup_type);
      if (rename (filename, backup_name))
	error (1, errno, _("cannot rename file `%s' as `%s'"),
	       quotearg (filename), quotearg (backup_name));
    }

  /* Open the file for reading */
  res = fopen (filename, "w");
  if (!res)
    {
      error (0, errno, _("cannot create file `%s'"), quotearg (filename));
      if (backup_name)
	{
	  if (rename (filename, backup_name))
	    error (0, errno, _("cannot rename file `%s' as `%s'"),
		   quotearg (filename), quotearg (backup_name));
	  else
	    fprintf (stderr, _("restored file `%s'"), quotearg (filename));
	}
      exit (EXIT_FAILURE);
    }

  return res;
}
#else /* !USE_OLD_FOPEN_BACKUP */
FILE *
fopen_backup (const char * filename, enum backup_type backup_type)
{
  FILE * res;
  int fd;

  fd = create_file_for_backup (filename, O_CREAT, 0666, backup_type);
  if (fd < 0)
    {
      if (backup_type == no_backups)
	error (1, errno, _("cannot create file `%s'"), quotearg (filename));
      else
	error (1, errno, ("cannot backup and create file `%s'"),
	       quotearg (filename));
    }

  res = fdopen (fd, "w");
  if (!res)
    error (1, errno, _("cannot create file `%s'"), quotearg (filename));

  return res;
}
#endif /* !USE_OLD_FOPEN_BACKUP */
