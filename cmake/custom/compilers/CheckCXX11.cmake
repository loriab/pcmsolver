# - Check which parts of the C++11 standard the compiler supports
#
# When found it will set the following variables
#
#  CXX_STANDARD_FLAG         - the compiler flags needed to get C++11 features
#
#  HAS_CXX11_AUTO               - auto keyword
#  HAS_CXX11_AUTO_RET_TYPE      - function declaration with deduced return types
#  HAS_CXX11_CLASS_OVERRIDE     - override and final keywords for classes and methods
#  HAS_CXX11_CONSTEXPR          - constexpr keyword
#  HAS_CXX11_CSTDINT_H          - cstdint header
#  HAS_CXX11_DECLTYPE           - decltype keyword
#  HAS_CXX11_FUNC               - __func__ preprocessor constant
#  HAS_CXX11_INITIALIZER_LIST   - initializer list
#  HAS_CXX11_LAMBDA             - lambdas
#  HAS_CXX11_LIB_REGEX          - regex library
#  HAS_CXX11_LONG_LONG          - long long signed & unsigned types
#  HAS_CXX11_NULLPTR            - nullptr
#  HAS_CXX11_RVALUE_REFERENCES  - rvalue references
#  HAS_CXX11_SIZEOF_MEMBER      - sizeof() non-static members
#  HAS_CXX11_STATIC_ASSERT      - static_assert()
#  HAS_CXX11_VARIADIC_TEMPLATES - variadic templates
#  HAS_CXX11_NOEXCEPT           - noexcept keyword

#=============================================================================
# Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
# Copyright 2012 Andreas Weis
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Each feature may have up to 3 checks, every one of them in its own file
# FEATURE.cpp              - example that must build and return 0 when run
# FEATURE_fail.cpp         - example that must build, but may not return 0 when run
# FEATURE_fail_compile.cpp - example that must fail compilation
#
# The first one is mandatory, the latter 2 are optional and do not depend on
# each other (i.e. only one may be present).

macro(discover_cxx11_support _CXX_STANDARD_FLAG)
    # Check which compiler flag is valid for the C++11 standard
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" _HAS_CXX11_FLAG)
    set(_discovered_flag "-std=c++11")
    if(NOT _HAS_CXX11_FLAG)
        check_cxx_compiler_flag("-std=c++0x" _HAS_CXX0X_FLAG)
        set(_discovered_flag "-std=c++0x")
    endif()
    set(${_CXX_STANDARD_FLAG} ${_discovered_flag})

    set(HAS_CXX11_SUPPORT FALSE)
    if(_HAS_CXX11_FLAG OR _HAS_CXX0X_FLAG)
	   set(HAS_CXX11_SUPPORT TRUE)
	   add_definitions(-DHAS_CXX11)
    endif()

    cxx11_check_feature("${_discovered_flag}" "__func__"             HAS_CXX11_FUNC)
    cxx11_check_feature("${_discovered_flag}" "auto"                 HAS_CXX11_AUTO)
    cxx11_check_feature("${_discovered_flag}" "auto_ret_type"        HAS_CXX11_AUTO_RET_TYPE)
    cxx11_check_feature("${_discovered_flag}" "class_override_final" HAS_CXX11_CLASS_OVERRIDE)
    cxx11_check_feature("${_discovered_flag}" "constexpr"            HAS_CXX11_CONSTEXPR)
    cxx11_check_feature("${_discovered_flag}" "cstdint"              HAS_CXX11_CSTDINT_H)
    cxx11_check_feature("${_discovered_flag}" "decltype"             HAS_CXX11_DECLTYPE)
    cxx11_check_feature("${_discovered_flag}" "initializer_list"     HAS_CXX11_INITIALIZER_LIST)
    cxx11_check_feature("${_discovered_flag}" "lambda"               HAS_CXX11_LAMBDA)
    cxx11_check_feature("${_discovered_flag}" "long_long"            HAS_CXX11_LONG_LONG)
    cxx11_check_feature("${_discovered_flag}" "nullptr"              HAS_CXX11_NULLPTR)
    cxx11_check_feature("${_discovered_flag}" "regex"                HAS_CXX11_LIB_REGEX)
    cxx11_check_feature("${_discovered_flag}" "rvalue-references"    HAS_CXX11_RVALUE_REFERENCES)
    cxx11_check_feature("${_discovered_flag}" "sizeof_member"        HAS_CXX11_SIZEOF_MEMBER)
    cxx11_check_feature("${_discovered_flag}" "static_assert"        HAS_CXX11_STATIC_ASSERT)
    cxx11_check_feature("${_discovered_flag}" "variadic_templates"   HAS_CXX11_VARIADIC_TEMPLATES)
    cxx11_check_feature("${_discovered_flag}" "noexcept"             HAS_CXX11_NOEXCEPT)

    # Add feature definitions
    foreach(_feature_def ${CXX11_DEFINITIONS})
        add_definitions(-D${_feature_def})
    endforeach()
endmacro(discover_cxx11_support)

function(cxx11_check_feature CXX_STANDARD_FLAG FEATURE_NAME RESULT_VAR)
    if(NOT DEFINED ${RESULT_VAR})
        set(_bindir "${CMAKE_CURRENT_BINARY_DIR}/CheckCXX11/cxx11_${FEATURE_NAME}")

        set(_SRCFILE_BASE ${CMAKE_CURRENT_LIST_DIR}/CheckCXX11Features/cxx11-test-${FEATURE_NAME})
        set(_LOG_NAME "\"${FEATURE_NAME}\"")

        set(_SRCFILE "${_SRCFILE_BASE}.cpp")
        set(_SRCFILE_FAIL "${_SRCFILE_BASE}_fail.cpp")
        set(_SRCFILE_FAIL_COMPILE "${_SRCFILE_BASE}_fail_compile.cpp")

        if(CROSS_COMPILING)
            try_compile(${RESULT_VAR} "${_bindir}" "${_SRCFILE}"
                        COMPILE_DEFINITIONS "${CXX_STANDARD_FLAG}")
            if (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
                try_compile(${RESULT_VAR} "${_bindir}_fail" "${_SRCFILE_FAIL}"
                            COMPILE_DEFINITIONS "${CXX_STANDARD_FLAG}")
            endif (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
        else(CROSS_COMPILING)
            try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
                    "${_bindir}" "${_SRCFILE}"
                    COMPILE_DEFINITIONS "${CXX_STANDARD_FLAG}")
            if(_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
                set(${RESULT_VAR} TRUE)
            else(_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
                set(${RESULT_VAR} FALSE)
            endif(_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
            if(${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
                try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
                        "${_bindir}_fail" "${_SRCFILE_FAIL}"
                         COMPILE_DEFINITIONS "${CXX_STANDARD_FLAG}")
                if(_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
                    set(${RESULT_VAR} TRUE)
                else(_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
                    set(${RESULT_VAR} FALSE)
                endif(_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
            endif(${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
        endif(CROSS_COMPILING)
        if(${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})
            try_compile(_TMP_RESULT "${_bindir}_fail_compile" "${_SRCFILE_FAIL_COMPILE}"
                        COMPILE_DEFINITIONS "${CXX_STANDARD_FLAG}")
            if(_TMP_RESULT)
                set(${RESULT_VAR} FALSE)
            else(_TMP_RESULT)
                set(${RESULT_VAR} TRUE)
            endif(_TMP_RESULT)
        endif(${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})

        if(${RESULT_VAR})
            message(STATUS "C++11 ${_LOG_NAME} feature works")
            list(APPEND CXX11_DEFINITIONS ${RESULT_VAR})
        else(${RESULT_VAR})
            message(STATUS "C++11 ${_LOG_NAME} feature not supported")
        endif(${RESULT_VAR})
        set(${RESULT_VAR} ${${RESULT_VAR}} CACHE INTERNAL "C++11 support for ${_LOG_NAME}")
        set(CXX11_DEFINITIONS "${CXX11_DEFINITIONS}" CACHE INTERNAL "")
    endif(NOT DEFINED ${RESULT_VAR})
endfunction(cxx11_check_feature)
