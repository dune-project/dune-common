// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_VERSION_HH
#define DUNE_VERSION_HH

/** \file
 * \brief Various macros to work with %Dune module version numbers
 */

/** \brief Constructs the preprocessor name used in config.h to hold version numbers
 * \param module The name of the Dune module
 * \param type The version number type, one of MAJOR, MINOR, or REVISION
 */
#define DUNE_VERSION_JOIN(module,type) module ## _VERSION_ ## type

/** \brief True if 'module' has the version major.minor */
#define DUNE_VERSION_EQUAL(module,major,minor) \
  ((DUNE_VERSION_JOIN(module,MAJOR) == major) && \
   (DUNE_VERSION_JOIN(module,MINOR) == minor))

/** \brief True if 'module' has the version major.minor.revision */
#define DUNE_VERSION_EQUAL_REV(module,major,minor,revision) \
  ( DUNE_VERSION_EQUAL(module,major,minor) && \
    (DUNE_VERSION_JOIN(module,REVISION) == revision))

/** \brief True if 'module' has the version major.minor or newer */
#define DUNE_VERSION_NEWER(module,major,minor) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) >= minor)))

/** \brief True if 'module' has the version major.minor.revision or newer */
#define DUNE_VERSION_NEWER_REV(module,major,minor,revision) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) > minor)) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) == minor) \
       && (DUNE_VERSION_JOIN(module,REVISION) >= revision)))

/** \brief Compute a unique uint id from the major, minor, and revision numbers */
#define DUNE_VERSION_ID(major,minor,revision) \
  ((unsigned int)((major << 24) + (minor << 16) + revision))

/** \brief Compute a unique uint id for the given module */
#define DUNE_MODULE_VERSION_ID(module) \
  DUNE_VERSION_ID( DUNE_VERSION_JOIN(module,MAJOR), DUNE_VERSION_JOIN(module,MINOR), DUNE_VERSION_JOIN(module,REVISION) )

#endif
