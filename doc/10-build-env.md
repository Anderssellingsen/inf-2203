Setting Up Your Build Environment
======================================================================

Linux Environment
======================================================================

This code is developed under Linux and we are assuming that you have
some kind of Linux (or other Unix) environment as a base.
From there we will install dependencies and the cross compiler
needed to build Munix.

- The reference build environment is Ubuntu 24.04 LTS (Noble Numbat).

    - If you use this distro, then you should be able to follow this guide
      step-by-step with no issues.

    - If you use another distro or another Ubuntu version,
      then you may have to make some tweaks to the procedure,
      such as adapting the `apt` example command lines to your
      distro's package manager or package names.

- If you are running Windows,
    you should be able to build the code using a Linux environment in WSL.
    See [Building on Windows with WSL2](15-build-env-wsl2.md).

- If you are running a Mac you,
    should be able to run the build in the Mac terminal.
    See [Building on a Mac](18-build-env-mac.md).

Once you have a base environment ready,
come back and follow the rest of this guide.

If you run into trouble,
check the [Build FAQ](19-build-faq.md).
Maybe your issue already has a workaround.

Building a Cross Compiler
======================================================================

In order to compile for our target architecture,
we need a cross compiler:
a compiler that runs on your machine (the _host_)
but that creates executables to be run on the _target_ architecture.

Even if your host machine has a similar architecture (e.g. x86_64 -> i386),
you still need a cross compiler. This is because we are compiling for our OS,
not Linux. Compiling for Linux enables many sophisticated CPU and ABI
features that we do not want to use. Instead, we will configure the compiler
to target a generic ELF-based OS, which is what we are building.
For more arguments for why you need a cross compiler,
see OSDev's
[Why do I need a Cross Compiler?](https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F)
page.

Build Essentials (Host C Compiler)
----------------------------------------------------------------------

First you need a normal C compiler toolchain in order to build the
cross compiler toolchain.
On apt-based systems (Debian, Ubuntu, etc.),
there is a package called `build_essential` that handles that.

<!-- Markdown note: I am using Python syntax for my shell examples
    because Doxygen's Markdown parser does not support bash/shell scripts,
    but Python has a similar-enough syntax with '#' comments. -->

```py
# Install base C compiler and other essential tools.
sudo apt install build-essential
```

Prep: Versions and Directories
----------------------------------------------------------------------

### Set variables

First, let's set some variables that we will refer to later on:

```py
# Target architecture: i386 with a generic ELF-based ABI
export CROSS_TARGET=i386-elf

# Versions
# As of January 2026, these versions match the versions
# that are packaged in Ubuntu 24.04 LTS (noble).
export BINUTILS_VERSION=2.40
export GCC_VERSION=13.4.0
export GDB_VERSION=15.2
export GRUB_VERSION=2.06

# GNU FTP mirror site
#   https://ftpmirror.gnu.org will choose automatically
export GNU_MIRROR=https://ftpmirror.gnu.org
```

### Choose paths

You need three paths:

1. A source directory to download source code to
2. A working directory to build the code in
3. A more permanent directory to install the cross compiler to

Let's say you are putting all your work for this course under
your home directory, in `~/projects/uit-inf2203/`.
You could do something like this:

```py
export CROSS_SRC=$HOME/projects/uit-inf2203/cross_src
export CROSS_BUILD=$HOME/projects/uit-inf2203/cross_build
export CROSS_INSTALL=$HOME/projects/uit-inf2203/cross_install

# Make sure the directories exist.
mkdir -p $CROSS_SRC $CROSS_BUILD $CROSS_INSTALL
```

GNU binutils and GCC
----------------------------------------------------------------------

For a cross compiler, we need to compile two GNU packages from source:

1. [GNU binutils](https://www.gnu.org/software/binutils/)
    --- low-level utilities for manipulating code and object files,
        including an assembler, a linker,
        and tools like `readelf` and `objdump`

2. [GCC, the GNU Compiler Collection](https://gcc.gnu.org/)
    --- The actual C compiler

To build these we will need a handful of GNU support tools and libraries
that should be available in your distro's package manager.

### Dependency packages for binutils and GCC

First we need to install some build dependencies.

```py
# GNU Math libraries
# These help the compiler implement math operations.
#   libgmp-dev  --- GNU Multiple-Precision math (integers)
#   libmpfr-dev --- GNU Multiple-Precision Floating-point Rounding
#   libmpc-dev  --- GNU Multiple-Precision Complex math
sudo apt install libgmp-dev libmpfr-dev libmpc-dev

# Compiler generators
# These help the compiler parse different languages.
#   flex        --- Lexical analysis
#   bison       --- Syntax/grammar parsing
sudo apt install flex bison

# Documentation tools
#   texinfo     --- GNU's document processing system
sudo apt install texinfo
```

### Optional packages

The [cURL](https://curl.se/) tool will be useful for downloading
source code from the command line.
It likely came installed by default in your distro,
but if it didn't, you can install it like so:

```py
# Command-line download tool and SSL certificates
#   curl            --- cURL, a tool that handles many protocols
#   ca-certificates --- Common public certificates for SSL
sudo apt install curl ca-certificates
```

### Download sources

Once we have installed the prerequisite libraries,
we can build the cross compiler from source.

```py
# Switch to source directory.
cd $CROSS_SRC

# If you have cURL installed, you can download via command line.
curl -LfO $GNU_MIRROR/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
curl -LfO $GNU_MIRROR/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz

# Or, print and copy the URLs and open them in your browser.
echo $GNU_MIRROR/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
echo $GNU_MIRROR/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
```

### Unpack sources

```py
# Switch to source directory.
cd $CROSS_SRC

tar -xzf binutils-$BINUTILS_VERSION.tar.gz
tar -xzf gcc-$GCC_VERSION.tar.gz
```

### Build and install binutils

The _configure_ step sets up the build:
what target architecture we want,
where to install the compiler, etc.

```py
# Switch to the build directory.
cd $CROSS_BUILD

# Create a binutils build directory,
# separate from the source tree.
mkdir binutils-$BINUTILS_VERSION-$CROSS_TARGET
cd binutils-$BINUTILS_VERSION-$CROSS_TARGET

# From the build directory,
# reach back into the source directory
# and run the configure script.
$CROSS_SRC/binutils-$BINUTILS_VERSION/configure \
                --target=$CROSS_TARGET --prefix=$CROSS_INSTALL \
                --disable-nls

# Build.
make -j `nproc --ignore=2`

# Install.
make install
```

If you get an error
"`make: option '--ignore-errors' doesn't allow an argument`,"
then [double-check the backticks in the command](19-build-faq.md).

Once you have binutils installed, you should see files in your
`$CROSS_INSTALL` directory. You should especially see the binutils
programs under `$CROSS_INSTALL/bin`:

```py
ls $CROSS_INSTALL/bin
# i386-elf-as
# i386-elf-ld
# i386-elf-nm
# i386-elf-objdump
# i386-elf-readelf
```

Note that these are prefixed with the target architecture.
So when we want to assemble for our target system,
we run `i386-elf-as` instead of just `as`.

### Add binutils to your $PATH

Now, in order to use these binutils, add the binutils directory to your PATH:

- To add it to the path in the current terminal session, run this:

    `export PATH=$CROSS_INSTALL/bin:$PATH`

- To have it automatically added to your path when you log in,
    open the file `.profile` in your home directory and add this line
    (be sure to change it to the actual location of your `$CROSS_INSTALL`
     directory):

    `export PATH=$HOME/projects/uit-inf2303/cross_install/bin`

To check if the binutils are in your path, use the `which` command:

```py
which i386-elf-as
# /home/user/projects-uit-inf2203/cross_install/bin/i386-elf-as
```

### Build and install GCC

Now that you have binutils available, you can compile GCC.
The procedure is the same as with binutils: `configure` and then `make`.

The GCC configuration flags given here specify a minimal install
to save on compilation time. For more GCC configure options,
see [Installing GCC: Configuration](https://gcc.gnu.org/install/configure.html)
in the GCC Manual.

```py
# Switch to the build directory.
cd $CROSS_BUILD

# Create a GCC build directory,
# separate from the source tree.
mkdir gcc-$GCC_VERSION-$CROSS_TARGET
cd gcc-$GCC_VERSION-$CROSS_TARGET

# From the build directory,
# reach back into the source directory
# and run the configure script.
$CROSS_SRC/gcc-$GCC_VERSION/configure \
                --target=$CROSS_TARGET --prefix=$CROSS_INSTALL \
                --disable-nls --without-headers \
                --enable-languages=c

# Build GCC.
# NB: Building GCC can take on the order of 10--20 minutes or more.
make all-gcc            -j `nproc --ignore=2`
make all-target-libgcc  -j `nproc --ignore=2`

# Install.
make install-gcc
make install-target-libgcc
```

Now you should have the cross compiler installed in `$CROSS_INSTALL/bin`:

```py
ls $CROSS_INSTALL/bin
# ... i386-elf-gcc ...

which i386-elf-gcc
# /home/user/projects/uit-inf2203/cross_install/bin/i386-elf-gcc
```

Debugger: GDB
----------------------------------------------------------------------

You will certainly want a debugger to help with development.

### Using distro GDB (if on x86)

If your host CPU architecture is related to the target architecture (e.g.
x86_64 -> i386), then you should be able to just use the GDB that is packaged
with your distro.

```py
# Install GDB for x86
# (if on x86 and not already installed).
sudo apt install gdb
```

If you are on a different architecture, then you will need to build
a cross debugger that can understand the target's machine code.

### Dependency packages for building GDB

```py
# For building GDB
#   pkg-config          --- A build configuration utility
#   libncurses-dev      --- Classic terminal library for text UIs
#   libsource-highlight-dev --- Syntax highlighting
sudo apt install pkg-config libncurses-dev libsource-highlight-dev
```

### Building GDB

The procedure for building GDB is the same as the GNU binutils and GCC:
download, extract, configure, make, install.

```py
# Switch to source directory.
cd $CROSS_SRC

# Download and extract source.
curl -LfO $GNU_MIRROR/gnu/gdb/gdb-$GDB_VERSION.tar.gz
tar -xzf gdb-$GDB_VERSION.tar.gz

# Switch to build directory.
cd $CROSS_BUILD
mkdir gdb-$GDB_VERSION-$CROSS_TARGET
cd gdb-$GDB_VERSION-$CROSS_TARGET

# From build directory, run source directory's configure script.
$CROSS_SRC/gdb-$GDB_VERSION/configure \
                --target=$CROSS_TARGET --prefix=$CROSS_INSTALL \
                --disable-nls

# Make and install.
make -j `nproc --ignore=2`
make install

# Check.
which i386-elf-gdb
# /home/user/projects/uit-inf2303/cross_install/bin/i386-elf-gdb
```

Bootloader: GRUB
----------------------------------------------------------------------

We also need a bootloader for our OS.
We will be using GRUB.

### Using distro GRUB (if on x86)

If your host system has an x86 CPU, then you should be able to use
the GRUB that came with your distro and skill the rest of this section.
You might just have to install the `grub-pc` package to get old-school
PC BIOS support.

```py
# Install GRUB for PC BIOS
# (if on x86 and not already installed).
#   grub-pc     --- GRUB with support for old-school PC BIOS
#   mtools      --- Tools for working with old-school FAT filesystems
sudo apt install grub-pc mtools
```

If you are on a different architecture,
you will have to build GRUB from source.

### Dependencies for building GRUB

```py
# For building GRUB
#   python3     --- Python interpreter (probably already installed)
sudo apt install python3
```

### Building GRUB

```py
# Switch to source directory.
cd $CROSS_SRC

# Download and extract.
curl -LfO $GNU_MIRROR/gnu/grub/grub-$GRUB_VERSION.tar.gz
tar -xzf grub-$GRUB_VERSION.tar.gz

# Switch to build directory.
cd $CROSS_BUILD
mkdir grub-$GRUB_VERSION-$CROSS_TARGET
cd grub-$GRUB_VERSION-$CROSS_TARGET

# From build directory, run source directory's configure script.
$CROSS_SRC/grub-$GRUB_VERSION/configure \
                --target=$CROSS_TARGET --prefix=$CROSS_INSTALL \
                --disable-nls --disable-werror

# Make and install.
make -j `nproc --ignore=2`
make install

# Check.
which grub-mkrescue
/home/user/projects/uit-inf2203/cross_install/bin/grub-mkrescue
```

Building and Running the Munix OS
======================================================================

Once the cross compiler and other packages are installed,
you are almost ready to build the Munix OS boot image and boot the OS.

Dependencies to build and run the boot image
----------------------------------------------------------------------

You need a few more packages for our build,
beyond the cross compiler.

```py
# For building and running the bootable disk image
#   cpio        --- Creates the cpio ramdisk archive
#   xorriso     --- Creates CD-ROM ISO images, used by GRUB
#   qemu-system-i386    --- i386 emulator
sudo apt install cpio xorriso qemu-system-i386
```

Make Targets Summary
----------------------------------------------------------------------

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

Basic Build and Boot
----------------------------------------------------------------------

First switch to the precode directory,
then run `make` to build the image.

```py
# Build the boot image.
make image

# This creates an ISO image file.
ls out/i386-elf/bootimage.iso
```

Once the image is created, you can launch it in the emulator.

```py
# Boot in the emulator.
# Use the QEMU UI to the serial view to see the serial output.
qemu-system-i386 -cdrom out/i386-elf/bootimage.iso -boot d

# Add '-serial stdio' to redirect serial I/O to your terminal.
# Then you can see the serial output and screen at the same time.
qemu-system-i386 -cdrom out/i386-elf/bootimage.iso -boot d \
        -serial stdio
```

If you have your cross compiler and your emulator installed in the same
environment (i.e. you haven't done anything fancy with containers),
then you can use the `make run` target to build and then launch with
one command.

```py
# Convenience target to build and launch with one command.
make run

# Use the QEMUFLAGS variable to pass additional arguments to QEMU.
make run QEMUFLAGS="-serial stdout"
```

Debugging with GDB
----------------------------------------------------------------------

QEMU has an option to connect to the GDB debugger and let you debug
the OS inside the emulator.

If you have QEMU and GDB installed in the same environment,
then you can use the `make debug` target to build, launch, and debug
all with one command.

```py
# Convenience target to build and debug.
make debug

# Then in the GDB console, try setting a breakpoint at kernel start
# and then stepping through some instructions.
break _start
continue
step
step
# ...
```

If you do have QEMU and GDB in different environments
(e.g. if you are using containers or virtual machines),
then you should still be able to connect GDB and QEMU
over TCP/IP.

```py
# In one terminal, launch QEMU, listening for GDB on a socket.
qemu-system-i386 -cdrom out/i386-elf/bootimage.iso -boot d \
    -gdb tcp:localhost:1234 -S

# In another terminal, launch GDB with a command to connect.
gdb out/i386-elf/kernel/kernel -ex "target remote localhost:1234"
```

Generating Documentation with Doxygen
----------------------------------------------------------------------

If you have [Doxygen and graphviz installed](#prereqs_make_optional),
you can generated a searchable, cross-referenced documentation website
that you can open in your browser.

```py
# Dependencies for generating HTML documentation
#   doxygen         --- C documentation generator
#   graphviz        --- Graph drawing tool, used by Doxygen
sudo apt install doxygen graphviz
```

```py
# Generate documentation.
make doc

# Open the generated HTML in your browser.
# Use this command to print the full path of the index.html file,
# and then paste it into your browser window.
readlink -f out/doxygen/html/index.html
```

Configuring Your IDE
----------------------------------------------------------------------

C tooling can be tricky, since the C compiler takes so many flags
to specify different include directories and `#define` macros.
This is especially true for operating systems development,
since we are not using the typical C standard library headers that are
installed on the host system.

If your IDE reports errors all through each C file even though `make`
completes without an issue, this is the problem.
You need to give your IDE more information about how this C is compiled.

### Generating Necessary Databases

The Munix build system supports two types of tooling:

- [CTAGS](https://en.wikipedia.org/wiki/Ctags),
    an old-school indexing system for C code used by editors like
    vim and emacs.
    Requires [ctags tool](https://ctags.io/).

- [compile_commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html),
    a list of the specific commands used (and their flags),
    which is used by VSCode and other
    editors that support the Language Server Protocol (LSP).
    Requires [`bear` utility](https://github.com/rizsotto/Bear).

```py
# Dependencies for editor tooling
#   universal-ctags --- Old-school code indexer (vim, emacs, etc.)
#   bear            --- New tool (VSCode and LSP servers)
sudo apt install universal-ctags bear
```

```py
# Build all tooling databases.
make dev

# Build only ctags.
make tags

# Build only compile_commands.json.
make compile_commands.json
```

### Configuring VSCode

For VSCode, you may need to tell it where to find the `compile_commands.json`
file. To do this:

1. Open the Microsoft C/C++ Extension's IntelliSense Configurations screen.

    1. Open a C code file in the project.
    2. In the status bar, next to the language setting ("C"),
        there is a place to select the C/C++ Configuration to use.
        Click this.
    3. A pop-up will ask you to "Select a Configuration".
        Click "Edit Configurations (UI)"

2. Expand the "Advanced Settings" at the bottom.

3. Find the "Compile Commands" heading.

4. In the compile commands box, add this path:

    `${workspaceFolder}/compile_commands.json`
