// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "alu3dmappings.hh"
#include "../alu3dgrid.hh"

namespace Dune {
  const int ALU3dImplTraits<hexa>::
  alu2duneFace_[6] =  {4, 5, 1, 3, 0, 2};
  //{4, 5, 2, 1, 3, 0};

  const int ALU3dImplTraits<hexa>::
  dune2aluFace_[6] =  {4, 2, 5, 3, 0, 1};
  //{5, 3, 2, 4, 0, 1};

  const double TrilinearMapping :: _epsilon = 1.0e-8 ;

  //- Bilinear surface mapping
  /*
     BilinearSurfaceMapping ::
     BilinearSurfaceMapping (const coord3_t& x0, const coord3_t& x1,
                          const coord3_t& x2, const coord3_t& x3)
     : _p0 (x0), _p1 (x1), _p2 (x2), _p3 (x3) {
     _b [0][0] = _p0 [0] ;
     _b [0][1] = _p0 [1] ;
     _b [0][2] = _p0 [2] ;
     _b [1][0] = _p1 [0] - _p0 [0] ;
     _b [1][1] = _p1 [1] - _p0 [1] ;
     _b [1][2] = _p1 [2] - _p0 [2] ;
     _b [2][0] = _p2 [0] - _p0 [0] ;
     _b [2][1] = _p2 [1] - _p0 [1] ;
     _b [2][2] = _p2 [2] - _p0 [2] ;
     _b [3][0] = _p3 [0] - _p2 [0] - _b [1][0] ;
     _b [3][1] = _p3 [1] - _p2 [1] - _b [1][1] ;
     _b [3][2] = _p3 [2] - _p2 [2] - _b [1][2] ;
     _n [0][0] = _b [1][1] * _b [2][2] - _b [1][2] * _b [2][1] ;
     _n [0][1] = _b [1][2] * _b [2][0] - _b [1][0] * _b [2][2] ;
     _n [0][2] = _b [1][0] * _b [2][1] - _b [1][1] * _b [2][0] ;
     _n [1][0] = _b [1][1] * _b [3][2] - _b [1][2] * _b [3][1] ;
     _n [1][1] = _b [1][2] * _b [3][0] - _b [1][0] * _b [3][2] ;
     _n [1][2] = _b [1][0] * _b [3][1] - _b [1][1] * _b [3][0] ;
     _n [2][0] = _b [3][1] * _b [2][2] - _b [3][2] * _b [2][1] ;
     _n [2][1] = _b [3][2] * _b [2][0] - _b [3][0] * _b [2][2] ;
     _n [2][2] = _b [3][0] * _b [2][1] - _b [3][1] * _b [2][0] ;
     return ;
     }

     BilinearSurfaceMapping ::
     BilinearSurfaceMapping (const BilinearSurfaceMapping & m)
     : _p0(m._p0), _p1(m._p1), _p2(m._p2), _p3(m._p3) {
     {for (int i = 0 ; i < 4 ; i ++)
      for (int j = 0 ; j < 3 ; j ++ )
        _b [i][j] = m._b [i][j] ;
     }
     {for (int i = 0 ; i < 3 ; i ++)
      for (int j = 0 ; j < 3 ; j ++ )
        _n [i][j] = m._n [i][j] ;
     }
     return ;
     }

     void BilinearSurfaceMapping ::
     map2world (const coord2_t& map, coord3_t& wld) const {
     double x = map [0];
     double y = map [1];
     double xy = x * y ;
     wld[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0] ;
     wld[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1] ;
     wld[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2] ;
     return ;
     }

     void BilinearSurfaceMapping ::
     map2world (double x, double y, coord3_t& w) const {
     coord2_t p ;
     p [0] = x ;
     p [1] = y ;
     map2world (p,w) ;
     return ;
     }

     void BilinearSurfaceMapping ::
     normal (const coord2_t& map, coord3_t& normal) const {
     double x = map [0];
     double y = map [1];
     normal [0] = -( _n [0][0] + _n [1][0] * x + _n [2][0] * y) ;
     normal [1] = -( _n [0][1] + _n [1][1] * x + _n [2][1] * y) ;
     normal [2] = -( _n [0][2] + _n [1][2] * x + _n [2][2] * y) ;
     return ;
     }
   */


} // end namespace Dune
