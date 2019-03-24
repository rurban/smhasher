// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Created by Alexander Gryanko on 16/09/2017.

#ifndef HIGHWAYHASH_OS_MAC_H_
#define HIGHWAYHASH_OS_MAC_H_

#ifndef HH_DISABLE_TARGET_SPECIFIC

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/sysctl.h>

#include <mach/mach_types.h>
#include <mach/thread_act.h>
#include <pthread.h>

typedef unsigned long int cpu_mask;

#define SYSCTL_CORE_COUNT "machdep.cpu.thread_count"
#define NR_CPUS 512 // from the linux kernel limit
#define NR_CPUBITS (8 * sizeof(cpu_mask))

struct cpu_set_t {
  cpu_mask bits[NR_CPUS / NR_CPUBITS];
};

static inline void CPU_ZERO(size_t setsize, cpu_set_t* set) {
  memset(set, 0, sizeof(cpu_set_t));
}

static inline int CPU_ISSET(int cpu, const cpu_set_t* set) {
  if (cpu < NR_CPUS) {
    return (set->bits[cpu / NR_CPUBITS] & 1L << (cpu % NR_CPUBITS)) != 0;
  }
  return 0;
}

static inline void CPU_SET(int cpu, cpu_set_t* set) {
  if (cpu < NR_CPUS) {
    set->bits[cpu / NR_CPUBITS] |= 1L << (cpu % NR_CPUBITS);
  }
}

int mac_getaffinity(cpu_set_t* set);
int mac_setaffinity(cpu_set_t* set);

#endif // !HH_DISABLE_TARGET_SPECIFIC
#endif // HIGHWAYHASH_OS_MAC_H_
