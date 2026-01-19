Building on Windows with WSL2
======================================================================

Windows facilitates to run thin virtual machines on top of an optimized subset
of hyper-v, we know it as Windows Subsystem for Linux. WSL gives support to
follow the step-by-step instructions in the Linux Build Environment
documentation. %% QEMU, crosscompiler, gdb, gcc %% 

WSL 2 Installation
----------------------------------------------------------------------

### Fresh Install of WSL 2

For newer builds of Windows the installation steps should be a one-liner from Powershell.

1. Open your preferred version of Powershell as administrator
2. List the available distributions
    1. `wsl --list --online`
3. Install WSL
    1. `wsl --install <distribution name>` i.e.
    2. `wsl --install Ubuntu`(default)

If you have trouble, see
[Microsoft's WSL troubleshooting page](https://learn.microsoft.com/en-us/windows/wsl/troubleshooting#installation-issues).

### Upgrade from WSL 1 to WSL 2

1. Open your preferred version of Powershell as administrator
2. Check the version of WSL
    1. `wsl.exe --list --verbose`
3. Upgrading from WSL 1 to WSL 2
    1. `wsl.exe --set-version <Distro> <1|2>`
    2. `wsl.exe --set-version Ubuntu 2`

### Setting Up the Course Code Inside WSL

Once you have your instance of Ubuntu Linux running inside of WSL,
you should be able to follow the course's main
[Linux setup guide](10-build-env.md)
to install your cross compiler and other tools needed for the build.

### More Tips from Microsoft

Microsoft also has a series of useful tutorials for
[Setting up a WSL development environment](https://learn.microsoft.com/en-us/windows/wsl/setup/environment).
These guides cover:

1. Creating your user in your new WSL2 distribution
2. How to update and upgrade packages
3. Setting up [Windows Terminal](https://github.com/microsoft/terminal)
4. What to do with your file system and much more.

These are written for developers who are used to Windows
but new to Linux, so they may be a helpful introduction.

### Running Graphical Linux Apps from Inside WSL (X Server)

You should be able to run graphical Linux apps from inside WSL,
unless you are running an older build of Windows.

Linux takes a client-server approach to graphics with a protocol called
[the X Window System](https://en.wikipedia.org/wiki/X_Window_System),
often called X11 or just _X_. The display is managed by an X server,
and apps connect as clients. There is also a competing
display server called
[Wayland](https://en.wikipedia.org/wiki/Wayland_(protocol)).

Microsoft now includes support for running X Windows and Wayland programs
in WSL via an extension called Windows Subsystem for Linux GUI (WSLg).
WSLg was released in 2021 at Microsoft's Build Conference.
It has been available in Windows Insider preview editions of Windows 10 since
build 21364, and is part of public production builds of Windows 11. So if you
have a recent version of Windows, GUI Linux apps should hopefully work out of
the box.

If you have older versions of Windows, you will have to configure an X-server
on the host side and route graphics to this server from inside WSL. We will not
go in depth on that here, contact one of the staff members for help on setting
this up or check out the links below.

- [The stack on VcXsrc](https://stackoverflow.com/questions/61110603/how-to-set-up-working-x11-forwarding-on-wsl2)
- [x410](https://x410.dev/cookbook/wsl/using-x410-with-wsl2/)

VSCode in WSL
----------------------------------------------------------------------

Many of us use VSCode as editor, and the process of installing it in WSL is
a bit different than normal. With WSL we will need to have VSCode installed on
the `host`(Windows).

1. Go to [VSCode](https://code.visualstudio.com/) and install VSCode for Windows(host)
    1. **Important**: make sure you select **Add to path** during the installation, just to simplify using VSCode later.
2. After installing VSCode, open up a folder you would like to open in VSCode, i.e. home: `cd ~`
3. Open the folder
    1. `code .`
4. Recommended base extensions for VSCode:(ctrl+shit+X)
    1. Remote Development: open folders in remote machines/containers
    2. C/C++: IntelliSense, debugging and code browsing
    3. MakefileTools: IntelliSense, build, debug/run

Contact any of the staff if you run into problems!

Github Setup
----------------------------------------------------------------------

Since we will be using GitHub for our projects, we recommend having a look at
GitHub's
[Quickstart](https://docs.github.com/en/get-started/quickstart/about-github-and-git)
on how to set up an account and get started with their hosting services.

### SSH for Password-Free Access

We recommend setting up a pair of ssh keys instead of using username/password
to authenticate when using git.

1. Generate your keys
    1. `ssh-keygen -t ed_25519 -C <a-useful-comment-about-this-key-pair>`
2. Upload your `public` key to GitHub. **NOT YOUR PRIVATE**
    1. The key-pair is usually found in `~/.ssh/` as `id_ed25519`(private) and `id_ed25519.pub`(public)
    2. Copy the content of `id_ed25519.pub` and go to [GitHub settings keys](https://github.com/settings/keys)
    3. Press `new SSH key` and add your public key

You should now be able to authenticate with SSH instead of username/password
when interacting with GitHub.

**Note**: You have to redo this process for each machine you want access on.

### Source Code on GitHub

To retrieve the source code for our projects, we recommend setting up a private repository where you and your group can work together with version control.

1. Create a new repository for your private use
    - We won't be able to use the course repository for our code.
    - Recommend a hosting platform, such as GitHub.
      - Create a new repository and copy the remote URL
        - Either SSH: `git@github.com:<username>/<repository-name>.git` or HTTPS: `https://github.com/<username>/<repository-name>.git`
2. Now we need to get it down to your local machine, you have two choices. Choose ONE path
    1. Clone the repository
       1. `cd <path-to-a-good-location>`
       2. `git clone <git@github.com:username/repository-name.git`
    2. Initialize the repository locally and configure a remote
        1. `mkdir <name-of-your-liking>`
        2. `cd <name-of-your-liking>`
        3. `git init`
        4. `git remote add <name> <url>` i.e.
           1. `git remote add origin <git@github.com:username/my-awesome-OS.git`
        5. You can verify that you have both push and fetch(pull)
           1. `git remote -v`
           2. Should give you an output on the format
              1. `<name> <url> (fetch)`
              2. `<name> <url> (push)`

We should be able to work with our repository like we're used to.

In this course, we will distribute patches and push new starting code for the different projects under our GitHub organization; we need to add that remote to our repository to get the updates.

1. Add remote to our repo:
    1. `git remote add <name> <url>` i.e.
      -  `git remote add source git@github.com:uit-inf-2201-s24/project-os.git`

We only want to pull down updates and new starting code from this repository, so we'll remove the possibility of pushing to it.

2. `git remote set-url --push <name> <anything-but-a-valid-link>` i.e.
    - `git remote set-url --push source disabled_push`

You can see that the remote link for source is disabled with `git remote -v`, or try `git push source main`, and you should expect an output like:

```
origin <url> (fetch)
origin <url> (push)
source <url> (fetch)
source disabled_push (push)
```

After these steps, you can try merging in the source code from project-os with:
- `git pull <remote> <branch>` i.e.
  - `git pull source main`

And to finally sync it up with our private remote:
- `git push -u <name> <branch>` i.e.
  - `git push -u origin main`

The `-u` flag will add the upstream(GitHub) reference as default for our local branch, so we don't have to specify our remote target for every `pull` and `push`.
