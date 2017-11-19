# Master (will become release 2.7)

## build-system

- Variables passed via `dunecontrol`'s command `--configure-opts=..` are now
  added to the CMake flags.

- Bash-style variables which are passed to `dunecontrol`'s command `configure-opts`
  are no longer transformed to their equivalent CMake command. Pass
  `-DCMAKE_C_COMPILER=gcc` instead of `CC=gcc`.
