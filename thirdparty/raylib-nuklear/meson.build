project('raylib_nuklear', 'c',
  version : '6.0.1',
  license : 'zlib',
  meson_version : '>=0.62.0',   # needed for the dict-kwargs trick used in examples/meson.build
)

# raylib_nuklear
subdir('include')

# Options
build_examples = get_option('build_examples') and not meson.is_subproject()

# Examples
if build_examples
  subdir('examples')

  # Testing
  subdir('test')
endif
