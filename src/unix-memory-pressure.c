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

static void *measureMemoryPressureThread() {
  const long page_size = sysconf(_SC_PAGESIZE);
  const long preallocated_size = BYTES / page_size;
  char *preallocated = mmap(NULL, BYTES, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  pthread_mutex_t localMutex;
  pthread_mutex_init(&localMutex, NULL);

  while (1) {
    struct timespec t0, t1, t2;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t0);

    cycle_memory();

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

    for (int i = 0; i < preallocated_size; i++) {
      // Use locking to make sure all writes are flushed to RAM
      pthread_mutex_lock(&localMutex);
      preallocated[i] = 1;
      pthread_mutex_unlock(&localMutex);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

    uint64_t cycle_us = (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_nsec - t0.tv_nsec) / 1000;
    uint64_t only_fill_us = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000;
    uint64_t factor = cycle_us / only_fill_us;

    pthread_mutex_lock(&memoryPressureMutex);
    memoryPressure = factor;
    pthread_mutex_unlock(&memoryPressureMutex);

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
