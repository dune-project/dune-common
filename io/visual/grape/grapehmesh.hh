// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#undef __GRAPE_HMESH_C__
#undef __GRAPE_HMESH_H__
#undef __GRAPE_ELDESC_H__

#include "ghmesh.hh"
#include "geldesc.hh"
#include "ghmesh.cc"

#if GRAPE_DIM == 3
#define G_CPP
#undef __GRAPE_PARTITIONDISPLAY_HH_
#include "partitiondisplay.cc"
#undef G_CPP
#endif

#undef GRAPE_DIM
#undef GRAPE_DIMWORLD

#undef HMesh
#undef GenMesh
#undef GrapeMesh
