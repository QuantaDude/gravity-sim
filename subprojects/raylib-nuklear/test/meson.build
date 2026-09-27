# raylib-nuklear-test
# raylib_dep, raylib_nuklear_dep and cc were already set up in examples/meson.build,
# which subdir('examples') runs before this file — Meson subdirs share the
# parent script's variables, same as the original CMakeLists.txt relying on
# add_subdirectory(examples) having already defined the raylib/raylib_nuklear
# targets before add_subdirectory(test) runs.
test_exe = executable('raylib-nuklear-test', 'raylib-nuklear-test.c',
  c_args : cc.get_supported_arguments('-Wall', '-Wextra', '-Wconversion', '-Wsign-conversion'),
  dependencies : [raylib_dep, raylib_nuklear_dep],
)

# Copy the resources
# file(GLOB resources resources/*) + file(COPY ... DESTINATION "resources/")
# Meson intentionally has no glob primitive (explicit file lists are the
# idiom), so this shells out to copy the whole directory the way the CMake
# GLOB did. If you know the exact file list, prefer configure_file(copy:
# true) per file instead, as examples/meson.build does.
resources_copy = custom_target('copy-test-resources',
  output : 'resources-stamp',
  command : [
    find_program('python3'),
    '-c',
    'import shutil, sys; shutil.copytree(sys.argv[1], sys.argv[2], dirs_exist_ok=True); open(sys.argv[3], "w").close()',
    meson.current_source_dir() / 'resources',
    meson.current_build_dir() / 'resources',
    '@OUTPUT@',
  ],
)

# add_test(NAME raylib-nuklear-test COMMAND raylib-nuklear-test)
# `list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")` has no config-file
# equivalent — just run `meson test --print-errorlogs` to get the same thing.
test('raylib-nuklear-test', test_exe,
  depends : resources_copy,
  workdir : meson.current_build_dir(),
)
