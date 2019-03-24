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

#include "os_mac.h"

int mac_getaffinity(cpu_set_t* set) {
  int64_t core_count = 0;
  size_t core_count_size = sizeof(core_count); // size is a pointer
  const int err = sysctlbyname(SYSCTL_CORE_COUNT, &core_count,
                                 &core_count_size, NULL, 0);
  if (err != 0)
    return err;

  CPU_ZERO(set);
  for (int64_t i = 0; i < core_count; ++i) {
    CPU_SET(i, set);
  }

  return 0;
}

int mac_setaffinity(cpu_set_t* set) {
  thread_port_t thread = pthread_mach_thread_np(pthread_self());

  uint16_t current_core;
  for (current_core = 0; current_core < NR_CPUS; ++current_core) {
    if (CPU_ISSET(current_core, set)) break;
  }
  thread_affinity_policy_data_t policy = { current_core };
  return thread_policy_set(thread, THREAD_AFFINITY_POLICY,
                                                (thread_policy_t)&policy, 1);
}
