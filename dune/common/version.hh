// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_VERSION_HH
#define DUNE_VERSION_HH

/** \file
 * \brief Various macros to work with %Dune module version numbers
 */

/** \brief Constructs the preprocessor name used in config.h to hold version numbers
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 *
 * \param module The name of the Dune module
 * \param type The version number type, one of MAJOR, MINOR, or REVISION
 */
#define DUNE_VERSION_JOIN(module,type) module ## _VERSION_ ## type

/**
 * \brief True if 'module' has the version major.minor
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_EQUAL(module,major,minor) \
  ((DUNE_VERSION_JOIN(module,MAJOR) == major) && \
   (DUNE_VERSION_JOIN(module,MINOR) == minor))

/**
 * \brief True if 'module' has the version major.minor.revision
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_EQUAL_REV(module,major,minor,revision) \
  ( DUNE_VERSION_EQUAL(module,major,minor) && \
    (DUNE_VERSION_JOIN(module,REVISION) == revision))

/**
 * \brief True if 'module' has the version major.minor or greater
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_GTE(module,major,minor) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) >= minor)))

/**
 * \brief True if 'module' has a version less than major.minor
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_LT(module,major,minor) \
  ! DUNE_VERSION_GTE(module,major,minor)

/**
 * \brief True if 'module' has the version major.minor or newer
 * \note Deprecated, use DUNE_VERSION_GTE instead.
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_NEWER(module,major,minor) \
  DUNE_VERSION_GTE(module,major,minor)

/**
 * \brief True if 'module' has a version greater than major.minor
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_GT(module,major,minor) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) > minor)))

/**
 * \brief True if 'module' has a version less than or equal to major.minor
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_LTE(module,major,minor) \
  ! DUNE_VERSION_GT(module,major,minor)

/**
 * \brief True if 'module' has the version major.minor.revision or greater
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_GTE_REV(module,major,minor,revision) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) > minor)) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) == minor) \
       && (DUNE_VERSION_JOIN(module,REVISION) >= revision)))

/**
 * \brief True if 'module' has a version lower than major.minor.revision
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_LT_REV(module,major,minor,revision) \
  ! DUNE_VERSION_GTE_REV(module,major,minor,revision)

/**
 * \brief True if 'module' has the version major.minor.revision or newer
 * \note Deprecated, use DUNE_VERSION_GTE_REV instead.
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_NEWER_REV(module,major,minor,revision) \
  DUNE_VERSION_GTE_REV(module,major,minor,revision)

/**
 * \brief True if 'module' has a greater version than major.minor.revision
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_GT_REV(module,major,minor,revision) \
  ((DUNE_VERSION_JOIN(module,MAJOR) > major) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) > minor)) \
   || ((DUNE_VERSION_JOIN(module,MAJOR) == major) && (DUNE_VERSION_JOIN(module,MINOR) == minor) \
       && (DUNE_VERSION_JOIN(module,REVISION) > revision)))

/**
 * \brief True if 'module' has a version lower or equal to major.minor.revision
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_LTE_REV(module,major,minor,revision) \
  ! DUNE_VERSION_GT_REV(module,major,minor,revision)

/**
 * \brief Compute a unique uint id from the major, minor, and revision numbers
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_VERSION_ID(major,minor,revision) \
  ((unsigned int)((major << 24) + (minor << 16) + revision))

/**
 * \brief Compute a unique uint id for the given module
 *
 * For the DUNE core modules you need to use the following module names:
 *  - DUNE_COMMON for dune-common
 *  - DUNE_GRID for dune-grid
 *  - DUNE_GEOMETRY for dune-geometry
 *  - DUNE_ISTL for dune-istl
 *  - DUNE_LOCALFUNCTIONS for dune-localfunctions
 *
 * For external DUNE modules, you should capitalize the name and
 * replace '-' by underscores. For example for the module foo-bar you
 * need to use FOO_BAR as module name in the context of this macro.
 */
#define DUNE_MODULE_VERSION_ID(module) \
  DUNE_VERSION_ID( DUNE_VERSION_JOIN(module,MAJOR), DUNE_VERSION_JOIN(module,MINOR), DUNE_VERSION_JOIN(module,REVISION) )

#endif
