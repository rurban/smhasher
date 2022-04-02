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
