// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../spgrid.hh"

namespace Dune {

  ////////////////////////////////////////////////////////////////////////////

  /**
     index operator *()
   */
  template<int DIM> inline
  typename spgrid<DIM>::index& spgrid<DIM>::iterator::
  operator *() const {
    assert(l_ <= g.levels);
    assert(id_ >= 0);
    assert(id_ < g.max(l_));
#ifdef CONDITIONAL
    if (changed) {
      referenz = *this;
      changed=false;
    }
#else
    index &referenz = const_cast<index&>(this->referenz);
    referenz = *this;
#endif
    return referenz;
  }; /* end operator *() */

  ////////////////////////////////////////////////////////////////////////////

  /**
     index operator *()
   */
  template<int DIM> inline
  typename spgrid<DIM>::index* spgrid<DIM>::iterator::
  operator ->() const {
    assert(l_ <= g.levels);
    assert(id_ >= 0);
    assert(id_ < g.max(l_));
#ifdef CONDITIONAL
    if (changed) {
      referenz = *this;
      changed=false;
    }
#else
    index &referenz = const_cast<index&>(this->referenz);
    referenz = *this;
#endif

    return &referenz;
  }; /* end operator *() */

  ////////////////////////////////////////////////////////////////////////////
#ifdef QUICKHACKNOELEMENTS

  /** who is the father? */
  template<int DIM> inline
  typename spgrid<DIM>::iterator spgrid<DIM>::iterator::
  father() const {
    assert (l_ > 0);
    /*
       A: refined via KeepNumber

       |0,1,2,3,4,5|6, , , , , |
     | 0 ' 1 ' 2 | 3 '   '   |

     | , , , , ,0|1,2,3,4,5,6|
     |   '   ' 0 | 1 ' 2 ' 3 |

       coord-trafo: f(x_i) = (x_i + 1)/2

       B: refined via KeepSize

     ||0,1,2,3,4,5|6,7, , , , |
     | 0 ' 1 ' 2 | 3 '   '   |

     | , , , ,0,1|2,3,4,5,6,7|
     |   '   ' 0 | 1 ' 2 ' 3 |

       coord-trafo: f(x_i) = x_i/2
     */
    array<DIM> father_coord  = g.id_to_coord(l_, id_);
    for (int d=0; d<DIM; d++) {
      if ( g.front_overlap(l_,d) &&
           (g.front_overlap(l_,d) == g.front_overlap(l_-1,d)) )
        father_coord[d] = (father_coord[d] + 1) / 2;
      else
        father_coord[d] = father_coord[d]/ 2;
    }
    iterator father(l_-1, father_coord, g);
    return father;
  }; /* end father */

  /** who is sitting left in direction dim? */
  template<int DIM> inline
  typename spgrid<DIM>::iterator spgrid<DIM>::iterator::
  left(int dim) const {
#define OLDLEFT
#ifdef OLDLEFT
    throw string("iterator::left(dim) needs to be updated - coord changed");
    array<DIM> coord = g.id_to_coord(l_, id_);
    coord[dim]--;
    if (coord[dim] < - g.front_overlap(l_, dim)) {
      coord[dim] = - g.front_overlap(l_, dim);
      /* if we are only one Process in direction dim
         and we have periodic boundry-conditions... */
      if (!g.do_front_share(dim)
          && !g.do_end_share(dim)
          && (g.periodic_[dim]==true))
        /* we are at the end again */
        coord[dim] = g.size(l_,dim)+g.end_overlap(l_,dim)-1;
    };
    return iterator(l_, coord, g);
#else
    int subsize = 1;
    for (int d=d-1; d>=0; d++) {
      subsize = subsize * ( g.size(l_,d) +
                            g.front_overlap(l_,d) +
                            g.end_overlap(l_,d) );
    }
    int newid = id_ + move;
    if (newid < g.max(l_))
      newid = id_;
    return iterator(l_, coord, g);
#endif
  }; /* end left */

  /** who is sitting right in direction dim? */
  template<int DIM> inline
  typename spgrid<DIM>::iterator spgrid<DIM>::iterator::
  right(int dim) const {
    throw string("iterator::right(dim) nneds to be updated - coord changed");
    array<DIM> coord = g.id_to_coord(l_, id_);
    coord[dim]++;
    /* check the border */
    if (coord[dim] >= g.size(l_, dim) + g.end_overlap(l_, dim)) {
      coord[dim] = g.size(l_, dim) + g.end_overlap(l_, dim)-1;
      /* if we are only one Process in direction dim
         and we have periodic boundry-conditions... */
      if (!g.do_front_share(dim)
          && !g.do_end_share(dim)
          && (g.periodic_[dim]==true))
        /* we are at the front again */
        coord[dim] = - g.front_overlap(l_, dim);
    };
    return iterator(l_, coord, g);
  }; /* end right */

  /** tell the iterators level */
  template<int DIM> inline
  typename spgrid<DIM>::level spgrid<DIM>::iterator::
  lvl() const {
    return l_;
  }; /* end lvl */

  /** tell the iterators coordinates */
  template<int DIM> inline
  const array<DIM> &spgrid<DIM>::iterator::
  coord() const {
    return g.id_to_coord(l_, id_);
  }; /* end coord */

  /** tell the iterators coordinates */
  template<int DIM> inline
  int spgrid<DIM>::iterator::
  coord(int d) const {
    return g.id_to_coord(l_, id_)[d];
  }; /* end coord */

#endif // QUICKHACKNOELEMENTS

}; /* end namespace Dune */
