/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2000 Johan Walles - d92-jwa@nada.kth.se
 *  http://www.nada.kth.se/~d92-jwa/code/#bubblemon
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "mail.h"

/* Perform an actual check for new mail on every N calls to
 * mail_hasUnreadMail. */
static const int CHECKEVERYNTH = 100;

int mail_hasUnreadMail(void)
{
  static int countdown = 0;
  static char *mailFileName = NULL;
  static int cachedMailState = 0;

  struct stat mailStat;

  if (countdown > 0)
  {
    countdown--;
    return cachedMailState;
  }
  countdown = CHECKEVERYNTH;

  if (mailFileName == NULL)
  {
    mailFileName = getenv("MAIL");
  }

  if (mailFileName == NULL)
  {
    // We don't know where to look for new mail
    return 0;
  }

  if (stat(mailFileName, &mailStat) != 0)
  {
    // Checking the file dates on the spool file failed
    return 0;
  }

  // New mail has arrived if the mail file has been updated after it
  // was last read from
  cachedMailState = mailStat.st_mtime > mailStat.st_atime;

  return cachedMailState;
}
