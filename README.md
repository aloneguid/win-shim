# win-shim

**<span style="color: red">THIS IS AN EARLY ALPHA!</span>**

Lightweight and customisable shim executable for **Windows**. Shims supposed to work just like target executables they are shadowing.

It's written in safe C++20.

## Features

- Lightweight and Fast. Does not add to startup time, even for tiny utilities.
- Self-sufficient and clean. Does not read config files or write anywhere. Does not need or require any external runtimes.
- Mirrors:
  - [ ] Executable name.
  - [ ] Icon.
  - [ ] Version information.
  - [ ] Exit status (error code).
- Extras
  - [ ] Limit registry access.
  - [ ] Limit filesystem access.
  - [ ] Limit amount of available RAM.
  - [ ] Change clock.

## How to Use

Download the latest release when ready.

## Building

As this is **Windows exclusive**, you need Visual Studio 2019 with Windows SDK installed. Normally I would use CMake, but is considerably harder (not impossible) to use when you need access to OS specific tools, especially native resources (which I utilise heavily to do the magic).

`shmake` (but not shim) has dependency on:
- `boost::program_options`.

All the dependencies are installed via [vcpkg](https://github.com/microsoft/vcpkg).

```
vcpkg install boost-program-options:x64-windows boost-program-options:x64-windows-static
```

`shim` does not have any dependencies and is kept as small and light as possible.

### Extra Oddness

`shmake` embeds `shim` inside it as a Windows native resource, so it's completely self-sufficient and can be distributed as a single `exe`. To do that, `smake`'s pre-build step copies `shim.exe` to `shim.bin` before build (as a pre-build step). Apparently `shim` is set as a project dependency of `shmake`, so it generates a full usable binary.