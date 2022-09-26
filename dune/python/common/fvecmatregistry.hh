// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_FVECMATREG_HH
#define DUNE_PYTHON_COMMON_FVECMATREG_HH

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/common/fvector.hh>
#include <dune/python/common/fmatrix.hh>
namespace Dune
{
  namespace Python
  {
    template <class Type>
    struct registerFieldVecMat;

    template <class K, int size>
    struct registerFieldVecMat<Dune::FieldVector<K,size>>
    {
      static void apply()
      {
        pybind11::module scope = pybind11::module::import("dune.common");
        registerFieldVector<K,size>(scope);
      }
    };
    template< class K, int row, int col >
    struct registerFieldVecMat<Dune::FieldMatrix<K,row,col>>
    {
      static void apply()
      {
        pybind11::module scope = pybind11::module::import("dune.common");
        registerFieldMatrix<K,row,col>(scope);
        registerFieldVector<K,col>(scope);
        registerFieldVector<K,row>(scope);
      }
    };
  }
}
#endif // DUNE_PYTHON_COMMON_FVECMATREG_HH
