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

As this is **Windows exclusive**, you need Visual Studio 2019 with Windows SDK installed. The main reason for this is `CMake` is considerably harder (but not impossible) to use when you need access to OS specific tools.

`shmake` (but not shim) has dependency on:
- `boost::program_options`.