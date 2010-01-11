// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MPI_CPPFLAGS_DEPRECATION
#define DUNE_MPI_CPPFLAGS_DEPRECATION

#warning The MPI_CPPFLAGS configure substitute is deprecated.  Please change
#warning your Makefile.am to use DUNEMPICPPFLAGS instead.  Note that it is a
#warning good idea to change any occurance of MPI_LDFLAGS into DUNEMPILIBS and
#warning DUNEMPILDFLAGS as apropriate, since it is not possible to issue a
#warning deprecation warning in that case.

#endif // DUNE_MPI_CPPFLAGS_DEPRECATION
