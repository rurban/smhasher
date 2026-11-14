# Reconfigure the same native x86 build with different compiler targets.
set(_base "-march=x86-64")
set(_aes "-march=x86-64 -msse4.2 -maes -mpclmul -mno-avx")
foreach(_case base aes base_again)
  if(_case STREQUAL "aes")
    set(_flags "${_aes}")
    set(_expect true)
  else()
    set(_flags "${_base}")
    set(_expect false)
  endif()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-H${SOURCE_DIR}" "-B${TEST_BINARY_DIR}"
            "-DCMAKE_C_COMPILER=${TEST_C_COMPILER}"
            "-DCMAKE_CXX_COMPILER=${TEST_CXX_COMPILER}"
            "-DCMAKE_C_FLAGS=${_flags}" "-DCMAKE_CXX_FLAGS=${_flags}"
    RESULT_VARIABLE _result OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Feature test configure failed: ${_out}\n${_err}")
  endif()
  file(READ "${TEST_BINARY_DIR}/CMakeCache.txt" _cache)
  foreach(_feature AES CLMUL SSE42)
    if(NOT _cache MATCHES "${_feature}_FOUND:BOOL=${_expect}([\r\n]|$)")
      message(FATAL_ERROR "${_feature} was not ${_expect} for ${_flags}")
    endif()
  endforeach()
  if(NOT _cache MATCHES "AVX2_FOUND:BOOL=false([\r\n]|$)")
    message(FATAL_ERROR "AVX2 was enabled for ${_flags}")
  endif()
endforeach()
