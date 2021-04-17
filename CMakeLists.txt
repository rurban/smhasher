cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0003 NEW)
if(POLICY CMP0056) # Honor link flags in try_compile() source-file signature.
  cmake_policy(SET CMP0056 NEW) # since 3.2
endif()
if(POLICY CMP0065)
  cmake_policy(SET CMP0065 NEW) # since 3.4. ENABLE_EXPORTS.
endif()
if(POLICY CMP0066) # honor per-config flags in try_compile() source-file
                   # signature.
  cmake_policy(SET CMP0066 NEW) # since 3.7
endif()

project(SMHasher C CXX ASM)

include(CheckCCompilerFlag)
# Check if the same compile family is used for both C and CXX
if(NOT (CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID))
  message(WARNING "CMAKE_C_COMPILER_ID (${CMAKE_C_COMPILER_ID}) is different "
                  "from CMAKE_CXX_COMPILER_ID (${CMAKE_CXX_COMPILER_ID}). "
                  "The final binary may be unusable.")
endif()

if(CMAKE_MAJOR_VERSION GREATER 2)
  include(TestBigEndian)
  test_big_endian(IS_BIG_ENDIAN)
  if(IS_BIG_ENDIAN)
    add_definitions(-DBIG_ENDIAN)
  endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose the type of build, options are: Debug Release
  RelWithDebInfo MinSizeRel Asan." FORCE)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

include(CheckTypeSize)
check_type_size(__int64 __INT64)
check_type_size(int64_t INT64_T)
check_type_size(__uint128_t __UINT128_T)

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
if ((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "i586")
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64") # freebsd
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64") # windows
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86"))
  set(PROCESSOR_FAMILY "Intel")
elseif ((CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))
  set(PROCESSOR_FAMILY "Arm")
  add_definitions(-DHAVE_NEON)
endif()

# TODO: rather parse `$CC -march=native -dM -E - <<< ''` [GH #10]
if(NOT CMAKE_CROSSCOMPILING)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    exec_program(
      cat ARGS
      "/proc/cpuinfo"
      OUTPUT_VARIABLE CPUINFO)
    string(REGEX REPLACE "^.*(sse2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "sse2" "${SSE_THERE}" SSE2_TRUE)
    string(REGEX REPLACE "^.*(sse4_2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "sse4_2" "${SSE_THERE}" SSE42_TRUE)
    set(FALKHASH_OBJ falkhash-elf64.o)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(FHTW_OBJ fhtw-elf64.o)
    endif()
    string(REGEX REPLACE "^.*(aes).*$" "\\1" AES_THERE ${CPUINFO})
    string(COMPARE EQUAL "aes" "${AES_THERE}" AES_TRUE)
    string(REGEX REPLACE "^.*(pclmulqdq).*$" "\\1" CLMUL_THERE ${CPUINFO})
    string(COMPARE EQUAL "pclmulqdq" "${CLMUL_THERE}" CLMUL_TRUE)
    string(REGEX REPLACE "^.* (sha_ni) .*$" "\\1" SHA_THERE ${CPUINFO})
    string(COMPARE EQUAL "sha_ni" "${SHA_THERE}" SHA_TRUE)
    string(REGEX REPLACE "^.* (avx2) .*$" "\\1" AVX2_THERE ${CPUINFO})
    string(COMPARE EQUAL "avx2" "${AVX2_THERE}" AVX2_TRUE)
    string(REGEX REPLACE "^.* (avx512f) .*$" "\\1" AVX512F_THERE ${CPUINFO})
    string(COMPARE EQUAL "avx512f" "${AVX512F_THERE}" AVX512F_TRUE)
    string(REGEX REPLACE "^.* (avx512vl) .*$" "\\1" AVX512VL_THERE ${CPUINFO})
    string(COMPARE EQUAL "avx512vl" "${AVX512VL_THERE}" AVX512VL_TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    exec_program("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE
                 CPUINFO)
    string(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
    string(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
    set(FALKHASH_OBJ falkhash-macho64.o)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(FHTW_OBJ fhtw-macho64.o)
    endif()
    string(REGEX REPLACE "^.*(AES).*$" "\\1" AES_THERE ${CPUINFO})
    string(COMPARE EQUAL "AES" "${AES_THERE}" AES_TRUE)
    string(REGEX REPLACE "^.*(SHA).*$" "\\1" SHA_THERE ${CPUINFO})
    string(COMPARE EQUAL "SHA" "${SHA_THERE}" SHA_TRUE)
    string(REGEX REPLACE "^.*(PCLMUL).*$" "\\1" CLMUL_THERE ${CPUINFO})
    string(COMPARE EQUAL "PCLMUL" "${CLMUL_THERE}" CLMUL_TRUE)
    string(REGEX REPLACE "^.* (AVX2) .*$" "\\1" AVX2_THERE ${CPUINFO})
    string(COMPARE EQUAL "AVX2" "${AVX2_THERE}" AVX2_TRUE)
    string(REGEX REPLACE "^.* (AVX512) .*$" "\\1" AVX512_THERE ${CPUINFO})
    string(COMPARE EQUAL "AVX512" "${AVX512_THERE}" AVX512_TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    exec_program("grep Features /var/run/dmesg.boot" OUTPUT_VARIABLE CPUINFO)
    string(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
    string(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE ${CPUINFO})
    string(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
    set(FALKHASH_OBJ falkhash-elf64.o)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(FHTW_OBJ fhtw-elf64.o)
    endif()
    string(REGEX REPLACE "^.*(AES).*$" "\\1" AES_THERE ${CPUINFO})
    string(COMPARE EQUAL "AES" "${AES_THERE}" AES_TRUE)
    string(REGEX REPLACE "^.*(PCLMULQDQ).*$" "\\1" CLMUL_THERE ${CPUINFO})
    string(COMPARE EQUAL "PCLMULQDQ" "${CLMUL_THERE}" CLMUL_TRUE)
    string(REGEX REPLACE "^.*(SHA).*$" "\\1" SHA_THERE ${CPUINFO}) # SHA1,SHA2
                                                                   # actually
    string(COMPARE EQUAL "SHA" "${SHA_THERE}" SHA_TRUE)
    string(REGEX REPLACE "^.* (AVX2) .*$" "\\1" AVX2_THERE ${CPUINFO})
    string(COMPARE EQUAL "AVX2" "${AVX2_THERE}" AVX2_TRUE)
    string(REGEX REPLACE "^.* (AVX512) .*$" "\\1" AVX512_THERE ${CPUINFO})
    string(COMPARE EQUAL "AVX512" "${AVX512_THERE}" AVX512_TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(_vendor_id)
    set(_cpu_family)
    set(_cpu_model)
    get_filename_component(
      _vendor_id
      "[HKEY_LOCAL_MACHINE\\Hardware\\Description\\System\\CentralProcessor\\0;VendorIdentifier]"
      NAME
      CACHE)
    get_filename_component(
      _cpu_id
      "[HKEY_LOCAL_MACHINE\\Hardware\\Description\\System\\CentralProcessor\\0;Identifier]"
      NAME
      CACHE)
    mark_as_advanced(_vendor_id _cpu_id)
    string(REGEX REPLACE ".* Family ([0-9]+) .*" "\\1" _cpu_family "${_cpu_id}")
    string(REGEX REPLACE ".* Model ([0-9]+) .*" "\\1" _cpu_model "${_cpu_id}")
    if(_vendor_id STREQUAL "GenuineIntel")
      if(_cpu_family EQUAL 6)
        set(SSE2_TRUE TRUE)
        set(SSE42_TRUE TRUE)
        set(AES_TRUE TRUE)
        set(CLMUL_TRUE TRUE) # since westmore
        if(_cpu_model GREATER_EQUAL 92) # Goldmont, Goldmont Plus, Tremont
          set(SHA_TRUE TRUE)
        endif()
      elseif(_cpu_family EQUAL 15) # NetBurst
        set(SSE2_TRUE TRUE)
        set(SSE42_TRUE TRUE)
      endif()
    elseif(_vendor_id STREQUAL "AuthenticAMD")
      if(_cpu_family GREATER 14)
        set(SSE2_TRUE TRUE)
        set(SSE42_TRUE TRUE)
        set(AES_TRUE TRUE)
        set(CLMUL_TRUE TRUE)
        if(_cpu_family GREATER_EQUAL 23) # zen (ryzen,epyc,...) -march=znver1
          set(AVX2_TRUE TRUE)
          set(SHA_TRUE TRUE)
        endif()
      endif()
    endif()
  endif()
endif(NOT CMAKE_CROSSCOMPILING)

if(PROCESSOR_FAMILY STREQUAL "Intel")
 if(CMAKE_COMPILER_IS_GNUCC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_ASM_FLAGS "${CFLAGS} -m64")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CMAKE_ASM_FLAGS "${CFLAGS} -m32")
  endif()
 elseif((CMAKE_C_COMPILER_ID STREQUAL AppleClang) OR (CMAKE_C_COMPILER_ID
                                                     STREQUAL Clang))
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_ASM_FLAGS "${CFLAGS} -arch x86_64")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CMAKE_ASM_FLAGS "${CFLAGS} -arch i386")
  endif()
 endif()
endif()

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1 -DHAVE_ASAN"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
          FORCE)
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1 -DHAVE_ASAN"
    CACHE STRING
          "Flags used by the C++ compiler during AddressSanitizer builds."
          FORCE)

if(CMAKE_COMPILER_IS_GNUCC
   OR (CMAKE_C_COMPILER_ID STREQUAL AppleClang)
   OR (CMAKE_C_COMPILER_ID STREQUAL Clang)
   OR (CMAKE_C_COMPILER_ID STREQUAL Intel))
  if(CLMUL_TRUE)
    set(CRC_PCLMUL_SRC crc32-pclmul_asm.S)
  endif()
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
  set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined -DHAVE_ASAN -DHAVE_UBSAN"
  )
  set(CMAKE_LINKER_FLAGS_DEBUG
      "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined"
  )
  # IF (SSE2_TRUE) set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -msse2")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2") ENDIF (SSE2_TRUE) IF
  # (SSE42_TRUE) set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -msse4")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4") ENDIF (SSE42_TRUE) IF
  # (AES_TRUE) set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -maes") set(CMAKE_CXX_FLAGS
  # "${CMAKE_CXX_FLAGS} -maes") ENDIF (AES_TRUE) IF (SHA_TRUE) set(CMAKE_C_FLAGS
  # "${CMAKE_C_FLAGS} -msha") set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msha")
  # ENDIF (SHA_TRUE)
  if(NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  endif()
  set_source_files_properties(blake3/blake3.c
                              PROPERTIES COMPILE_FLAGS "-std=c99")
elseif(MSVC)
  # using Visual Studio C++, already the default with VS17
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
else()
  message(WARNING "Unknown ${CMAKE_CXX_COMPILER_ID} compiler")
  # TODO: BSD and ARM8 crc detection TODO: 32bit filter
endif()

if(SSE2_TRUE)
  set(SSE2_FOUND
      true
      CACHE BOOL "SSE2 available")
else(SSE2_TRUE)
  set(SSE2_FOUND
      false
      CACHE BOOL "SSE2 not available")
  message(WARNING "SSE2 not available")
endif(SSE2_TRUE)
if(SSE42_TRUE)
  set(SSE42_FOUND
      true
      CACHE BOOL "SSE4.2 available")
else(SSE42_TRUE)
  set(SSE42_FOUND
      false
      CACHE BOOL "SSE4.2 not available")
  message(WARNING "SSE4.2 not available")
endif(SSE42_TRUE)
if(AES_TRUE)
  set(AES_FOUND
      true
      CACHE BOOL "AES-NI available")
else(AES_TRUE)
  set(AES_FOUND
      false
      CACHE BOOL "AES-NI not available")
  message(STATUS "AES-NI not available")
endif(AES_TRUE)
if(CLMUL_TRUE)
  set(CLMUL_FOUND
      true
      CACHE BOOL "CLMUL available")
else()
  set(CLMUL_FOUND
      false
      CACHE BOOL "CLMUL not available")
endif()
if(SHA_TRUE)
  set(SHA_FOUND
      true
      CACHE BOOL "SHA-NI available")
else(SHA_TRUE)
  set(SHA_FOUND
      false
      CACHE BOOL "SHA-NI not available")
  message(STATUS "SHA-NI not available")
endif(SHA_TRUE)
if(AVX2_TRUE)
  set(AVX2_FOUND
      true
      CACHE BOOL "AVX2 available")
else(AVX2_TRUE)
  set(AVX2_FOUND
      false
      CACHE BOOL "AVX2 not available")
  message(STATUS "AVX2 not available")
endif(AVX2_TRUE)
if(AVX512F_TRUE)
  set(AVX512F_FOUND
      true
      CACHE BOOL "AVX512F available")
else(AVX512F_TRUE)
  set(AVX512F_FOUND
      false
      CACHE BOOL "AVX512F not available")
  message(STATUS "AVX512F not available")
endif(AVX512F_TRUE)
if(AVX512VL_TRUE)
  set(AVX512VL_FOUND
      true
      CACHE BOOL "AVX512VL available")
else(AVX512VL_TRUE)
  set(AVX512VL_FOUND
      false
      CACHE BOOL "AVX512VL not available")
  message(STATUS "AVX512VL not available")
endif(AVX512VL_TRUE)

if(CMAKE_CROSSCOMPILING)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(SSE2_FOUND true)
    set(SSE42_FOUND true)
  else()
    set(SSE2_FOUND false)
    set(SSE42_FOUND false)
  endif()
  set(AES_FOUND false)
  set(SHA_FOUND false)
  set(CLMUL_FOUND false)
  set(CRC_PCLMUL_SRC "")
endif(CMAKE_CROSSCOMPILING)

if(SSE2_FOUND)
  add_definitions(-DHAVE_SSE2)
endif()
if(SSE42_FOUND)
  add_definitions(-DHAVE_SSE42)
endif()
if(AES_FOUND)
  add_definitions(-DHAVE_AESNI)
endif()
if(SHA_FOUND)
  add_definitions(-DHAVE_SHANI)
endif()
if(CLMUL_FOUND)
  add_definitions(-DHAVE_CLMUL)
endif(CLMUL_FOUND)
if(HAVE___UINT128_T)
  add_definitions(-DHAVE_INT128)
endif()
if(HAVE_INT64_T)
  add_definitions(-DHAVE_INT64)
elseif(HAVE__INT64)
  add_definitions(-DHAVE_INT64)
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  add_definitions(-DHAVE_BIT32)
endif()

# probe for HAVE_ALIGNED_ACCESS_REQUIRED
if(NOT CMAKE_CROSSCOMPILING)
  include(CMakePushCheckState)
  include(CheckCXXSourceRuns)
  set(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  cmake_push_check_state()
  if(CMAKE_COMPILER_IS_GNUCC
      OR (CMAKE_C_COMPILER_ID STREQUAL AppleClang)
      OR (CMAKE_C_COMPILER_ID STREQUAL Clang))
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -fno-sanitize-recover=undefined")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-recover=undefined")
    endif()
  endif()
  check_cxx_source_runs(
    "
#include <cstdint>
#include <arpa/inet.h>
uint32_t load(char* p, size_t offset)
{
  return *reinterpret_cast<uint32_t*>(p + offset);
}
bool good(uint32_t lhs, uint32_t small_endian)
{
  uint32_t rhs;
  if (ntohl(0xdeadbeaf) == 0xdeadbeaf) {
    return lhs == ntohl(small_endian);
  } else {
    return lhs == small_endian;
  }
}
int main(int argc, char **argv)
{
  char a1[] = \"ABCDEFG\";
  uint32_t a2[] = {0x44434241,
                   0x45444342,
                   0x46454443,
                   0x47464544};
  for (size_t i = 0; i < 4; i++) {
    if (!good(load(a1, i), a2[i])) {
      return 1;
    }
  }
  return 0;
}"
    ALLOW_ALIGNED_ACCESS)
  cmake_pop_check_state()
  set(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}")
  if(NOT ALLOW_ALIGNED_ACCESS)
    add_definitions(-DHAVE_ALIGNED_ACCESS_REQUIRED)
    message(STATUS "Set HAVE_ALIGNED_ACCESS_REQUIRED (ubsan or strict hardware, like arm, sparc)")
  else()
    message(STATUS "HAVE_ALIGNED_ACCESS_REQUIRED not required")
  endif()
else()
  add_definitions(-DHAVE_ALIGNED_ACCESS_REQUIRED)
  message(STATUS "Assuming aligned access is required, as on arm with -maligned-access or sparc")
endif(NOT CMAKE_CROSSCOMPILING)

if(AES_FOUND AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  set(SSE4_OBJ ${FALKHASH_OBJ})
endif()

find_package( Threads )
#set(CMAKE_THREAD_LIBS_INIT "-lpthread")

if(SSE2_FOUND)
  set(SSE2_SRC hasshe2.c)
endif()
if(SSE42_FOUND)
  # MSVC CL 14.16.27023 32bit crashes, 14.28.29910 works fine.
  if (MSVC AND (CMAKE_SIZEOF_VOID_P EQUAL 4) AND (MSVC_VERSION LESS 1928))
    add_definitions(-DHAVE_BROKEN_MSVC_CRC32C_HW)
  else()
    set(SSE4_SRC "crc32_hw.c")
  endif()
  # 64bit only:
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # enable_language(ASM-NASM) set(CMAKE_ASM_FLAGS "-f elf64") # or
    # macho64/win64
    set(X86_64ONLY_SRC farmhash-c.c farmhash-c-test.cc clhash.c)
    if(NOT MSVC)
      list(APPEND X86_64ONLY_SRC metrohash/metrohash64crc.cpp
           metrohash/metrohash128crc.cpp)
    endif()
    if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64") AND NOT MSVC)
      list(APPEND X86_64ONLY_SRC crc32_hw1.c)
    endif()
  else()
    message(STATUS "32bit only: CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
  endif()
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(SIPHASH_SRC siphash.c)
  else()
    set(SIPHASH_SRC siphash_ssse3.c)
  endif()
elseif(SSE2_FOUND)
  set(SIPHASH_SRC siphash_sse2.c)
else()
  set(SIPHASH_SRC siphash.c)
endif(SSE42_FOUND)

if(AES_FOUND AND (PROCESSOR_FAMILY STREQUAL "Intel"))
  set(T1HA_SRC
      t1ha/t1ha0.c t1ha/t1ha1.c t1ha/t1ha2.c t1ha/t1ha0_ia32aes_noavx.c
      t1ha/t1ha0_ia32aes_avx.c t1ha/t1ha0_ia32aes_avx2.c)
  if(MSVC)
    # ignoring unknown option '/arch:ia32'
    # set_source_files_properties(t1ha/t1ha0_ia32aes_noavx.c PROPERTIES
    # COMPILE_FLAGS "/arch:ia32")
    set_source_files_properties(t1ha/t1ha0_ia32aes_avx.c
                                PROPERTIES COMPILE_FLAGS "/arch:avx")
    set_source_files_properties(t1ha/t1ha0_ia32aes_avx2.c
      PROPERTIES COMPILE_FLAGS "/arch:avx2")
  else()
    set_source_files_properties(
      t1ha/t1ha0_ia32aes_noavx.c PROPERTIES COMPILE_FLAGS
                                            "-mno-avx2 -mno-avx -maes")
    set_source_files_properties(
      t1ha/t1ha0_ia32aes_avx.c PROPERTIES COMPILE_FLAGS "-mno-avx2 -mavx -maes")
    set_source_files_properties(t1ha/t1ha0_ia32aes_avx2.c
                                PROPERTIES COMPILE_FLAGS "-mavx -mavx2 -maes")
  endif()
else()
  set(T1HA_SRC t1ha/t1ha0.c t1ha/t1ha1.c t1ha/t1ha2.c)
endif(AES_FOUND AND (PROCESSOR_FAMILY STREQUAL "Intel"))

if(SHA_FOUND)
  # todo: arm, power
  set(SHA_SRC SHA-Intrinsics/sha1-x86.c SHA-Intrinsics/sha256-x86.c)
endif(SHA_FOUND)

if(SSE42_FOUND AND (PROCESSOR_FAMILY STREQUAL "Intel") AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  set(SSE4_OBJ ${SSE4_OBJ} pearson_hash/pearson.c)
endif()

if(NOT MSVC)
  set(NOTMSVC_SRC seahash.c)
  set_source_files_properties(HashMapTest.cpp PROPERTIES COMPILE_FLAGS "-std=c++11")
  set_source_files_properties(SpeedTest.cpp PROPERTIES COMPILE_FLAGS "-std=c++0x -I${CMAKE_SOURCE_DIR}")
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-std=c++0x")
  set_source_files_properties(Hashes.cpp PROPERTIES COMPILE_FLAGS "-std=c++1y -I${CMAKE_SOURCE_DIR} -flax-vector-conversions")
  set_source_files_properties(bittest.cpp PROPERTIES COMPILE_FLAGS "-std=c++0x")
  set_source_files_properties(Hashes.cpp PROPERTIES COMPILE_FLAGS "-std=c++1y -I${CMAKE_SOURCE_DIR} -flax-vector-conversions")
  # https://github.com/dgryski/trifles/tree/master/tsip
  set(TSIP_SRC tsip.c)
else(NOT MSVC)
  set_source_files_properties(SpeedTest.cpp PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}")
  # nmhash32 wants "/std:c++latest"
  set_source_files_properties(Hashes.cpp PROPERTIES COMPILE_FLAGS "/std:c++latest -I${CMAKE_SOURCE_DIR}")
endif(NOT MSVC)

find_library(HIGHWAY highwayhash)
if(HIGHWAY
   AND (NOT(CMAKE_CROSSCOMPILING)))
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(HIGHWAY_SRC HighwayHash.cpp)
    set(HIGHWAY_LIB highwayhash)
    add_definitions(-DHAVE_HIGHWAYHASH)
  endif()
endif()

# rust staticlib: https://github.com/tkaitchuck/aHash/ smhasher/ahash-cbindings/install.sh
find_library(AHASH_C ahash_c
             HINTS /usr/local/lib)
if(AHASH_C
   AND (NOT(CMAKE_CROSSCOMPILING)))
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(AHASH_C_LIB ahash_c)
    add_definitions(-DHAVE_AHASH_C)
  endif()
endif()

if(SSE42_FOUND AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  find_library(BLAKE3 blake3)
  # if (BLAKE3)
  #   set(BLAKE3_LIB ${BLAKE3})
  #   add_definitions(-DHAVE_BLAKE3)
  # endif()
endif()

set(BLAKE3_SRC blake3/blake3.c blake3/blake3_dispatch.c
               blake3/blake3_portable.c)
if(SSE42_FOUND)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(BLAKE3_SRC ${BLAKE3_SRC} blake3/blake3_neon.c)
  else()
    set(BLAKE3_SRC ${BLAKE3_SRC} blake3/blake3_sse41.c)
    set_source_files_properties(blake3/blake3_sse41.c
      PROPERTIES COMPILE_FLAGS "-DBLAKE3_USE_SSE41")
  endif()
endif()
if(AVX2_FOUND)
  set(BLAKE3_SRC ${BLAKE3_SRC} blake3/blake3_avx2.c)
  set_source_files_properties(
    blake3/blake3_avx2.c PROPERTIES COMPILE_FLAGS
                                    "-DBLAKE3_USE_SSE41 -DBLAKE3_USE_AVX2 -mavx2")
endif()
if(AVX512F_FOUND AND AVX512VL_FOUND)
  set(BLAKE3_SRC ${BLAKE3_SRC} blake3/blake3_avx512.c)
  set_source_files_properties(
    blake3/blake3_avx512.c
    PROPERTIES COMPILE_FLAGS
               "-DBLAKE3_USE_SSE41 -DBLAKE3_USE_AVX2 -DBLAKE3_USE_AVX512 -mavx512f -mavx512vl")
endif()
if(SSE42_FOUND
   AND (CMAKE_SIZEOF_VOID_P EQUAL 8)
   AND (NOT MSVC))
  set(UMASH_SRC umash.c umash.hpp)
endif()

set(ASCON_SRC asconhashv12/hash.c)
set_source_files_properties(
    ${ASCON_SRC}
    PROPERTIES COMPILE_FLAGS "-Iasconhashv12 -DASCON_INLINE_MODE")

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  # PMP breaks Debug build
  if(NOT (CMAKE_BUILD_TYPE STREQUAL "asan"))
    set(PMPML_SRC PMP_Multilinear.cpp PMP_Multilinear_64.cpp
                  PMP_Multilinear_test.cpp)
  endif()
endif()
if(CMAKE_CROSSCOMPILING)
  unset(PMPML_SRC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCROSSCOMPILING")
endif()
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  #HAVE_SSE42
  unset(PMPML_SRC)
endif()


# IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/BeagleHashes_test.c") set(BEAGLE_SRC
# BeagleHashes_test.c) set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
# -DHAVE_BEAGLE_HASHES") ENDIF() IF(EXISTS
# "${CMAKE_CURRENT_SOURCE_DIR}/funny_hash.h") set(CMAKE_CXX_FLAGS
# "${CMAKE_CXX_FLAGS} -DHAVE_FUNNY_HASH") ENDIF() IF(EXISTS
# "${CMAKE_CURRENT_SOURCE_DIR}/fanom_hash.h") set(CMAKE_CXX_FLAGS
# "${CMAKE_CXX_FLAGS} -DHAVE_FANOM_HASH") ENDIF()

if((CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION GREATER_EQUAL 9)
   AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
  cmake_policy(SET CMP0069 NEW)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT error)
endif()

add_library(
  SMHasherSupport STATIC
  AvalancheTest.cpp
  Bitslice.cpp
  Bitvec.cpp
  chaskey.c
  CityTest.cpp
  City.cpp
  crc.cpp
  DifferentialTest.cpp
  HashMapTest.cpp
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
  tifuhash.cpp
  floppsyhash.cpp
  fasthash.cpp
  beamsplitter.cpp
  discohash.cpp
  xxhash.c
  metrohash/metrohash64.cpp
  metrohash/metrohash128.cpp
  cmetrohash64.c
  opt_cmetrohash64_1.c
  farmhash.cc
  FarmTest.cc
  ${SSE4_OBJ}
  # ${FHTW_OBJ}
  ${T1HA_SRC}
  ${SHA_SRC}
  mum.cc
  jody_hash32.c
  jody_hash64.c
  ${TSIP_SRC}
  # ${BEAGLE_SRC}
  ${NOTMSVC_SRC}
  ${X86_64ONLY_SRC}
  ${CRC_PCLMUL_SRC}
  blake2b.c
  blake2s.c
  rmd128.c
  rmd160.c
  rmd256.c
  sha2/sha224.c
  sha2/sha256.c
  sha2/sha512_224.c
  sha2/sha512_256.c
  sha3.c
  ${PMPML_SRC}
  vmac.cpp
  rijndael-alg-fst.c
  ${BLAKE3_SRC}
  pengyhash.c
  pearson_hash/pearsonb.c
  # wyhash_condom.c
  ${ASCON_SRC}
  ${UMASH_SRC})

add_executable(SMHasher main.cpp)

if(ipo_supported)
  message(STATUS "IPO / LTO enabled")
  set_property(TARGET SMHasherSupport PROPERTY INTERPROCEDURAL_OPTIMIZATION
                                               True)
  set_property(TARGET SMHasher PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLTO")
  # set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-DLTO")
else()
  message(STATUS "IPO / LTO not supported: <${error}>")
endif()

target_link_libraries(SMHasher SMHasherSupport
                      ${HIGHWAY_LIB} ${BLAKE3_LIB} ${AHASH_C_LIB}
                      ${CMAKE_THREAD_LIBS_INIT})

# add_executable( bittest bittest.cpp )
#
# target_link_libraries( bittest SMHasherSupport ${CMAKE_THREAD_LIBS_INIT} )

if(NOT (CMAKE_CROSSCOMPILING))
  enable_testing()
  add_test(VerifyAll SMHasher --test=VerifyAll)
  add_test(Sanity SMHasher --test=Sanity)
  add_test(Speed SMHasher --test=Speed)
  add_test(Cyclic SMHasher --test=Cyclic)
  add_test(Zeroes SMHasher --test=Zeroes)
  add_test(Seed SMHasher --test=Seed)
endif()

add_custom_target(
  TAGS
  COMMAND etags --language=c++ *.cpp *.cc *.h
  DEPENDS ${SRCS}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
