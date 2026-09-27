# raylib
# CMake: find_package(raylib) QUIET, else FetchContent from GitHub.
# Meson: try an installed raylib via pkg-config first, then fall back to the
# raylib subproject (put a raylib.wrap under subprojects/, or vendor it the
# way the earlier meson.build/meson_options.txt for raylib itself does).
raylib_dep = dependency('raylib', required : false)
if not raylib_dep.found()
  raylib_proj = subproject('raylib')
  raylib_dep  = raylib_proj.get_variable('raylib_dep')
endif

cc = meson.get_compiler('c')
if cc.get_id() == 'msvc'
  warning_flags = cc.get_supported_arguments('/W4', '/WX')
else
  warning_flags = cc.get_supported_arguments('-Wall', '-Wextra', '-Wpedantic', '-Werror')
endif

is_emscripten = host_machine.system() == 'emscripten'

# Web Configurations
# CMake's `if (EMSCRIPTEN)` block sets CMAKE_EXE_LINKER_FLAGS, which is a
# directory-wide variable that ends up applying to every executable target
# below (not just the demo) — reproduced here as link_args shared by all of
# them. `add_definitions(-DCMAKE_BUILD_TYPE=Release)` was CMake-only cruft
# with no meaningful Meson equivalent and is dropped.
web_link_args = []
if is_emscripten
  web_link_args = [
    '--shell-file', meson.current_source_dir() / 'minshell.html',
    '-sUSE_GLFW=3',
  ]
  add_project_arguments('-DPLATFORM=Web', language : 'c')
endif

example_names = [
  'raylib-nuklear-example',
  'raylib-nuklear-demo',
  'raylib-nuklear-font',
  'raylib-nuklear-font-default',
  'raylib-nuklear-texture',
]

foreach name : example_names
  target_name = name
  extra_kwargs = {}

  # Only the demo gets renamed to index.html for the Web build
  # (set_target_properties(raylib-nuklear-demo PROPERTIES SUFFIX ".html" OUTPUT_NAME "index"))
  if is_emscripten and name == 'raylib-nuklear-demo'
    target_name = 'index'
    extra_kwargs += {'name_suffix' : 'html'}
  endif

  executable(target_name, name + '.c',
    dependencies : [raylib_dep, raylib_nuklear_dep],
    c_args : warning_flags,
    link_args : web_link_args,
    override_options : ['c_std=c99'],
    kwargs : extra_kwargs,
  )
endforeach

# Resources
# configure_file(..., COPYONLY) just needs these sitting next to the built
# binaries at runtime.
foreach res : ['anonymous_pro_bold.ttf', 'test-image.png']
  configure_file(
    input : 'resources' / res,
    output : res,
    copy : true,
  )
endforeach
