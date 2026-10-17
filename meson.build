project(
  'gravity-sim',
  'c',
  version: '0.1.0',
  license: 'MIT',
  default_options: [
    'c_std=c17',
    'b_vscrt=static_from_buildtype', # MSVC: /MT on release-ish buildtypes, /MTd on debug
    'warning_level=2',
  ],
  subproject_dir: 'thirdparty',
)
cc = meson.get_compiler('c')

# ---------------------------------------------------------------------------
# Compiler-specific SIMD flags
# cc.get_supported_arguments() silently drops any flag the active
# compiler doesn't understand instead of erroring out.
# ---------------------------------------------------------------------------
simd_flags = []
if cc.get_id() == 'msvc'
  simd_flags += cc.get_supported_arguments('/arch:AVX2')
else
  simd_flags += cc.get_supported_arguments('-mavx2', '-mfma')
endif
add_project_arguments(simd_flags, language: 'c')

# ---------------------------------------------------------------------------
# Debug/Release optimization flags
#
# Meson's buildtype already drives per-compiler optimization + debug-info
# flags automatically (-O0/-g for debug, -O3 for release, etc. — mapped
# correctly whether the compiler is MSVC, GCC or Clang), so there's no
# manual `if(CMAKE_BUILD_TYPE STREQUAL ...)`-style branching to write.
# Select it at configure time instead:
#   meson setup builddir --buildtype=debug          (default)
#   meson setup builddir-release --buildtype=release
#
# b_vscrt=static_from_buildtype above is the one piece MSVC needs spelled
# out explicitly — it reproduces your CMAKE_MSVC_RUNTIME_LIBRARY block
# (MultiThreaded / MultiThreadedDebug) with a single option.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# raylib
#
raylib_proj = subproject('raylib', default_options: ['default_library=static'])
raylib_dep = raylib_proj.get_variable('raylib_dep')
# ---------------------------------------------------------------------------
# nuklear + raylib-nuklear
#
raylib_nuklear_proj = subproject('raylib-nuklear')
raylib_nuklear_dep = raylib_nuklear_proj.get_variable('raylib_nuklear_dep')
# OR:
# Both are single-header libraries, so they don't need building — just to be
# on the include path.
# In exactly one .c file, before including raylib-nuklear.h:
#   #define RAYLIB_NUKLEAR_IMPLEMENTATION
#   #include "raylib-nuklear.h"
# ---------------------------------------------------------------------------
# raylib-nuklear.h and its own bundled nuklear.h live in
# thirdparty/raylib-nuklear/include/, not the repo root — that bundled copy
# is what raylib-nuklear.h's own `#include "nuklear.h"` will actually find,
# so thirdparty/Nuklear (the standalone submodule) isn't even needed on this
# include path, but it's left here in case you want to build against your
# own Nuklear checkout instead of raylib-nuklear's bundled one.
# nuklear_inc = include_directories(
#   'thirdparty/raylib-nuklear/include',
# )

sources = files(
  'src/main.c',
  # add the rest of the .c files here
)

executable(
  'gravity-sim',
  sources,
  #include_directories : nuklear_inc,
  dependencies: [raylib_dep, raylib_nuklear_dep, cc.find_library('m', required: false)],
  install: true,
  install_dir: meson.current_source_dir()
  + '/'
  + host_machine.system()
  + '_'
  + host_machine.cpu(),
)

install_symlink(
  'shaders',
  install_dir: meson.current_source_dir()
  + '/'
  + host_machine.system()
  + '_'
  + host_machine.cpu()
  + '/',
  pointing_to: meson.current_source_dir() + '/src/shaders/',
)
