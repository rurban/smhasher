project(SMHasher)

cmake_minimum_required(VERSION 2.4)
cmake_policy(SET CMP0003 NEW)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the type of build, options are: Debug Release
  RelWithDebInfo MinSizeRel."
  FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
   EXEC_PROGRAM(cat ARGS "/proc/cpuinfo" OUTPUT_VARIABLE CPUINFO)
   STRING(REGEX REPLACE "^.*(sse2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "sse2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(sse4_2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "sse4_2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-elf64.o)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   EXEC_PROGRAM("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE
      CPUINFO)
   STRING(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-macho64.o)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
   EXEC_PROGRAM("grep Features /var/run/dmesg.boot" OUTPUT_VARIABLE
      CPUINFO)
   STRING(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-elf64.o)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
# TODO: Windows, BSD and ARM8 crc detection
# TODO: 32bit filter

IF (SSE2_TRUE)
   set(SSE2_FOUND true CACHE BOOL "SSE2 available")
ELSE (SSE2_TRUE)
   set(SSE2_FOUND false CACHE BOOL "SSE2 not available")
ENDIF (SSE2_TRUE)
IF (SSE42_TRUE)
   set(SSE4_2_FOUND true CACHE BOOL "SSE4.2 available")
ELSE (SSE42_TRUE)
  set(SSE4_2_FOUND false CACHE BOOL "SSE4.2 not available")
ENDIF (SSE42_TRUE)

IF(SSE4_2_FOUND)
  set(CMAKE_C_FLAGS   "-msse2 -msse4.2")
  set(CMAKE_CXX_FLAGS "-msse2 -msse4.2")
  set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
  set(SSE2_SRC        hasshe2.c)
  set(SSE4_SRC        crc32_hw.c)
# 64bit only:
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  #enable_language(ASM-NASM)
  #set(CMAKE_ASM_FLAGS "-f elf64")
  set(SSE4_SRC1       crc32_hw1.c)
  set(METROCRC64_SRC  metrohash64crc.cpp)
  set(METROCRC128_SRC metrohash128crc.cpp)
  set(SSE4_OBJ        ${FALKHASH_OBJ})
ENDIF()
  set(SIPHASH_SRC     siphash_ssse3.c)
ELSEIF(SSE2_FOUND)
  set(CMAKE_C_FLAGS   "-msse2")
  set(CMAKE_CXX_FLAGS "-msse2")
  set(SSE2_SRC        hasshe2.c)
  set(SIPHASH_SRC     siphash_sse2.c)
ELSE()
  set(SIPHASH_SRC     siphash.c)
ENDIF(SSE4_2_FOUND)

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(X64ONLY_SRC     xxhash.c metrohash64.cpp metrohash128.cpp farmhash.cc
                      FarmTest.cc farmhash-c.c farmhash-c-test.cc cmetrohash64.c opt_cmetrohash64_1.c
                      fasthash.cpp)
ENDIF()

add_library(
  SMHasherSupport
  AvalancheTest.cpp
  Bitslice.cpp
  Bitvec.cpp
  CityTest.cpp
  City.cpp
  crc.cpp
  DifferentialTest.cpp
  Hashes.cpp
  ${SSE2_SRC}
  ${SSE4_SRC} 
  ${SSE4_SRC1}
  KeysetTest.cpp
  lookup3.cpp
  md5.cpp
  MurmurHash1.cpp
  MurmurHash2.cpp
  MurmurHash3.cpp
  Platform.cpp
  Random.cpp
  sha1.cpp
  ${SIPHASH_SRC}
  SpeedTest.cpp
  Spooky.cpp
  SpookyTest.cpp
  Stats.cpp
  SuperFastHash.cpp
  Types.cpp
  PMurHash.c
  ${X64ONLY_SRC}
  ${METROCRC64_SRC} 
  ${METROCRC128_SRC}
  ${SSE4_OBJ}
  t1ha.cc
)

add_executable(
  SMHasher
  main.cpp
)

target_link_libraries(
  SMHasher
  SMHasherSupport
  ${CMAKE_THREAD_LIBS_INIT}
)
