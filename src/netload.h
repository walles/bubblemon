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

/*
 * This is the header file for network load measuring.
 */

#ifndef NETLOAD_H
#define NETLOAD_H

struct netload_interface
{
  char *name;
  
  unsigned long currentBytesIn;
  unsigned long previousBytesIn;
  unsigned long maxBytesInPerSecond;
  
  unsigned long currentBytesOut;
  unsigned long previousBytesOut;
  unsigned long maxBytesOutPerSecond;
  
  int isAlive;

  struct netload_interface *next;
} netload_interface;

void netload_updateLoadstats(int msecsSinceLastCall);
int netload_getLoadPercentage(void);

void netload_reportBack(const char *name, unsigned long bytesSent, unsigned long bytesReceived);

/* This platform dependent function must be implemented when porting
 * to a new platform.  For each network interface, it must call
 * netload_reportBack(), passing it the name of the interface, and the
 * total number of bytes sent / received since it was first
 * configured. */
void netload_reportNetworkLoad(void);

#endif // NETLOAD_H
