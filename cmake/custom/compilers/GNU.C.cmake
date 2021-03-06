# Overrides contents of all variables previously set by CMake
if(NOT DEFINED ENV{CFLAGS})
    if(CMAKE_C_COMPILER_ID MATCHES GNU)
        set(CMAKE_C_FLAGS         "-std=c99 -DRESTRICT=restrict -DFUNDERSCORE=1 -fPIC")
        set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3 -DDEBUG -Wall -Wextra -Winit-self -Wuninitialized -Wmissing-declarations -Wwrite-strings ")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
    endif()
endif()
