Build FAQ
======================================================================

Common Errors
--------------------------------------------------

### make: option '--ignore-errors' doesn't allow an argument

When [setting up your build environment](10-build-env.md)
and running this command, you may see this error:

```
$ make -j nproc --ignore=2
make: option '--ignore-errors' doesn't allow an argument
Usage: make [options] [target] ...
Options:
  ...
```

#### Solution: check the command for backticks

The part of the command that says `nproc --ignore=2` is supposed to
be quoted with backticks.
Make sure you are typing / copy-pasting the command exactly as written:

```
make -j `nproc --ignore=2`
```

#### Explanation: backticks are special shell syntax

Backticks in shell syntax tell the shell
"run this as a separate command first,
 and then insert the output back into the command line."

```
make -j `nproc --ignore=2`
        |----------------|
         This part is a separate command that is run first.
```

The command `nproc` is short for "number of processors."
It simply prints the number of processors or CPU cores you have.
The command `nproc --ignore=2` counts all but two processors.
For example, on a 16-core PC, it prints "14":

```
$ nproc --ignore=2
14
```

This then gets plugged into Make's `-j` option,
which tells it how many jobs to run at once.

```
make -j 14
```

This speeds up compilation by using multiple cores,
while leaving two for other tasks.

If you lose the backticks, then `nproc --ignore=2` just get passed as
arguments to Make, and then Make gets confused.
The closest thing it has to an `--ignore=2` option is `--ignore-errors`,
and that does not take an `=2` argument.
Hence, "option '--ignore-errors' doesn't allow an argument."
