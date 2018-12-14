set(SDL2_ROOT "${PROJECT_SOURCE_DIR}/vendor/SDL2-2.0.8")
set(SDL2_INCLUDE_DIRS "${SDL2_ROOT}/include")

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(SDL2_LIB_DIR "${SDL2_ROOT}/lib/x64")
else ()
  set(SDL2_LIB_DIR "${SDL2_ROOT}/lib/x86")
endif ()

set(SDL2_LIBRARIES "${SDL2_LIB_DIR}/SDL2.lib;${SDL2_LIB_DIR}/SDL2main.lib")
set(SDL2_DLL "${SDL2_LIB_DIR}/SDL2.dll")

string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)

