/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2008 Johan Walles - johan.walles@gmail.com
 *  http://www.nongnu.org/bubblemon/
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
#include <string.h>
#include <pwd.h>

#include "mail.h"

/* Check $MAIL for new mail on every N calls to mail_hasUnreadMail. */
static const int CHECKEVERYNTH = 100;

// Returns 1 if fileName seems to be a mail spool file, and 0
// otherwise.  Currently just verifies that the permissions and file
// type look sane.
static int isUserMailSpoolFile(const char *fileName)
{
  struct stat fileInfo;
  uid_t currentUser = getuid();
  unsigned int userReadWrite = S_IRUSR | S_IWUSR;

  // Does this directory entry exist?
  if (stat(fileName, &fileInfo) != 0)
  {
    return 0;
  }

  // Is it a regular file?
  if (S_ISREG(fileInfo.st_mode))
  {
    // Is it owned by the current user?
    if (fileInfo.st_uid != currentUser)
    {
      return 0;
    }

    // Does the current user have read / write access to it?
    if ((fileInfo.st_mode & userReadWrite) != userReadWrite)
    {
      return 0;
    }

    return 1;
  }

  // Is it /dev/null?
  if (S_ISCHR(fileInfo.st_mode))
  {
    // We could make a more thorough check here, but until somebody
    // reports they are having problems with this, I'll just assume
    // people who use a character device as spool file are actually
    // using /dev/null.  //Johan, 2005jan16
    return 1;
  }

  return 0;
}

// Returns the name of the user's mail spool file, or NULL if no spool
// file could be located.
static char *getMailFileName(void)
{
  static char *mailFileName = NULL;
  const char *varSpoolMail = "/var/spool/mail/";
  struct passwd *userinfo;

  if (mailFileName != NULL)
  {
    goto done;
  }

  mailFileName = getenv("MAIL");
  if (mailFileName != NULL && isUserMailSpoolFile(mailFileName))
  {
    mailFileName = strdup(mailFileName);
    goto done;
  }

  userinfo = getpwuid(getuid());
  if (userinfo == NULL)
  {
    // How do we get here?  By having the current user removed from
    // the passwd file?
    mailFileName = "";
    goto done;
  }

  mailFileName = (char*)malloc(sizeof(char)
			       * (strlen(varSpoolMail)
				  + strlen(userinfo->pw_name)
				  + 1));
  strcpy(mailFileName, varSpoolMail);
  strcat(mailFileName, userinfo->pw_name);
  if (isUserMailSpoolFile(mailFileName)) {
    goto done;
  }

  strcpy(mailFileName, "/var/mail/");
  strcat(mailFileName, userinfo->pw_name);
  if (isUserMailSpoolFile(mailFileName)) {
    goto done;
  }

  free(mailFileName);
  mailFileName = "";

 done:
  return mailFileName[0] == '\0' ? NULL : mailFileName;
}

mail_status_t mail_getMailStatus(void)
{
  static int countdown = 0;
  static mail_status_t cachedMailState = 0;
  char *mailFileName;

  struct stat mailStat;

  if (countdown > 0)
  {
    countdown--;
    return cachedMailState;
  }
  countdown = CHECKEVERYNTH;

  mailFileName = getMailFileName();
  if (mailFileName == NULL)
  {
    cachedMailState = NO_MAIL;
    return cachedMailState;
  }

  if (stat(mailFileName, &mailStat) != 0)
  {
    // Checking the file dates on the spool file failed
    cachedMailState = NO_MAIL;
    return cachedMailState;
  }

  if (mailStat.st_size == 0) {
    /* No mail */
    cachedMailState = NO_MAIL;
  } else {
    /* New mail has arrived if the mail file has been updated after it
       was last read from */
    cachedMailState = ((mailStat.st_mtime > mailStat.st_atime)
		       ? UNREAD_MAIL
		       : READ_MAIL);
  }

  return cachedMailState;
}
