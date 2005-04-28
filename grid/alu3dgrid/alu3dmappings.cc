// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "alu3dmappings.hh"

namespace Dune {
  const double TrilinearMapping :: _epsilon = 1.0e-8 ;

  void TrilinearMapping :: world2map (const coord_t& wld , coord_t& map ) {
    //  Newton - Iteration zum Invertieren der Abbildung f.
    double err = 10.0 * _epsilon ;
#ifndef NDEBUG
    int count = 0 ;
#endif
    map [0] = map [1] = map [2] = .0 ;
    do {
      coord_t upd ;
      map2world (map, upd) ;
      inverse (map) ;
      double u0 = upd [0] - wld [0] ;
      double u1 = upd [1] - wld [1] ;
      double u2 = upd [2] - wld [2] ;
      double c0 = Dfi [0][0] * u0 + Dfi [0][1] * u1 + Dfi [0][2] * u2 ;
      double c1 = Dfi [1][0] * u0 + Dfi [1][1] * u1 + Dfi [1][2] * u2 ;
      double c2 = Dfi [2][0] * u0 + Dfi [2][1] * u1 + Dfi [2][2] * u2 ;
      map [0] -= c0 ;
      map [1] -= c1 ;
      map [2] -= c2 ;
      err = fabs (c0) + fabs (c1) + fabs (c2) ;
      assert (count ++ < 1000) ;
    } while (err > _epsilon) ;
    return ;
  }



} // end namespace Dune
