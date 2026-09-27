# raylib_nuklear — the INTERFACE-library equivalent: no sources, just the two
# headers on the include path.
#   add_library(raylib_nuklear INTERFACE)
#   target_include_directories(raylib_nuklear INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
raylib_nuklear_dep = declare_dependency(
  include_directories : include_directories('.'),
)

# install(FILES raylib-nuklear.h nuklear.h DESTINATION include)
# install_headers with no `subdir:` installs straight to {prefix}/include,
# matching DESTINATION include exactly (no raylib_nuklear/ subfolder).
install_headers(
  'raylib-nuklear.h',
  'nuklear.h',
)
