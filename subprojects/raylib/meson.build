project('raylib', 'c',
  version : '6.0.0',
  license : 'zlib',
  default_options : [
    'default_library=static',   # BUILD_SHARED_LIBS=ON  ->  -Ddefault_library=shared
    'warning_level=1',
  ],
)

# WITH_PIC / ENABLE_ASAN / ENABLE_UBSAN / ENABLE_MSAN from CMakeOptions.txt
# are Meson builtins rather than custom options (see meson_options.txt):
#   -Db_staticpic=true            (WITH_PIC)
#   -Db_sanitize=address          (ENABLE_ASAN)
#   -Db_sanitize=undefined        (ENABLE_UBSAN)
#   -Db_sanitize=memory           (ENABLE_MSAN, can't combine with address)

fs = import('fs')
cc = meson.get_compiler('c')

version_array = meson.project_version().split('.')
api_version   = '@0@.@1@'.format(version_array[0], version_array[1])

raylib_sources = [
  'src/raudio.c',
  'src/rcore.c',
  'src/rmodels.c',
  'src/rshapes.c',
  'src/rtext.c',
  'src/rtextures.c',
]

raylib_public_headers = [
  'src/raylib.h',
  'src/rcamera.h',
  'src/rlgl.h',
  'src/raymath.h',
]

platform            = get_option('platform')
graphics            = get_option('graphics')
build_shared_libs   = get_option('default_library') == 'shared'
include_everything  = get_option('include_everything')

compiler_flags = []
link_args      = []
dependencies   = []
include_dirs   = ['src']
defines        = []

# ---------------------------------------------------------------------------
# 'desktop' (PLATFORM_DESKTOP -> rcore.c auto-forces PLATFORM_DESKTOP_GLFW)
# has been removed along with src/external/glfw. RGFW is the only desktop
# backend wired up below.
# ---------------------------------------------------------------------------
if platform == 'web'
  defines += 'PLATFORM_WEB'
  if graphics == 'opengles3'
    defines += 'GRAPHICS_API_OPENGL_ES3'
  else
    defines += 'GRAPHICS_API_OPENGL_ES2'
  endif

elif platform == 'android'
  defines += ['PLATFORM_ANDROID', 'GRAPHICS_API_OPENGL_ES2']
  dependencies += dependency('log', required : true)
  dependencies += dependency('android', required : true)
  dependencies += dependency('egl', required : true)
  dependencies += dependency('glesv2', required : true)

elif platform == 'drm'
  defines += ['PLATFORM_DRM', '_DEFAULT_SOURCE', 'GRAPHICS_API_OPENGL_ES2', 'EGL_NO_X11']
  dependencies += dependency('drm', required : true)
  dependencies += dependency('gbm', required : false)
  dependencies += dependency('egl', required : false)
  dependencies += dependency('glesv2', required : false)

elif platform == 'sdl'
  defines += 'PLATFORM_DESKTOP_SDL'
  sdl_dep = dependency('sdl3', required : false)
  if not sdl_dep.found()
    sdl_dep = dependency('sdl2', required : true)
  endif
  dependencies += sdl_dep
  dependencies += dependency('opengl', required : true)

elif platform == 'rgfw'
  defines += 'PLATFORM_DESKTOP_RGFW'
  # Link requirements straight from src/external/RGFW/RGFW.h's own header
  # comments:
  #   linux : gcc main.c -lX11 -lXrandr -lGL
  #   windows : gcc main.c -lopengl32 -lgdi32
  #   macos : gcc main.c -framework Cocoa -framework CoreVideo -framework OpenGL -framework IOKit
  if host_machine.system() == 'darwin'
    dependencies += dependency('appleframeworks', modules : ['Cocoa', 'CoreVideo', 'OpenGL', 'IOKit'])
  elif host_machine.system() == 'windows'
    dependencies += dependency('opengl', required : true)
    dependencies += dependency('gdi32', required : true)
  elif host_machine.system() == 'linux'
    dependencies += dependency('x11', required : true)
    dependencies += dependency('xrandr', required : true)
    dependencies += dependency('opengl', required : true)
    dependencies += dependency('glx', required : false)
  endif
endif

# threads is needed by raylib itself regardless of windowing backend, not
# just the ones that used to request it individually above.
dependencies += dependency('threads', required : true)

# ---------------------------------------------------------------------------
# Graphics API (mirrors CompileDefinitions.cmake)
# ---------------------------------------------------------------------------
if graphics == 'opengl33'
  defines += 'GRAPHICS_API_OPENGL_33'
elif graphics == 'opengl43'
  defines += 'GRAPHICS_API_OPENGL_43'
elif graphics == 'opengles2'
  defines += 'GRAPHICS_API_OPENGL_ES2'
elif graphics == 'opengles3'
  defines += 'GRAPHICS_API_OPENGL_ES3'
elif platform == 'rgfw' or platform == 'sdl'
  defines += 'GRAPHICS_API_OPENGL_33'
endif

# ---------------------------------------------------------------------------
# Modules / optional file formats
#
# CMakeOptions.txt generates these from config.h via ParseConfigHeader.cmake
# + a `foreach(FLAG IN LISTS CONFIG_HEADER_FLAGS)` loop, so any new SUPPORT_*
# flag added to config.h automatically becomes a CMake option. Meson has no
# equivalent generic header-scraping step, so the known flag set is listed
# explicitly here (and in meson_options.txt) instead — you'll need to add a
# line in both files if raylib's config.h grows a new SUPPORT_* flag.
# ---------------------------------------------------------------------------
if get_option('support_module_rshapes')
  defines += 'SUPPORT_MODULE_RSHAPES=1'
else
  defines += 'SUPPORT_MODULE_RSHAPES=0'
endif
if get_option('support_module_rtextures')
  defines += 'SUPPORT_MODULE_RTEXTURES=1'
else
  defines += 'SUPPORT_MODULE_RTEXTURES=0'
endif
if get_option('support_module_rtext')
  defines += 'SUPPORT_MODULE_RTEXT=1'
else
  defines += 'SUPPORT_MODULE_RTEXT=0'
endif
if get_option('support_module_rmodels')
  defines += 'SUPPORT_MODULE_RMODELS=1'
else
  defines += 'SUPPORT_MODULE_RMODELS=0'
endif

if get_option('support_module_raudio') and get_option('use_audio')
  defines += 'SUPPORT_MODULE_RAUDIO=1'
  dependencies += dependency('openal', required : false)
else
  defines += 'SUPPORT_MODULE_RAUDIO=0'
endif

if get_option('support_tracelog')
  defines += 'SUPPORT_TRACELOG=1'
else
  defines += 'SUPPORT_TRACELOG=0'
endif
if get_option('support_camera_system')
  defines += 'SUPPORT_CAMERA_SYSTEM=1'
else
  defines += 'SUPPORT_CAMERA_SYSTEM=0'
endif
if get_option('support_gestures_system')
  defines += 'SUPPORT_GESTURES_SYSTEM=1'
else
  defines += 'SUPPORT_GESTURES_SYSTEM=0'
endif
if get_option('support_compression_api')
  defines += 'SUPPORT_COMPRESSION_API=1'
else
  defines += 'SUPPORT_COMPRESSION_API=0'
endif
if get_option('support_automation_events')
  defines += 'SUPPORT_AUTOMATION_EVENTS=1'
else
  defines += 'SUPPORT_AUTOMATION_EVENTS=0'
endif

foreach fmt : ['tga', 'jpg', 'bmp', 'hdr', 'ktx', 'astc', 'pvr', 'obj', 'gltf', 'vox', 'flac']
  if get_option('support_fileformat_' + fmt) or include_everything
    defines += 'SUPPORT_FILEFORMAT_' + fmt.to_upper() + '=1'
  endif
endforeach

if build_shared_libs
  defines += 'BUILD_LIBTYPE_SHARED'
  compiler_flags += '-fvisibility=hidden'
endif

if host_machine.system() != 'windows'
  link_args += '-lm'
endif

defines_args = []
foreach d : defines
  defines_args += '-D' + d
endforeach
all_c_args = compiler_flags + defines_args

# ---------------------------------------------------------------------------
# The library target (mirrors add_library(raylib ...) + set_target_properties
# + install(TARGETS raylib) from src/CMakeLists.txt / InstallConfigurations.cmake)
# ---------------------------------------------------------------------------
if build_shared_libs
  raylib_lib = shared_library('raylib',
    sources : raylib_sources,
    dependencies : dependencies,
    link_args : link_args,
    include_directories : include_dirs,
    c_args : all_c_args,
    version : meson.project_version(),
    soversion : api_version,
    install : true,
  )
else
  raylib_lib = static_library('raylib',
    sources : raylib_sources,
    dependencies : dependencies,
    link_args : link_args,
    include_directories : include_dirs,
    c_args : all_c_args,
    install : true,
  )
endif

raylib_dep = declare_dependency(
  include_directories : include_dirs,
  sources : raylib_public_headers,
  dependencies : dependencies,
  link_args : link_args,
  link_with : raylib_lib,
)

install_headers(raylib_public_headers, subdir : 'raylib')

pkg = import('pkgconfig')
pkg.generate(raylib_lib,
  name : 'raylib',
  description : 'Simple and easy-to-use library to enjoy videogames programming',
  version : meson.project_version(),
  filebase : 'raylib',
)

# removable with `ninja -C builddir uninstall`.

# BUILD_EXAMPLES: only meaningful when raylib is the top-level project,
if get_option('build_examples') and not meson.is_subproject()
  if fs.exists('examples/meson.build')
    subdir('examples')
  else
    warning('build_examples is enabled but examples/meson.build does not exist — skipping')
  endif
endif
