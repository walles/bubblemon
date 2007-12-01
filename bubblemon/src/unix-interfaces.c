/*
 * 
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2007 Johan Walles - johan.walles@gmail.com
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

#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include "interfaces.h"

/**
 * The socket we use to look for interfaces.
 */
static int querySocket = 0;

/**
 * Our array of interfaces.
 */
static struct ifreq *interfaces = NULL;

/**
 * The size of our interfaces array.
 */
static int interfacesSize = 0;

/**
 * Return the socket to query for network interface status, or 0 if we can't come up with
 * any socket.
 */
static int getSocket(void) {
  if (querySocket == 0) {
    querySocket = socket(PF_INET, SOCK_DGRAM, 0);
  }
  if (querySocket > 0) {
    return querySocket;
  } else {
    return 0;
  }
}

char **interfaces_getcandidates(void) {
  int error;
  int nInterfaces;
  char **interfaceNames;
  
  if (interfacesSize == 0) {
    interfacesSize = 1;
    interfaces =
      malloc(interfacesSize * sizeof(struct ifreq));
  }
  
  error = 0;
  while (1)
  {
    struct ifconf ifc;
    // Get at most maxNInterfaces...
    ifc.ifc_len = interfacesSize * sizeof(struct ifreq);
    // ... and put them in interfaces.
    ifc.ifc_req = interfaces;
    
    if (getSocket() == 0) {
      // Unable to get any info, give up.
      nInterfaces = 0;
      break;
    }
    if (ioctl(getSocket(), SIOCGIFCONF, &ifc) < 0) {
      error = errno;
      break;
    }
    
    nInterfaces = ifc.ifc_len / sizeof(*interfaces);
    if (nInterfaces < interfacesSize) {
      // Done, our array wasn't filled
      break;
    }
    
    interfacesSize *= 2;
    interfaces =
      realloc(interfaces, interfacesSize * sizeof(struct ifreq));
    assert(interfaces != NULL);
  }
  if (error != 0) {
    // Trouble, give up
    nInterfaces = 0;
  }
  
  interfaceNames = (char**)malloc((nInterfaces + 1) * sizeof(char*));
  assert(interfaceNames != NULL);
  int i;
  for (i = 0; i < nInterfaces; i++) {
    interfaceNames[i] = interfaces[i].ifr_name;
  }
  interfaceNames[nInterfaces] = NULL;
  
  return interfaceNames;
}

void interfaces_freecandidates(char** candidates) {
  free(candidates);
}
