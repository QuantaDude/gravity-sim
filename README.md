# Gravity N-Body Simulator


## Dependencies

- A C compiler which supports the C17 standard (Clang recommended).
- Meson
- Ninja

## Install Dependencies

### Windows:
```sh
winget install --id mesonbuild.meson --id Ninja-build.Ninja --id LLVM.LLVM
```

### Arch Based Linux:
```bash
sudo pacman -S meson ninja clang
```

## Build
configure the project with Meson:

> Note: To use clang toolchain specifically, you need to set the CC environment
> variable in the shell before the configuration.

```bash
meson setup build
```

compile:

```bash
meson compile -C build
```
or
```bash
ninja -C builddir
```
For a debug build:

```bash
meson setup build --buildtype=debug
meson compile -C build
```
For an optimized release build:

```bash
meson setup build --buildtype=release
meson compile -C build
```

## Run

### Linux

```bash
./build/gravity-sim
```

### Windows

```bash
build\gravity-sim.exe
```


## Note


