cmake_minimum_required(VERSION 3.21)

project(raylib
  VERSION 6.0.0
  DESCRIPTION "Simple and easy-to-use library to enjoy videogames programming"
  HOMEPAGE_URL "https://www.raylib.com"
  LANGUAGES C
)
# License: zlib

# Meson's default buildtype is 'debug'; keep that for single-config generators
# when raylib is the top-level project.
if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeOptions.txt")

set(RAYLIB_API_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# ---------------------------------------------------------------------------
# Install location: <top-level source dir>/<os>_<cpu>   (e.g. linux_x86_64)
# Shared by raylib, raylib-nuklear and gravity-sim — whichever project is
# configured first defines it.
# ---------------------------------------------------------------------------
if(NOT DEFINED PLATFORM_INSTALL_DIR)
  string(TOLOWER "${CMAKE_SYSTEM_NAME}" _sys)
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _cpu)
  if(_cpu STREQUAL "amd64")
    set(_cpu "x86_64")
  elseif(_cpu STREQUAL "arm64")
    set(_cpu "aarch64")
  endif()
  set(PLATFORM_INSTALL_DIR "${CMAKE_SOURCE_DIR}/${_sys}_${_cpu}" CACHE PATH
      "Where libraries, headers and binaries get installed")
endif()

set(RAYLIB_SOURCES
  src/raudio.c
  src/rcore.c
  src/rmodels.c
  src/rshapes.c
  src/rtext.c
  src/rtextures.c
)

set(RAYLIB_PUBLIC_HEADERS
  src/raylib.h
  src/rcamera.h
  src/rlgl.h
  src/raymath.h
)

# ---------------------------------------------------------------------------
# Platform
# ---------------------------------------------------------------------------
string(TOUPPER "${PLATFORM}" RAYLIB_PLATFORM)

set(RAYLIB_DEFINES "")
set(RAYLIB_LIBS "")
set(RAYLIB_PC_LIBS_PRIVATE "")

# 'Desktop' (PLATFORM_DESKTOP -> rcore.c auto-forces PLATFORM_DESKTOP_GLFW)
# has been removed along with src/external/glfw. RGFW is the only desktop
# backend wired up below.
if(RAYLIB_PLATFORM STREQUAL "WEB")
  list(APPEND RAYLIB_DEFINES PLATFORM_WEB)
  if(OPENGL_VERSION STREQUAL "ES 3.0")
    list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_ES3)
  else()
    list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_ES2)
  endif()

elseif(RAYLIB_PLATFORM STREQUAL "ANDROID")
  list(APPEND RAYLIB_DEFINES PLATFORM_ANDROID GRAPHICS_API_OPENGL_ES2)
  foreach(_lib log android EGL GLESv2)
    find_library(RAYLIB_${_lib}_LIB ${_lib} REQUIRED)
    list(APPEND RAYLIB_LIBS "${RAYLIB_${_lib}_LIB}")
  endforeach()

elseif(RAYLIB_PLATFORM STREQUAL "DRM")
  list(APPEND RAYLIB_DEFINES PLATFORM_DRM _DEFAULT_SOURCE GRAPHICS_API_OPENGL_ES2 EGL_NO_X11)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(RAYLIB_DRM REQUIRED IMPORTED_TARGET libdrm)
  list(APPEND RAYLIB_LIBS PkgConfig::RAYLIB_DRM)
  foreach(_mod gbm egl glesv2)           # optional, like required : false
    pkg_check_modules(RAYLIB_${_mod} IMPORTED_TARGET ${_mod})
    if(RAYLIB_${_mod}_FOUND)
      list(APPEND RAYLIB_LIBS PkgConfig::RAYLIB_${_mod})
    endif()
  endforeach()

elseif(RAYLIB_PLATFORM STREQUAL "SDL")
  list(APPEND RAYLIB_DEFINES PLATFORM_DESKTOP_SDL)
  find_package(SDL3 CONFIG QUIET)
  if(TARGET SDL3::SDL3)
    list(APPEND RAYLIB_LIBS SDL3::SDL3)
    list(APPEND RAYLIB_PC_LIBS_PRIVATE "-lSDL3")
  else()
    find_package(SDL2 CONFIG REQUIRED)
    if(TARGET SDL2::SDL2)
      list(APPEND RAYLIB_LIBS SDL2::SDL2)
    else()
      list(APPEND RAYLIB_LIBS SDL2::SDL2-static)
    endif()
    list(APPEND RAYLIB_PC_LIBS_PRIVATE "-lSDL2")
  endif()
  find_package(OpenGL REQUIRED)
  list(APPEND RAYLIB_LIBS OpenGL::GL)

elseif(RAYLIB_PLATFORM STREQUAL "RGFW")
  list(APPEND RAYLIB_DEFINES PLATFORM_DESKTOP_RGFW)
  # Link requirements straight from src/external/RGFW/RGFW.h's own header
  # comments:
  #   linux : gcc main.c -lX11 -lXrandr -lGL
  #   windows : gcc main.c -lopengl32 -lgdi32
  #   macos : gcc main.c -framework Cocoa -framework CoreVideo -framework OpenGL -framework IOKit
  if(APPLE)
    foreach(_fw Cocoa CoreVideo OpenGL IOKit)
      find_library(RAYLIB_FW_${_fw} ${_fw} REQUIRED)
      list(APPEND RAYLIB_LIBS "${RAYLIB_FW_${_fw}}")
    endforeach()
    list(APPEND RAYLIB_PC_LIBS_PRIVATE
      "-framework Cocoa -framework CoreVideo -framework OpenGL -framework IOKit")
  elseif(WIN32)
    find_package(OpenGL REQUIRED)
    list(APPEND RAYLIB_LIBS OpenGL::GL gdi32)
    list(APPEND RAYLIB_PC_LIBS_PRIVATE "-lopengl32 -lgdi32")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    if(NOT TARGET X11::Xrandr)
      message(FATAL_ERROR "raylib (RGFW): Xrandr development files not found (need -lXrandr)")
    endif()
    find_package(OpenGL REQUIRED OPTIONAL_COMPONENTS GLX)
    list(APPEND RAYLIB_LIBS X11::X11 X11::Xrandr OpenGL::GL)
    if(TARGET OpenGL::GLX)               # optional, like required : false
      list(APPEND RAYLIB_LIBS OpenGL::GLX)
    endif()
    list(APPEND RAYLIB_PC_LIBS_PRIVATE "-lX11 -lXrandr -lGL")
  endif()
else()
  message(FATAL_ERROR "Unknown PLATFORM '${PLATFORM}' (expected RGFW, Web, Android, DRM or SDL)")
endif()

# threads is needed by raylib itself regardless of windowing backend.
find_package(Threads REQUIRED)
list(APPEND RAYLIB_LIBS Threads::Threads)

# ---------------------------------------------------------------------------
# Graphics API
# ---------------------------------------------------------------------------
if(OPENGL_VERSION STREQUAL "3.3")
  list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_33)
elseif(OPENGL_VERSION STREQUAL "4.3")
  list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_43)
elseif(OPENGL_VERSION STREQUAL "ES 2.0")
  list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_ES2)
elseif(OPENGL_VERSION STREQUAL "ES 3.0")
  list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_ES3)
elseif(RAYLIB_PLATFORM STREQUAL "RGFW" OR RAYLIB_PLATFORM STREQUAL "SDL")
  list(APPEND RAYLIB_DEFINES GRAPHICS_API_OPENGL_33)
endif()

# ---------------------------------------------------------------------------
# Modules / optional file formats
# ---------------------------------------------------------------------------
foreach(_flag RSHAPES RTEXTURES RTEXT RMODELS)
  if(SUPPORT_MODULE_${_flag})
    list(APPEND RAYLIB_DEFINES SUPPORT_MODULE_${_flag}=1)
  else()
    list(APPEND RAYLIB_DEFINES SUPPORT_MODULE_${_flag}=0)
  endif()
endforeach()

if(SUPPORT_MODULE_RAUDIO AND USE_AUDIO)
  list(APPEND RAYLIB_DEFINES SUPPORT_MODULE_RAUDIO=1)
  find_package(OpenAL QUIET)             # optional, like required : false
  if(OPENAL_FOUND AND TARGET OpenAL::OpenAL)
    list(APPEND RAYLIB_LIBS OpenAL::OpenAL)
  endif()
else()
  list(APPEND RAYLIB_DEFINES SUPPORT_MODULE_RAUDIO=0)
endif()

foreach(_flag TRACELOG CAMERA_SYSTEM GESTURES_SYSTEM COMPRESSION_API AUTOMATION_EVENTS)
  if(SUPPORT_${_flag})
    list(APPEND RAYLIB_DEFINES SUPPORT_${_flag}=1)
  else()
    list(APPEND RAYLIB_DEFINES SUPPORT_${_flag}=0)
  endif()
endforeach()

# File formats are only defined when enabled (no =0 counterpart).
foreach(_fmt TGA JPG BMP HDR KTX ASTC PVR OBJ GLTF VOX FLAC)
  if(SUPPORT_FILEFORMAT_${_fmt} OR INCLUDE_EVERYTHING)
    list(APPEND RAYLIB_DEFINES SUPPORT_FILEFORMAT_${_fmt}=1)
  endif()
endforeach()

if(BUILD_SHARED_LIBS)
  list(APPEND RAYLIB_DEFINES BUILD_LIBTYPE_SHARED)
endif()

list(REMOVE_DUPLICATES RAYLIB_DEFINES)

# ---------------------------------------------------------------------------
# The library target
# ---------------------------------------------------------------------------
add_library(raylib ${RAYLIB_SOURCES} ${RAYLIB_PUBLIC_HEADERS})
add_library(raylib::raylib ALIAS raylib)

target_include_directories(raylib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(raylib PRIVATE ${RAYLIB_DEFINES})
target_link_libraries(raylib PUBLIC ${RAYLIB_LIBS})
if(NOT WIN32)
  target_link_libraries(raylib PUBLIC m)
  list(APPEND RAYLIB_PC_LIBS_PRIVATE "-lm -lpthread")
endif()

# meson warning_level=1
if(MSVC)
  target_compile_options(raylib PRIVATE /W2)
else()
  target_compile_options(raylib PRIVATE -Wall)
endif()

set_target_properties(raylib PROPERTIES
  VERSION       ${PROJECT_VERSION}
  SOVERSION     ${RAYLIB_API_VERSION}
  PUBLIC_HEADER "${RAYLIB_PUBLIC_HEADERS}"
)

if(BUILD_SHARED_LIBS)
  # was -fvisibility=hidden
  set_target_properties(raylib PROPERTIES C_VISIBILITY_PRESET hidden)
  # lets Windows consumers see dllimport instead of dllexport
  target_compile_definitions(raylib INTERFACE USE_LIBTYPE_SHARED)
endif()

if(WITH_PIC)
  set_target_properties(raylib PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Sanitizers (PUBLIC so executables linking raylib get the runtime too)
set(_san "")
if(ENABLE_ASAN)
  list(APPEND _san address)
endif()
if(ENABLE_UBSAN)
  list(APPEND _san undefined)
endif()
if(ENABLE_MSAN)
  if(ENABLE_ASAN)
    message(FATAL_ERROR "ENABLE_MSAN can't be combined with ENABLE_ASAN")
  endif()
  list(APPEND _san memory)
endif()
if(_san)
  if(MSVC)
    message(WARNING "Sanitizer options are only wired up for GCC/Clang")
  else()
    list(JOIN _san "," _san_arg)
    target_compile_options(raylib PUBLIC -fsanitize=${_san_arg} -fno-omit-frame-pointer)
    target_link_options(raylib PUBLIC -fsanitize=${_san_arg})
  endif()
endif()

# ---------------------------------------------------------------------------
# Install  (everything goes to PLATFORM_INSTALL_DIR, not CMAKE_INSTALL_PREFIX)
# ---------------------------------------------------------------------------
install(TARGETS raylib
  ARCHIVE       DESTINATION "${PLATFORM_INSTALL_DIR}"
  LIBRARY       DESTINATION "${PLATFORM_INSTALL_DIR}"
  RUNTIME       DESTINATION "${PLATFORM_INSTALL_DIR}"
  PUBLIC_HEADER DESTINATION "${PLATFORM_INSTALL_DIR}/include"
)

# raylib.pc — points at PLATFORM_INSTALL_DIR so it matches where the files
# actually land, and is installed next to them.
set(RAYLIB_PC_PREFIX "${PLATFORM_INSTALL_DIR}")
string(JOIN " " RAYLIB_PC_LIBS_PRIVATE ${RAYLIB_PC_LIBS_PRIVATE})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/raylib.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/raylib.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/raylib.pc"
        DESTINATION "${PLATFORM_INSTALL_DIR}/pkgconfig")

# BUILD_EXAMPLES: only meaningful when raylib is the top-level project.
if(BUILD_EXAMPLES AND PROJECT_IS_TOP_LEVEL)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
  else()
    message(WARNING "BUILD_EXAMPLES is enabled but examples/CMakeLists.txt does not exist — skipping")
  endif()
endif()
