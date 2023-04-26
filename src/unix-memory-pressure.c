#include "meter.h"

#include <assert.h>
#include <pthread.h>
#include <sys/mman.h>

// Try to get this much memory to see how long it takes. 100MB is around
// what one web page seems to be using on my machine.
//
// Used in measureMemoryPressureThread().
#define BYTES (1024L * 1024L * 100L)

// Measure (poll) memory pressure this often
#define SECONDS 5

static pthread_mutex_t memoryPressureMutex;
static uint64_t memoryPressure = -1;

static void cycle_memory() {
    // Allocate 100MB of memory
    char *mem = mmap(NULL, BYTES, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Write to one byte in every page
    for (int i = 0; i < BYTES; i += page_size) {
        mem[i] = 1;
    }

    // Hand the memory back to the OS
    munmap(mem, BYTES);
}

static void *measureMemoryPressureThread() {
  const long page_size = sysconf(_SC_PAGESIZE);
  const long preallocated_size = BYTES / page_size;
  char *preallocated = mmap(NULL, BYTES, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // Used for making sure each write we do to our preallocated page go
  // out to RAM, like (I assume) the new-pages writes do. So this locking
  // is an attempt at making our filling of the preallocated RAM CPU-load
  // comparable to the filling of the new RAM in cycle_memory().
  pthread_mutex_t localMutex;
  pthread_mutex_init(&localMutex, NULL);

  while (1) {
    struct timespec t0, t1, t2;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t0);

    int err = cycle_memory();
    if (err != 0) {
      assert(0 == pthread_mutex_lock(&memoryPressureMutex));
      memoryPressure = memoryPressureMaxedOut;
      assert(0 == pthread_mutex_unlock(&memoryPressureMutex));
      continue;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

    for (int i = 0; i < preallocated_size; i++) {
      // Use locking to make sure all writes are flushed to RAM
      assert(0 == pthread_mutex_lock(&localMutex));
      preallocated[i] = 1;
      assert(0 == pthread_mutex_unlock(&localMutex));
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

    uint64_t cycle_us = (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_nsec - t0.tv_nsec) / 1000;
    uint64_t only_fill_us = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000;
    uint64_t factor = cycle_us / only_fill_us;

    assert(0 == pthread_mutex_lock(&memoryPressureMutex));
    memoryPressure = factor;
    assert(0 == pthread_mutex_unlock(&memoryPressureMutex));

    sleep(SECONDS);
  }
}

void initMemoryPressure(meter_sysload_t *load) {
  load->memoryPressureLow = memoryPressureUnknown;
  load->memoryPressureLowWatermark = memoryPressureUnknown;
  load->memoryPressureHighWatermark = memoryPressureUnknown;

  assert(0 == pthread_mutex_init(&memoryPressureMutex, NULL));

  pthread_t threadId;
  assert(0 == pthread_create(&threadId, NULL, measureMemoryPressureThread, NULL));
}

int64_t getMemoryPressure() {
  uint64_t returnMe;
  assert(0 == pthread_mutex_lock(&memoryPressureMutex));
  returnMe = memoryPressure;
  assert(0 == pthread_mutex_unlock(&memoryPressureMutex));
  return returnMe;
}
