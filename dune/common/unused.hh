// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_UNUSED_HH
#define DUNE_COMMON_UNUSED_HH

#ifndef HAS_ATTRIBUTE_UNUSED
#define DUNE_UNUSED
#else
#define DUNE_UNUSED __attribute__((unused))
#endif

#endif
