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
 * This is the platform independent network load refinery functions.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "netload.h"
#include "bubblemon.h"

static int netload_percentage = 0;
static int receivingUpdates = 0;
static struct netload_interface *interfaces;

static struct netload_interface *netload_findInterface(const char *name)
{
  struct netload_interface *iter;

  assert(name != NULL);
  
  for (iter = interfaces; iter != NULL; iter = iter->next)
  {
    if (strcmp(iter->name, name) == 0)
    {
      return iter;
    }
  }

  return NULL;
}

static struct netload_interface *netload_forcedFindInterface(const char *name)
{
  struct netload_interface *interface;

  interface = netload_findInterface(name);
  if (interface == NULL)
  {
    interface = (struct netload_interface *)malloc(sizeof(struct netload_interface));

    interface->name = strdup(name);

    interface->currentBytesIn = 0;
    interface->previousBytesIn = 0;
    interface->maxBytesInPerSecond = 0;

    interface->currentBytesOut = 0;
    interface->previousBytesOut = 0;
    interface->maxBytesOutPerSecond = 0;

    interface->isAlive = 0;

    interface->next = interfaces;
    interfaces = interface;
  }

  return interface;
}

void netload_reportBack(const char *name, unsigned long bytesSent, unsigned long bytesReceived)
{
  struct netload_interface *interface;
  
  assert(receivingUpdates &&
	 "netload_reportBack() called from outside netload_reportNetworkLoad()");
  
  // Grab a pointer to the struct holding this interface's data.  If
  // there is no such struct, construct and initialize a new one.
  interface = netload_forcedFindInterface(name);
  
  // Verify that the interface isn't marked live.  If it is, it means
  // we have received two reports about the same interface.
  assert(!interface->isAlive &&
	 "netload_reportBack() called twice on the same interface");
  
  // Verify that the incoming values are >= the ones we already got.
  assert(bytesSent >= interface->currentBytesOut &&
	 bytesReceived >= interface->currentBytesIn &&
	 "The interface byte count has moved backwards");
  
  // Store the incoming values.
  interface->previousBytesIn = interface->currentBytesIn;
  interface->currentBytesIn = bytesReceived;
  // Newly added interfaces don't get credit for any bytes during the
  // first iteration
  if (interface->previousBytesIn == 0)
  {
    interface->previousBytesIn = interface->currentBytesIn;
  }

  interface->previousBytesOut = interface->currentBytesOut;
  interface->currentBytesOut = bytesSent;
  // Newly added interfaces don't get credit for any bytes during the
  // first iteration
  if (interface->previousBytesOut == 0)
  {
    interface->previousBytesOut = interface->currentBytesOut;
  }

  // Mark the interface live
  interface->isAlive = 1;
}

static void netload_receiveUpdates()
{
  struct netload_interface *iter, *prev;
  
  // Mark all known interfaces as shut down
  for (iter = interfaces;
       iter != NULL;
       iter = iter->next)
  {
    iter->isAlive = 0;
  }

  // Ask the platform dependent function to report network loads to us
  receivingUpdates = 1;
  netload_reportNetworkLoad();
  receivingUpdates = 0;

  // Forget about all interfaces marked as shut down.
  while (interfaces != NULL && !interfaces->isAlive)
  {
    struct netload_interface *temp = interfaces->next;
    free(interfaces->name);
    free(interfaces);
    interfaces = temp;
  }
  if (interfaces != NULL)
  {
    for (prev = interfaces, iter = interfaces->next;
	 iter != NULL;
	 prev = iter, iter = iter->next)
    {
      if (!iter->isAlive)
      {
	prev->next = iter->next;
	free(iter->name);
	free(iter);
	iter = prev->next;
      }
    }
  }
}

static int netload_calculateLoad(int lastIntervalLength)
{
  struct netload_interface *iter;
  int maxLoad = 0;
  
  // For all interfaces
  for (iter = interfaces; iter != NULL; iter = iter->next)
  {
    unsigned long bytesPerSec;
    int load;
    
    // Calculate incoming load
    bytesPerSec = ((iter->currentBytesIn - iter->previousBytesIn) * 1000) / lastIntervalLength;
    if (bytesPerSec > iter->maxBytesInPerSecond)
    {
      iter->maxBytesInPerSecond = bytesPerSec;
    }
    if (iter->maxBytesInPerSecond > 0)
    {
      load = (100 * bytesPerSec) / iter->maxBytesInPerSecond;
    }
    else
    {
      load = 0;
    }
    if (load > maxLoad)
    {
      maxLoad = load;
    }
    
    // Calculate outgoing load
    bytesPerSec = ((iter->currentBytesOut - iter->previousBytesOut) * 1000) / lastIntervalLength;
    if (bytesPerSec > iter->maxBytesOutPerSecond)
    {
      iter->maxBytesOutPerSecond = bytesPerSec;
    }
    if (iter->maxBytesOutPerSecond > 0)
    {
      load = (100 * bytesPerSec) / iter->maxBytesOutPerSecond;
    }
    else
    {
      load = 0;
    }
    if (load > maxLoad)
    {
      maxLoad = load;
    }
  }
  
  // Return the largest percentage
  return maxLoad;
}

void netload_updateLoadstats(int msecsSinceLastCall)
{
  static int timeToNextMeasurement = 0;
  
  // If enough time has elapsed since the last measurement...
  timeToNextMeasurement -= msecsSinceLastCall;
  if (timeToNextMeasurement <= 0)
  {
    int intervalLength = NETLOAD_INTERVAL - timeToNextMeasurement;

    netload_receiveUpdates();
    
    netload_percentage = netload_calculateLoad(intervalLength);
    
    timeToNextMeasurement += NETLOAD_INTERVAL;
  }
}

int netload_getLoadPercentage(void)
{
  assert((netload_percentage >= 0) &&
	 (netload_percentage <= 100));

  return netload_percentage;
}
