cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0003 NEW)

project(SMHasher C CXX)

include(CheckCCompilerFlag)
# Check if the same compile family is used for both C and CXX
if (NOT (CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID))
    message(WARNING "CMAKE_C_COMPILER_ID (${CMAKE_C_COMPILER_ID}) is different "
                    "from CMAKE_CXX_COMPILER_ID (${CMAKE_CXX_COMPILER_ID}). "
                    "The final binary may be unusable.")
endif()

IF (CMAKE_MAJOR_VERSION GREATER 2)
  IF (CMAKE_MAJOR_VERSION GREATER 3 OR CMAKE_MINOR_VERSION GREATER 1)
    cmake_policy(SET CMP0056 OLD) # since 3.2. Honor link flags in try_compile() source-file signature.
  ENDIF()
  IF (CMAKE_MAJOR_VERSION GREATER 3 OR CMAKE_MINOR_VERSION GREATER 3)
    cmake_policy(SET CMP0065 NEW) # since 3.4. ENABLE_EXPORTS.
  ENDIF()
  IF (CMAKE_MAJOR_VERSION GREATER 3 OR CMAKE_MINOR_VERSION GREATER 6)
    cmake_policy(SET CMP0066 OLD) # since 3.7. Honor per-config flags in try_compile() source-file signature.
  ENDIF()

  include (TestBigEndian)
  TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
  if(IS_BIG_ENDIAN)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DBIG_ENDIAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBIG_ENDIAN")
  endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the type of build, options are: Debug Release
  RelWithDebInfo MinSizeRel."
  FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# TODO: rather parse `$CC -march=native -dM -E - <<< ''` [gh #10]
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
   EXEC_PROGRAM(cat ARGS "/proc/cpuinfo" OUTPUT_VARIABLE CPUINFO)
   STRING(REGEX REPLACE "^.*(sse2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "sse2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(sse4_2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "sse4_2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-elf64.o)
   IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
     set(FHTW_OBJ        fhtw-elf64.o)
   ENDIF()
   STRING(REGEX REPLACE "^.*(aes).*$" "\\1" AES_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "aes" "${AES_THERE}" AES_TRUE)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   EXEC_PROGRAM("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE
      CPUINFO)
   STRING(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-macho64.o)
   IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
     set(FHTW_OBJ        fhtw-macho64.o)
   ENDIF()
   STRING(REGEX REPLACE "^.*(AES).*$" "\\1" AES_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "AES" "${AES_THERE}" AES_TRUE)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
   EXEC_PROGRAM("grep Features /var/run/dmesg.boot" OUTPUT_VARIABLE
      CPUINFO)
   STRING(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
   STRING(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
   set(FALKHASH_OBJ      falkhash-elf64.o)
   IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
     set(FHTW_OBJ        fhtw-elf64.o)
   ENDIF()
   STRING(REGEX REPLACE "^.*(AES).*$" "\\1" AES_THERE ${CPUINFO})
   STRING(COMPARE EQUAL "AES" "${AES_THERE}" AES_TRUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

if (CMAKE_COMPILER_IS_GNUCC
    OR (CMAKE_C_COMPILER_ID STREQUAL AppleClang)
    OR (CMAKE_C_COMPILER_ID STREQUAL Clang)
    OR (CMAKE_C_COMPILER_ID STREQUAL Intel))
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
  set(CMAKE_C_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_C_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  #IF (SSE2_TRUE)
  #  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -msse2")
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
  #ENDIF (SSE2_TRUE)
  #IF (SSE42_TRUE)
  #  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -msse4")
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4")
  #ENDIF (SSE42_TRUE)
  #IF (AES_TRUE)
  #  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -maes")
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes")
  #ENDIF (AES_TRUE)
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -march=native")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
elseif (MSVC)
  # using Visual Studio C++, already the default with VS17
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /arch:SSE2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
else()
  message(WARNING "Unknown ${CMAKE_CXX_COMPILER_ID} compiler")
# TODO: BSD and ARM8 crc detection
# TODO: 32bit filter
endif()

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
IF (AES_TRUE)
   set(AES_FOUND true CACHE BOOL "AES-NI available")
ELSE (AES_TRUE)
  set(AES_FOUND false CACHE BOOL "AES-NI not available")
ENDIF (AES_TRUE)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_BIT32")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_BIT32")
ENDIF()

IF(AES_FOUND AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  set(SSE4_OBJ        ${FALKHASH_OBJ})
ENDIF()

IF(SSE4_2_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SSE42")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_SSE42")
  SET(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(SSE2_SRC        hasshe2.c)
  set(SSE4_SRC        crc32_hw.c)
  # 64bit only:
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    #enable_language(ASM-NASM)
    #set(CMAKE_ASM_FLAGS "-f elf64") # or macho64/win64
    set(X86_64ONLY_SRC farmhash-c.c farmhash-c-test.cc metrohash64crc.cpp
                       metrohash128crc.cpp crc32_hw1.c clhash.c)
  ELSE()
    message(WARNING "32bit only: CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
  ENDIF()
  set(SIPHASH_SRC     siphash_ssse3.c)
ELSEIF(SSE2_FOUND)
  set(SSE2_SRC        hasshe2.c)
  set(SIPHASH_SRC     siphash_sse2.c)
ELSE()
  set(SIPHASH_SRC     siphash.c)
ENDIF(SSE4_2_FOUND)

IF(AES_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_AESNI")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_AESNI")
  set(T1HA_SRC
    t1ha/t1ha0.c t1ha/t1ha1.c t1ha/t1ha2.c
    t1ha/t1ha0_ia32aes_noavx.c t1ha/t1ha0_ia32aes_avx.c
    t1ha/t1ha0_ia32aes_avx2.c)
ELSE()
  set(T1HA_SRC t1ha/t1ha0.c t1ha/t1ha1.c t1ha/t1ha2.c)
ENDIF()
if(MSVC)
set_source_files_properties(t1ha/t1ha0_ia32aes_noavx.c PROPERTIES COMPILE_FLAGS "/arch:ia32")
set_source_files_properties(t1ha/t1ha0_ia32aes_avx.c PROPERTIES COMPILE_FLAGS "/arch:avx")
set_source_files_properties(t1ha/t1ha0_ia32aes_avx2.c PROPERTIES COMPILE_FLAGS "/arch:avx2")
else()
set_source_files_properties(t1ha/t1ha0_ia32aes_noavx.c PROPERTIES COMPILE_FLAGS "-mno-avx2 -mno-avx -maes")
set_source_files_properties(t1ha/t1ha0_ia32aes_avx.c PROPERTIES COMPILE_FLAGS "-mno-avx2 -mavx -maes")
set_source_files_properties(t1ha/t1ha0_ia32aes_avx2.c PROPERTIES COMPILE_FLAGS "-mavx -mavx2 -maes")
endif()

find_library(HIGHWAY highwayhash)
if (HIGHWAY)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(HIGHWAY_SRC HighwayHash.cpp)
    set(HIGHWAY_LIB highwayhash)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_HIGHWAYHASH")
  endif()
endif()

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
  ${HIGHWAY_SRC}
  ${SSE2_SRC}
  ${SSE4_SRC}
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
  fasthash.cpp
  xxhash.c
  metrohash64.cpp
  metrohash128.cpp
  cmetrohash64.c opt_cmetrohash64_1.c
  farmhash.cc FarmTest.cc
  ${SSE4_OBJ}
  # ${FHTW_OBJ}
  ${T1HA_SRC}
  mum.cc
  jody_hash32.c
  jody_hash64.c
  ${X86_64ONLY_SRC}
)

add_executable(
  SMHasher
  main.cpp
)

target_link_libraries(
  SMHasher
  SMHasherSupport
  ${HIGHWAY_LIB}
  ${CMAKE_THREAD_LIBS_INIT}
)
