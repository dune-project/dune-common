// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_PYTHON_COMMON_HH
#define DUNE_PYTHON_COMMON_HH

namespace Dune
{

  namespace CorePy
  {

    enum class Reader { dgf, dgfString, gmsh, structured };
    enum class VTKDataType { CellData, PointData, CellVector, PointVector };
    enum class Marker { Coarsen = -1, Keep = 0, Refine = 1 };

  } // namespace CorePy

} // namespace Dune

#endif // ifndef DUNE_PYTHON_COMMON_HH
