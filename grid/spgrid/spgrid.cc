// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../spgrid.hh"

namespace Dune {

  namespace SPGridStubs {
    template <int DIM>
    class InitExchange {
      spgrid<DIM> & g;
    public:
      InitExchange(spgrid<DIM> & g_) :
        g(g_) {}
      void evaluate(level l, const array<DIM> & coord, int id) {
        spgrid<DIM>::iterator it(id,g);
        spgrid<DIM>::index i=*it;
        spgrid<DIM>::remotelist remote=i.remote();
        /* if i own the data, I'll search all processes
           to receive the data */
        if(i.owner()) {
          for (int r=0; r<remote.size; r++) {
            int & size = g.exchange_data_to[l][remote.list[r].process()].size;
            realloc<int>(g.exchange_data_to[l][remote.list[r].process()].id, size + 1);
            g.exchange_data_to[l][remote.list[r].process()].id[size] = it.id();
            size ++;
          }
        }
        /* if I share the data, find the owner-processes */
        else {
          for (int r=0; r<remote.size; r++) {
            if (remote.list[r].owner()) {
              int & size = g.exchange_data_from[l][remote.list[r].process()].size;
              realloc<int>(g.exchange_data_from[l][remote.list[r].process()].id, size + 1);
              g.exchange_data_from[l][remote.list[r].process()].id[size] = it.id();
              size ++;
              continue;
            }
          }
        }
      }
    };
  } // namespace SPGridStubs

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
      std::cout << rank_ << " expected front_overlap[process]=" << ov2
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
      std::cout << rank_ << " expected front_overlap=" << ov2
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
      std::cout << rank_ << " expected front_overlap[process]=" << ov2
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
      std::cout << rank_ << " expected front_overlap=" << ov2
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
    return (size_[d]+addon)*(1<<l)+
           (process[d]==dim_[d]-1)*(periodic_[d]==false);
  };

  template<int DIM> inline
  int spgrid<DIM>::
  size(level l, int d) const {
    int addon = size_add_[d]*(process_[d]==dim_[d]-1);
    return (size_[d]+addon)*(1<<l)+
           (process_[d]==dim_[d]-1)*(periodic_[d]==false);
  };

  /** calc globalsize of level l in direction d */
  template<int DIM> inline
  int spgrid<DIM>::
  globalsize(level l, int d) const {
    return (globalsize_[d])*(1<<l)+(periodic_[d]==false);
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
    cout << rank_ << " level = " << l << " id = " << id << endl;
    for (int d=0; d<DIM; d++)
      cout << rank_ << " " << "d=" << d
           << " front " << front_overlap(l,d) << endl
           << rank_ << " " << "d=" << d
           << " size " << size(l,d) << endl
           << rank_ << " " << "d=" << d
           << " end " << end_overlap(l,d) << endl;

    cout << rank_ << " add = " << add << endl;
    array<DIM> & c = id_to_coord_impl(l,id);
    for (int d=0; d<DIM; d++)
      cout << rank_ << " " << "d=" << d
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

  ////////////////////////////////////////////////////////////////////////////

  /**
      Datenabgleich auf Level l vorbereiten
   */
  template <int DIM>
  void spgrid<DIM>::initExchange() {
    int P;
    /* Size of Communicator */
    MPI_Comm_size(comm_, &P);
    exchange_data_from = new exchange_data*[smoothest()+1];
    exchange_data_to = new exchange_data*[smoothest()+1];
    for (level l=roughest(); l<=smoothest(); l++) {
      exchange_data_from[l] = new exchange_data[P];
      exchange_data_to[l] = new exchange_data[P];
      for (int p=0; p<P; p++) {
        exchange_data_from[l][p].size = 0;
        exchange_data_from[l][p].id = malloc<int>(1);
        exchange_data_to[l][p].size = 0;
        exchange_data_to[l][p].id = malloc<int>(1);
      }
      SPGridStubs::InitExchange<DIM> stub(*this);
      loop_overlap(l, stub);
    }
  };

  /**
      Datenabgleich auf Level l
   */
  template <int DIM>
  void spgrid<DIM>::exchange(level l, Vector< spgrid<DIM> > & ex) {
    TIME_EX -= MPI_Wtime();
    for (int d=0; d<DIM; d++) {
      for (int s=-1; s<=2; s+=2) {
        /* remote rank */
        array<DIM> remote_process = process();
        int shift;
        if ( process()[d] % 2 == 0 ) {
          shift = s;
        }
        else {
          shift = -s;
        }
        // calc neighbour coord
        remote_process[d] += shift;

        // check cart boundries
        if (shift==-1) {
          if (! do_front_share(d) ) continue;
        }
        else {
          if (! do_end_share(d) ) continue;
        }

        int remote_rank;
        MPI_Cart_rank(comm_, remote_process, &remote_rank);
        if (remote_rank < 0 )
          continue;

        /* data buffers and ids */
        int* & id_from =
          exchange_data_from[l][remote_rank].id;
        int* & id_to =
          exchange_data_to[l][remote_rank].id;
        int size_from = exchange_data_from[l][remote_rank].size;
        int size_to = exchange_data_to[l][remote_rank].size;
        double* data_from = new double[size_from];
        double* data_to = new double[size_to];

        /* collect data */
        for (int i=0; i<size_to; i++)
          data_to[i] = ex[id_to[i]];

        /* the real exchange */
        if ( process()[d] % 2 == 0 ) {
          MPI_Send( data_to, size_to, MPI_DOUBLE, remote_rank,
                    exchange_tag, comm_);
          MPI_Recv( data_from, size_from, MPI_DOUBLE, remote_rank,
                    exchange_tag, comm_, &mpi_status);
        }
        else {
          MPI_Recv( data_from, size_from, MPI_DOUBLE, remote_rank,
                    exchange_tag, comm_, &mpi_status);
          MPI_Send( data_to, size_to, MPI_DOUBLE, remote_rank,
                    exchange_tag, comm_);
        }

        /* store data */
        for (int i=0; i<size_from; i++)
          ex[id_from[i]] = data_from[i];

        /* clean up */
        delete[] data_from;
        delete[] data_to;
      }
    }
    TIME_EX += MPI_Wtime();
  }; /* end exchange() */

}
