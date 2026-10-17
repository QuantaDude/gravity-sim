cmake_minimum_required(VERSION 3.21)

# MSVC: /MT on non-debug configs, /MTd on Debug (was b_vscrt=static_from_buildtype).
# Must be set before any target is created so raylib picks it up too.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(gravity-sim VERSION 0.1.0 LANGUAGES C)
# ---------------------------------------------------------------------------
# Debug/Release
#
# Single-config generators (Make/Ninja) default to Debug
#   cmake -B build                           (Debug)
#   cmake -B build-release -DCMAKE_BUILD_TYPE=Release
# Multi-config generators (VS, Xcode): pick with --config.
# ---------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Generate compile_commands.json for clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where binaries land on install: <source dir>/<os>_<cpu>
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

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
set(RAYLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/raylib" CACHE PATH
    "raylib source tree")
set(RAYLIB_NUKLEAR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/raylib-nuklear" CACHE PATH
    "raylib-nuklear source tree")

# raylib, static
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("${RAYLIB_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/raylib")

# nuklear + raylib-nuklear
# Both are single-header libraries, so nothing gets built — the `raylib_nuklear`
# target just puts raylib-nuklear.h and its bundled nuklear.h on the include path.
add_subdirectory("${RAYLIB_NUKLEAR_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/raylib-nuklear")

# OR, without the subproject — thirdparty/raylib-nuklear/include holds
# raylib-nuklear.h and the bundled nuklear.h that its own `#include "nuklear.h"`
# target_include_directories(gravity-sim PRIVATE thirdparty/raylib-nuklear/include)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(gravity-sim)

target_sources(gravity-sim PRIVATE ${MY_SOURCES})

set_target_properties(gravity-sim PROPERTIES
  C_STANDARD          17
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS        OFF      # -std=c17, like Meson's c_std=c17
)

target_link_libraries(gravity-sim PRIVATE raylib raylib_nuklear)
if(NOT WIN32)
  target_link_libraries(gravity-sim PRIVATE m)
endif()

# warning_level=2
if(MSVC)
  target_compile_options(gravity-sim PRIVATE /W3)
else()
  target_compile_options(gravity-sim PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Compiler-specific SIMD flags
# ---------------------------------------------------------------------------
include(CheckCCompilerFlag)
if(MSVC)
  check_c_compiler_flag("/arch:AVX2" GRAVITY_HAS_ARCH_AVX2)
  if(GRAVITY_HAS_ARCH_AVX2)
    target_compile_options(gravity-sim PRIVATE /arch:AVX2)
  endif()
else()
  check_c_compiler_flag("-mavx2" GRAVITY_HAS_MAVX2)
  if(GRAVITY_HAS_MAVX2)
    target_compile_options(gravity-sim PRIVATE -mavx2)
  endif()
  check_c_compiler_flag("-mfma" GRAVITY_HAS_MFMA)
  if(GRAVITY_HAS_MFMA)
    target_compile_options(gravity-sim PRIVATE -mfma)
  endif()
endif()

# ---------------------------------------------------------------------------
# Dev conveniences
# ---------------------------------------------------------------------------
# <project root>/compile_commands.json -> <build dir>/compile_commands.json
if(PROJECT_IS_TOP_LEVEL AND CMAKE_GENERATOR MATCHES "Makefiles|Ninja")
  set(_cc_link "${CMAKE_SOURCE_DIR}/compile_commands.json")
  if(EXISTS "${_cc_link}" AND NOT IS_SYMLINK "${_cc_link}")
    message(WARNING "${_cc_link} is a regular file — not replacing it with a symlink")
  else()
    file(REMOVE "${_cc_link}")
    file(CREATE_LINK "${CMAKE_BINARY_DIR}/compile_commands.json" "${_cc_link}"
         RESULT _cc_res SYMBOLIC)
    if(NOT _cc_res EQUAL 0)
      message(WARNING "Could not symlink compile_commands.json: ${_cc_res}")
    endif()
  endif()
endif()

# <build_dir>/shaders -> <source dir>/src/shaders
add_custom_target(link-shaders ALL
  COMMAND ${CMAKE_COMMAND} -E create_symlink
          "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
          "$<TARGET_FILE_DIR:gravity-sim>/shaders"
  COMMENT "Symlinking src/shaders next to the executable"
  VERBATIM
)
add_dependencies(gravity-sim link-shaders)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(TARGETS gravity-sim RUNTIME DESTINATION "${PLATFORM_INSTALL_DIR}")

# <install dir>/shaders -> <source dir>/src/shaders
install(CODE "
  set(_link \"\$ENV{DESTDIR}${PLATFORM_INSTALL_DIR}/shaders\")
  file(MAKE_DIRECTORY \"\$ENV{DESTDIR}${PLATFORM_INSTALL_DIR}\")
  file(REMOVE \"\${_link}\")
  file(CREATE_LINK \"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders\" \"\${_link}\" SYMBOLIC)
")
