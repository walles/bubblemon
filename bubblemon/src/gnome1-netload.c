/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004 Johan Walles - walles@mailblocks.com
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

/*
 * This is the Gnome 1 specific network load measuring stuff.
 */

#include <stdlib.h>
#include <stdio.h>
#include <glibtop/netload.h>

#include "netload.h"

// This is a workaround for my inability to get libgtop to tell me
// what interfaces exist.  All of these interfaces will be queried for
// their current load.
static const char *interface_candidates[] =
{
  "eth0",
  "eth1",
  "eth2",
  "le0",
  "le1",
  "le2",
  "lo",
  "ppp0",
  "ppp1",
  "ppp2",
  NULL
};

void netload_reportNetworkLoad(void)
{
  int i;

  for (i = 0; interface_candidates[i] != NULL; i++)
  {
    glibtop_netload load;

    load.bytes_total = 0;
    glibtop_get_netload(&load, interface_candidates[i]);
    if (load.bytes_total == 0)
    {
      // Interface doesn't exist or hasn't seen any traffic
      continue;
    }

    if (load.bytes_total == load.bytes_in + load.bytes_out)
    {
      // The platform keeps separate tabs on incoming and outgoing traffic
      netload_reportBack(interface_candidates[i], load.bytes_out, load.bytes_in);
    }
    else
    {
      // The platform only keeps track of total traffic / interface
      netload_reportBack(interface_candidates[i], load.bytes_total, 0);
    }
  }
  
  return;
}
