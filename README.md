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
like blas.

This package contains the basic DUNE common classes.

Dependencies
------------

dune-common depends on the following software packages

- pkg-config
- GNU C, C++ >=4.4
  these might also work:
  icc (C/C++) >= 13.0
  Clang >= 3.2

The following software is recommend but optional:

- MPI (either OpenMPI, lam, or mpich suffice)

For a full explanation of the DUNE installation process please read
the installation notes [0]. The following introduction is meant for
the impatient.

License
-------

The DUNE-library and headers are licensed under version 2 of the GNU
General Public License, with the so-called "runtime exception", as
follows:

   As a special exception, you may use the DUNE source files as part
   of a software library or application without restriction.
   Specifically, if other files instantiate templates or use macros or
   inline functions from one or more of the DUNE source files, or you
   compile one or more of the DUNE source files and link them with
   other files to produce an executable, this does not by itself cause
   the resulting executable to be covered by the GNU General Public
   License.  This exception does not however invalidate any other
   reasons why the executable file might be covered by the GNU General
   Public License.

This licence clones the one of the libstc++ library. For further
implications of this library please see their licence page [3]

See the file COPYING for full copying permissions.

Installation
------------

Short installation instructions can be found in file INSTALL. For the
full instructions please see [0].

Links
-----

0. http://www.dune-project.org/doc/installation-notes.html
1. http://www.dune-project.org/download.html
2. http://dune-project.org/doc/buildsystem/buildsystem.pdf
3. http://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.license
