<!--
SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
-->

DUNE-library
============

DUNE, the Distributed and Unified Numerics Environment is a modular toolbox
for solving partial differential equations with grid-based methods.

The main intention is to create slim interfaces allowing an efficient use of
legacy and/or new libraries. Using C++ techniques DUNE allows one to use very
different implementation of the same concept (i.e. grid, solver, ...) under
a common interface with a very low overhead.

DUNE was designed with flexibility in mind. It supports easy discretization
using methods, like Finite Elements, Finite Volume and also Finite
Differences. Through separation of data structures DUNE allows fast Linear
Algebra like provided in the ISTL module, or usage of external libraries
like BLAS.

This package contains the basic DUNE common classes.

Dependencies
------------

dune-common depends on the following software packages

- CMake >= 3.16
- Compiler (C, C++): GNU >= 10 or Clang >= 13
- Library: GNU libstdc++ >= 10 or Clang libc++ >= 13

Other compilers may also work, but they must support C++20 to the same extent as
the above. For an overview of the C++20 features supported by these versions, see
https://en.cppreference.com/w/cpp/compiler_support. For compiling the Python
bindings, libc++ is not currently supported.

The following software is recommended but optional:

- pkg-config
- MPI (either OpenMPI, lam, or mpich suffice)
- Python >= 3.7 (interpreter and development kit for building the python bindings)

License
-------

The DUNE-library and headers are licensed under version 2 of the GNU
General Public License, with the so-called "runtime exception", as
follows:

> As a special exception, you may use the DUNE source files as part
> of a software library or application without restriction.
> Specifically, if other files instantiate templates or use macros or
> inline functions from one or more of the DUNE source files, or you
> compile one or more of the DUNE source files and link them with
> other files to produce an executable, this does not by itself cause
> the resulting executable to be covered by the GNU General Public
> License.  This exception does not however invalidate any other
> reasons why the executable file might be covered by the GNU General
> Public License.

This license clones the one of the libstdc++ library. For further
implications of this library please see their [license page][license]

See the file COPYING for full copying permissions.

Installation
------------

Short installation instructions can be found in file INSTALL. For the
full instructions please see [here][installation].

Links
-----

0. https://www.dune-project.org/installation/installation-faq/
1. https://dune-project.org/releases/
2. https://dune-project.org/doc/buildsystem/
3. https://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.license

[installation]: https://www.dune-project.org/installation/installation-buildsrc
[license]: https://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.license
