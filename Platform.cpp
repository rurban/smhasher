#include "Platform.h"
#include <memory>
#include <stdio.h>
#include <math.h> // for lrint
#include <assert.h>

long getenvlong(const char *name, long minval, long defval, long maxval)
{
  assert(minval <= defval && defval <= maxval);
  const char *s = getenv(name);
  if (!s)
    return defval;
  char *tail;
  long l = strtol(s, &tail, 0);
  if (*tail)
    return defval;
  if (l < minval) l = minval;
  if (l > maxval) l = maxval;
  return l;
}

struct StampedRdtsc
{
  const uint64_t ticks, ns;

  StampedRdtsc() : ticks(timer_start()), ns(timeofday()) { }

  bool GoodDelta(const StampedRdtsc& t) const {
    constexpr uint64_t ms20 = 20*1000*1000; // 20ms, two ticks of HZ=100
    constexpr uint64_t mln2 = 2*1000*1000; // 20ms @ 100 MHz = 2M ticks
    const uint64_t dtick = timer_sub(ticks, t.ticks);
    return mln2 <= dtick && dtick != timer_inf && ms20 <= (ns - t.ns);
  }

  unsigned int FreqMHzSince(const StampedRdtsc& t) const {
    return lrint(double(timer_sub(ticks, t.ticks)) / (ns - t.ns) * 1e9 / 1e6);
  }
};

static unsigned int CpuFreqMHz;
static std::unique_ptr<StampedRdtsc> BaseTS;

void SampleCpuFreq ( void )
{
  if (!BaseTS) {
    if (CpuFreqMHz)
      return;
    else
      BaseTS.reset(new StampedRdtsc());
  }
  StampedRdtsc now;
  if (now.GoodDelta(*BaseTS))
    CpuFreqMHz = now.FreqMHzSince(*BaseTS);
}

unsigned int GetCpuFreqMHz( void )
{
  if (BaseTS)
    BaseTS.reset();
  return CpuFreqMHz;
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

uint64_t timeofday(void)
{
  // GetTickCount               ~ Windows 2000+
  // GetTickCount64             ~ Vista+, Server 2008+
  // QueryUnbiasedInterruptTime ~ Windows 7+, Server 2008 R2
  // see https://learn.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
  unsigned long long ns100;
  if (QueryUnbiasedInterruptTime(&ns100))
    return ns100 * 100u;
  return GetTickCount64() * 1000000U;
}

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
