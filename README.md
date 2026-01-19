Munix: A Simple Operating System
======================================================================

INF-2203 Operating Systems Fundamentals  
UiT The Arctic University of Norway  

This repository contains skeleton code (aka "precode") for a simple operating
system that we call **Munix**.

This code forms the basis for the
Operating Systems Fundamentals course (INF-2203)
at UiT The Arctic University of Norway.

The name Munix comes from UiT's mascots,
the ravens Huginn and Muninn from Norse mythology.
The _mu_ can also refer to the Greek letter _μ_ as in the metric
prefix for _micro_, since this is a very tiny OS.

Introduction
----------------------------------------------------------------------

This operating system targets a 32-bit Intel x86 PC architecture, aka i386.
It should be possible to build from any Linux/Unix host environment
with a C compiler toolchain and a cross-compiler to i386.

- OS target architecture: **i386** (x86-32)
- Host build environment: **Linux** or other Unix/POSIX OS with GCC toolchain

<!-- -->

- Reference target emulator: **qemu-system-i386**
- Reference host build environment: **Ubuntu 24.04 (Noble Numbat)** on x86_64

For more information on setting up a build environment,
see the [Setting Up Your Build Environment](doc/10-build-env.md) doc.

- If you are running Windows,
    you should be able to build the code using a Linux environment in WSL.
    See [Building on Windows with WSL2](doc/15-build-env-wsl2.md).

- If you are running a Mac you,
    should be able to run the build in the Mac terminal.
    See [Building on a Mac](doc/18-build-env-mac.md).

For more information about the code and the assignments,
see the assignment hand-out lectures and slides.

Make Targets
----------------------------------------------------------------------

Once your build environment is set up,
the actual build is controlled by `make`.
These are the main targets:

<!-- Markdown note: I am using Python syntax for my shell examples
    because Doxygen's Markdown parser does not support bash/shell scripts,
    but Python has a similar-enough syntax with '#' comments. -->

```py
make            # Default: Run tests and build image

make dev        # Set up development tooling
make doc        # Generate documentation HTML (Doxygen)
make test       # Run tests
make image      # Build bootable disk image
make all        # Build all: doc, dev, test, image

make run        # Build image and launch it in an emulator
make debug      # Build image and debug it in an emulator

make clean      # Remove most built files
make distclean  # Remove all non-source files
```
