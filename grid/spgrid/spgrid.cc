// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../spgrid.hh"

namespace Dune {

  ////////////////////////////////////////////////////////////////////////////

  /** overlap size at front */
  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d, const array<DIM> &process) const {
    int ov1=
      overlap_[l+1]*(dim_[d]>1)*
      // and not (perodic and front border process)
      (!((periodic_[d]==false)&&(process[d]==0)));
    int ov2=
      overlap_[l+1]*do_front_share(d,process);
    if (ov1 != ov2) {
      std::cout << rank << " expected front_overlap=" << ov2
                << " got " << ov1 << std::endl;
      throw std::string("ERROR");
    }
    return ov2;
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and front border process)
           (!((periodic_[d]==false)&&(process[d]==0)));
  };

  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d) const {
    int ov1=
      overlap_[l+1]*(dim_[d]>1)*
      // and not (perodic and front border process)
      (!((periodic_[d]==false)&&(process_[d]==0)));
    int ov2=
      overlap_[l+1]*do_front_share(d);
    if (ov1 != ov2) {
      std::cout << rank << " expected front_overlap=" << ov2
                << " got " << ov1 << std::endl;
      throw std::string("ERROR");
    }
    return ov2;
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and front border process)
           (!((periodic_[d]==false)&&(process_[d]==0)));
  };

  /** overlap size at end */
  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d, const array<DIM> &process) const {
    int ov1=
      overlap_[l+1]*(dim_[d]>1)*
      // and not (perodic and front border process)
      (!((periodic_[d]==false)&&(process[d]==dim_[d]-1)));
    int ov2=
      overlap_[l+1]*do_end_share(d,process);
    if (ov1 != ov2) {
      std::cout << rank << " expected front_overlap=" << ov2
                << " got " << ov1 << std::endl;
      throw std::string("ERROR");
    }
    return ov2;
    // return  overlap  if ( >1 processes in this direction
    return overlap_[+1]*(dim_[d]>1)*
           // and not (perodic and end border process)
           (!((periodic_[d]==false)&&(process[d]==dim_[d]-1)));
  };

  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d) const {
    int ov1=
      overlap_[l+1]*(dim_[d]>1)*
      // and not (perodic and front border process)
      (!((periodic_[d]==false)&&(process_[d]==dim_[d]-1)));
    int ov2=
      overlap_[l+1]*do_end_share(d);
    if (ov1 != ov2) {
      std::cout << rank << " expected front_overlap=" << ov2
                << " got " << ov1 << std::endl;
      throw std::string("ERROR");
    }
    return ov2;
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and end border process)
           (!((periodic_[d]==false)&&(process_[d]==dim_[d]-1)));
  };

#if 0
  /** overlap size at front */
  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d, const array<DIM> &process) const {
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and front border process)
           (!((periodic_[d]==false)&&(process[d]==0)));
  };

  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d) const {
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and front border process)
           (!((periodic_[d]==false)&&(process_[d]==0)));
  };

  /** overlap size at end */
  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d, const array<DIM> &process) const {
    // return  overlap  if ( >1 processes in this direction
    return overlap_[+1]*(dim_[d]>1)*
           // and not (perodic and end border process)
           (!((periodic_[d]==false)&&(process[d]==dim_[d]-1)));
  };

  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d) const {
    // return  overlap  if ( >1 processes in this direction
    return overlap_[l+1]*(dim_[d]>1)*
           // and not (perodic and end border process)
           (!((periodic_[d]==false)&&(process_[d]==dim_[d]-1)));
  };
#endif

  /** maximum index in level l */
  template<int DIM> inline
  int spgrid<DIM>::
  max(level l, const array<DIM> &process) const {
    int Max=0;
    while(l>=0) { //roughest()) {
      int max=1;
      for (int d=0; d<DIM; d++)
        max *= size(l,d,process)
               + front_overlap(l,d,process)
               + end_overlap(l,d,process);
      Max += max;
      l--;
    }
    return Max;
  };

  /** maximum global index in level l */
  template<int DIM> inline
  int spgrid<DIM>::
  globalmax(level l) const {
    int Max=0;
    while(l>=roughest()) {
      int max=1;
      for (int d=0; d<DIM; d++)
        max*=globalsize(l,d);
      Max += max;
      l--;
    }
    return Max;
  };

  /** calc size of level l in direction d (without overlap) */
  template<int DIM> inline
  int spgrid<DIM>::
  size(level l, int d, const array<DIM> &process) const {
    int addon = size_add_[d]*(process[d]==dim_[d]-1);
    return (size_[d]+addon)*(1<<l)+(process[d]==dim_[d]-1);
  };

  template<int DIM> inline
  int spgrid<DIM>::
  size(level l, int d) const {
    int addon = size_add_[d]*(process_[d]==dim_[d]-1);
    return (size_[d]+addon)*(1<<l)+(process_[d]==dim_[d]-1);
  };

  /** calc globalsize of level l in direction d */
  template<int DIM> inline
  int spgrid<DIM>::
  globalsize(level l, int d) const {
    return (globalsize_[d])*(1<<l)+1;
  };

  ////////////////////////////////////////////////////////////////////////////

  /**
     void id_to_coord(&array<DIM>)

     Set my local coord
   */

  template<int DIM> inline
  const array<DIM>&
  spgrid<DIM>::id_to_coord(level l, int id, const array<DIM> &process) const
  {
    init_add(l,process);
    return id_to_coord_impl(l,id);
  }; /* end id_to_coord */

  template<int DIM> inline
  const array<DIM>& spgrid<DIM>::
  id_to_coord(level l, int id) const {
    init_add(l);
#if 0
    cout << rank << " level = " << l << " id = " << id << endl;
    for (int d=0; d<DIM; d++)
      cout << rank << " " << "d=" << d
           << " front " << front_overlap(l,d) << endl
           << rank << " " << "d=" << d
           << " size " << size(l,d) << endl
           << rank << " " << "d=" << d
           << " end " << end_overlap(l,d) << endl;

    cout << rank << " add = " << add << endl;
    array<DIM> & c = id_to_coord_impl(l,id);
    for (int d=0; d<DIM; d++)
      cout << rank << " " << "d=" << d
           << " coord = " << c[d] << endl;
    return c;
#else
    return id_to_coord_impl(l,id);
#endif
  } /* end id_to_coord */

  template<int DIM> inline
  const array<DIM>& spgrid<DIM>::
  id_to_coord_impl(level l, int id) const
  {
    array<DIM> &coord_buffer = const_cast<array<DIM>&>(this->coord_buffer);

    // remove offset
    int pos=id - max(l-1);
    int subpos = 0;

    coord_buffer[0] = pos / add_buffer[0];
    for (int d=0; d<DIM; d++) {
      coord_buffer[d] = ( pos - subpos ) / add_buffer[d];
      subpos += coord_buffer[d]*add_buffer[d];
    };
    return coord_buffer;
  }; /* end id_to_coord_impl */

  /**
     int coord_to_id(array<DIM>)

     Set my local id
   */
  template<int DIM> inline
  int spgrid<DIM>::
  coord_to_id(level l, const array<DIM> &coord,
              const array<DIM> &process) const
  {
    init_add(l, process);
    return coord_to_id_impl(l, coord);
  }; /* end coord_to_id */

  template<int DIM> inline
  int spgrid<DIM>::
  coord_to_id(level l, const array<DIM> &coord) const
  {
    init_add(l);
    return coord_to_id_impl(l, coord);
  }

  template<int DIM> inline
  int spgrid<DIM>::
  coord_to_id_impl(level l, const array<DIM> &coord) const
  {
    int id = max(l-1);
    for (int d=0; d<DIM; d++)
      id += coord[d]*add_buffer[d];
    assert(id >= 0);

    return id;
  }; /* end coord_to_id */

  ////////////////////////////////////////////////////////////////////////////

  /** initialize the class */
  template<int DIM>
  void spgrid<DIM>::init()
  {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_);

    /*
     * Set up the topology of the comm-mesh
     */
    int P;
    MPI_Comm_size(MPI_COMM_WORLD, &P); // Number of Processors
    dim_=0;
    MPI_Dims_create(P, DIM, dim_);
    bool reorder = true;
    array<DIM> periodic_dummy;
    for (int d=0; d<DIM; d++) {
      periodic_dummy[d] = periodic_[d];
    }
    MPI_Cart_create(MPI_COMM_WORLD, DIM, dim_,
                    periodic_dummy, reorder, &comm_);

    /*
     * Find my coordinates in the comm-mesh
     */
    MPI_Cart_coords(comm_, rank_, DIM, process_);

    /*
     * Find my coordinates in the data-mesh
     */
    for (int d=0; d<DIM; d++) {
      size_[d] = globalsize_[d]/dim_[d];
      size_add_[d] = globalsize_[d]%dim_[d];
    };

    /*
     * Initialize max_
     */
    max_[0]=0;
    for (level l=0; l<=levels; l++)
      max_[l+1] = max(l, process_);
    for (level l=levels+1; l<maxlevels; l++)
      overlap_[l]=0;
  };

  template<int DIM>
  void spgrid<DIM>::refine(refStrategy r)
  {
    if ( levels >= maxlevels ) {
      throw "maximum Level count reached.. sorry";
    }
    overlap_[levels+1]=overlap_[levels]*r;
    levels++;
    max_[levels] = max(levels-1, process_);
    max_[levels+1] = max(levels, process_);
  }

  /** who is the father? */
  template<int DIM> inline
  int spgrid<DIM>::father_id(level l, const array<DIM> & coord) const {
    assert (l > 0);
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
    array<DIM> father_coord;
    for (int d=0; d<DIM; d++) {
      if ( has_coord_shift(l,d) ) {
        father_coord[d] = (coord[d] + 1) / 2;
        assert((coord[d] + 1) % 2 == 0);
      }
      else {
        father_coord[d] = coord[d]/ 2;
        assert((coord[d]) % 2 == 0);
      }
    }
    return coord_to_id(l-1, father_coord);
  }; /* end father */

  template<int DIM> inline
  int spgrid<DIM>::has_coord_shift(level l, int d) const {
    if ( front_overlap(l,d) &&
         (front_overlap(l,d) == front_overlap(l-1,d)) )
      return 1;
    return 0;
  }
}
