# win-shim

<img src="media/icon.svg" style="width:80px; float: left;" />



Lightweight and customisable shim executable for **Windows** written in safe modern `C++ 20`.

## Features

- Lightweight and Fast. Does not add to startup time, even for tiny utilities.
- Self-sufficient and clean. Does not read config files or write anywhere. Does not need or require any external runtimes.
- Mirrors original application icons, file information and version.
- Supports any command line, parametrised.
- Supports UI and command-line applications. UI applications are launched without console window, console apps get a console. Simple but tricky to implement.
- Respects original exit code.
- No hanging processes. Terminating shim terminates any process tree it has created.
- Fully UNICODE compliant.
- Secure
  - Disables dangerous UI capabilities:
    - Creating and switching desktops.
    - Changing display settings.
    - Logging of from Windows.
    - Changing system settings.
    - *Optionally* allows clipboard access.

## How to Use

Download the [latest release](https://github.com/aloneguid/win-shim/releases).

Originally this shim was written because I wanted to associate a file extension in Windows with [vim](https://www.vim.org/), but opened as a tab in [windows terminal](https://github.com/microsoft/terminal) instead of an ugly terminal popup window. WT [does allow it](https://docs.microsoft.com/en-us/windows/terminal/command-line-arguments?tabs=windows), however I would need to build a command line like:

```bash
wt -w 0 nt vim.exe <path_to_file>
```

Unfortunately (until now, thanks to me!), you cannot associate a command line with an extension in Windows, only executable with an extension, so I though I'd build a shim executable for this.

In order to achieve this, run `shmake` like following:

```bash
.\shmake.exe -i vim.exe -o vw.exe -a "-w 0 nt vim.exe %s" --app-path wt.exe -c no-kill
```

which should generate a new executable `winwt.exe` that when called with an argument will open a tab in WT! `%s` is replaced by arguments passed to `wimwt.exe` when it executes. `--app-path` tell shim to launch `wt.exe` instead of input. `-c` tells that shim must not kill target process when it exists (wt launches, signals existing instance and exits).

Yes, the shim will have vim's icon and looks exactly like `vim.exe` in Windows explorer, even when you click "properties" on it.


## Building

As this is **Windows Exclusive**, you need Visual Studio 2019+ with Windows SDK installed. Normally I would use CMake, but it is considerably harder (not impossible) when you need access to OS specific tools, especially native resources (which I utilise heavily to do the magic). It is also very well integrated with `vcpkg`.

`shmake` (but not shim) has dependency on:
- `boost::program_options` to present you with a nice command line.

All the dependencies are installed via [vcpkg](https://github.com/microsoft/vcpkg).

```
vcpkg install boost-program-options:x64-windows boost-program-options:x64-windows-static
```

`shim` **does not have any dependencies** and is kept as small and light as possible.

### Extra Oddness

`shmake` embeds `shim` inside it as a Windows native resource, so it's completely self-sufficient and can be distributed as a single `.exe`. To do that, `shmake`'s pre-build step copies `shim.exe` to `shim.bin` before build (as a pre-build step). Apparently `shim` is set as a project dependency of `shmake`, so it generates a full usable binary.

## Related Projects

- [scoop-better-shim](https://github.com/71/scoop-better-shimexe) does a few things right, but is not complete.
- [This stackoverflow answer](https://stackoverflow.com/a/14806704/80858) has a good suggestion on changing exe subsystem.
