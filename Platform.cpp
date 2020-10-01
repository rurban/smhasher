#include "Platform.h"

#include <stdio.h>

void testRDTSC ( void )
{
  int64_t temp = rdtsc();
  printf("%ld",(long)temp);
}

#if defined(_WIN32)

#include <windows.h>

void SetAffinity ( int cpu )
{
  SetProcessAffinityMask(GetCurrentProcess(),cpu);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

#if NCPU > 1
void SetThreadAffinity ( std::thread &t, int cpu )
{
    SetThreadIdealProcessor((HANDLE)t.native_handle(), (DWORD)cpu);
}
#endif

#else

#include <sched.h>

void SetAffinity ( int /*cpu*/ )
{
#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(__FreeBSD__)
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(2,&mask);
  if( sched_setaffinity(0,sizeof(mask),&mask) == -1)
  {
    printf("WARNING: Could not set CPU affinity\n");
  }
#endif
}

#if NCPU > 1
void SetThreadAffinity ( std::thread &t, int cpu )
{
#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(__FreeBSD__)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
  thread_affinity_policy_data_t policy = { cpu };
  thread_policy_set(pthread_mach_thread_np(t.native_handle()), THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
#endif
}
#endif

#endif


// From https://encode.su/threads/3389-Code-snippet-to-compute-CPU-frequency
// Perform CYCLES simple in-order operations
static unsigned cycles_loop(unsigned cycles)
{
    unsigned a = rand(),  b = rand(),  x = rand();
    for (unsigned i=0; i < cycles/10; i++)
    {
        x = (x + a) ^ b;
        x = (x + a) ^ b;
        x = (x + a) ^ b;
        x = (x + a) ^ b;
        x = (x + a) ^ b;
    }
    return x;
}

#ifdef __linux__
#include <string>
#include <iostream>
#include <fstream>
#endif

double cpu_freq()
{
  const unsigned cycles = 100*1000*1000;
  unsigned x = cycles_loop(cycles/10);  // warmup
#ifdef __linux__
  std::ifstream curfreqfile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
  if (curfreqfile.is_open()) {
    std::string line;
    getline(curfreqfile, line);
    long res = strtol(line.c_str(), NULL, 10);
    curfreqfile.close();
    fprintf (stderr, "scaling_cpu_freq: %ld\n", res);
    fflush(NULL);
    if (res > 1000)
      return (double) res / 1000000.0; // 1801310, 2452626
  }
#endif
  double result = 3.0;
  const uint64_t t1 = timer_start();
  x += cycles_loop(cycles);
  const uint64_t t2 = timer_end();
  // constant adjustment factor, when compared to scaling_cur_freq
  // ADJ = (res / 1000000.0) / (t2-t1)
  const double ADJ = 236163824.32526;
  if (x)
    result = (double) (ADJ / (t2-t1));
  if (result < 0.001) {
    fprintf (stderr, "%u pseudo-cycles, %lu time-cycles\n", cycles, (unsigned long)(t2-t1));
    return 3.0;
  }
  return result;
  //printf("CPU freq %.2f GHz", (cycles/1e9)/(t2-t1));
}
