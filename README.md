# Pintos
Pintos is a simple operating system framework for the 80x86 architecture. This is our semester project for CS 333 at Alexandria University. We are assigned certain tasks to improve the implementation of Pintos.

## Quickstart

### Install Bochs
- Download the source code of [Bochs 2.2.6](https://sourceforge.net/projects/bochs/files/bochs/2.2.6/bochs-2.2.6.tar.gz/download) or [Bochs 2.6.2](https://sourceforge.net/projects/bochs/files/bochs/2.6.2/bochs-2.6.2.tar.gz/download)
- Use either of the two build scripts `src/misc/bochs-2.2.6-build.sh` and `src/misc/bochs-2.6.2-build.sh` depending on your downloaded version of Bochs to patch, build, and install it

  ```
  usage: env SRCDIR=<srcdir> PINTOSDIR=<srcdir> DSTDIR=<dstdir> sh src/misc/bochs-2.6.2-build.sh
    where <srcdir> contains bochs-2.6.2.tar.gz
      and <pintosdir> is the root of the pintos source tree
      and <dstdir> is the installation prefix (e.g. /usr/local)
  ```

### Run Pintos
- Setup the environment variables by sourcing `SETUP_ENV.sh` from the root of the repository:

  ```bash
  $ source SETUP_ENV.sh
  Updating PATH...
  Updating GDBMACROS...
  ```

- Build the source code of the project you are working on (e.g.: `threads`, `userprog`, `vm`, `filesys`):

  ```bash
  src/threads $ make
  ```

- Run Pintos in a simulator using `pintos`, for example:

  ```bash
  src/threads $ pintos -h
  ```

  Output:

  ```
  PiLo hda1
  Loading............
  Kernel command line: -h

  Command line syntax: [OPTION...] [ACTION...]
  Options must precede actions.
  Actions are executed in the order specified.

  Available actions:
    run TEST           Run TEST.

  Options:
    -h                 Print this help message and power off.
    -q                 Power off VM after actions or on panic.
    -r                 Reboot after actions.
    -rs=SEED           Set random number seed to SEED.
    -mlfqs             Use multi-level feedback queue scheduler.
  Timer: 0 ticks
  Thread: 0 idle ticks, 0 kernel ticks, 0 user ticks
  Console: 552 characters output
  Keyboard: 0 keys pressed
  Powering off...
  ```
