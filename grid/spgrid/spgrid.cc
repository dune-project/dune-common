// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "dune/common/misc.hh"
#include "../spgrid.hh"

namespace Dune {

  namespace SPGridStubs {
    template <int DIM>
    class Vec2Buf {
      Vector< spgrid<DIM> > & in;
      double * out;
      int size;
      int count;
    public:
      Vec2Buf (Vector< spgrid<DIM> > & _in, double * _out, int _size) :
        in(_in), out(_out), size(_size), count(0) {}
      void evaluate(level l, const array<DIM> & coord, int id) {
        assert(count < size);
        out[count] = in[id];
        count ++;
      }
    };

    template <int DIM>
    class Buf2Vec {
      double * in;
      int size;
      Vector< spgrid<DIM> > & out;
      int count;
    public:
      Buf2Vec (double * _in, int _size, Vector< spgrid<DIM> > & _out) :
        in(_in), size(_size), out(_out), count(0) {}
      void evaluate(level l, const array<DIM> & coord, int id) {
        assert(count < size);
        out[id] = in[count];
        count ++;
      }
    };
  } // namespace SPGridStubs

  ////////////////////////////////////////////////////////////////////////////

  /** overlap size at front */
  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d, const array<DIM> &process) const {
    int ov=
      overlap_[l+1]*do_front_share(d,process);
    return ov;
  };

  template<int DIM> inline
  int spgrid<DIM>::
  front_overlap(level l, int d) const {
    int ov=
      overlap_[l+1]*do_front_share(d);
    return ov;
  };

  /** overlap size at end */
  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d, const array<DIM> &process) const {
    int ov=
      overlap_[l+1]*do_end_share(d,process);
    return ov;
  };

  template<int DIM> inline
  int spgrid<DIM>::
  end_overlap(level l, int d) const {
    int ov=
      overlap_[l+1]*do_end_share(d);
    return ov;
  };

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
    return id_to_coord_impl(l,id);
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
    bool reorder = false;
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
       f[d] = ( x[d] - f_overlap(l,d) ) / 2 + f_overlap(l-1,d)

       =>       f[d]  =  ( x[d] + shift[d] ) / 2
           shift[d]  =  2 * f_overlap(l-1,d) - f_overlap(l,d)
     */
    array<DIM> father_coord;
    for (int d=0; d<DIM; d++) {
      father_coord[d] = (coord[d] + coord_shift(l,d)) / 2;
      assert((coord[d] + coord_shift(l,d)) % 2 == 0);
    }
    return coord_to_id(l-1, father_coord);
  }; /* end father */

  template<int DIM> inline
  int spgrid<DIM>::coord_shift(level l, int d) const {
    return 2 * front_overlap(l-1,d) - front_overlap(l,d);
  }

  ////////////////////////////////////////////////////////////////////////////

  template <int DIM>
  void spgrid<DIM>::exchange(level l, Vector< spgrid<DIM> > & ex) {
    //    TIME_EX -= MPI_Wtime();
    for (int d=0; d<DIM; d++) {
      /* every Process who's d'th coord is even */
      if(process(d)%2 == 0) {
        Send(d, Dune::left, l, ex);
        Recv(d, Dune::left, l, ex);
        Send(d, Dune::right, l, ex);
        Recv(d, Dune::right, l, ex);
      }
      /* every Process who's d'th coord is odd */
      else {
        Recv(d, Dune::right, l, ex);
        Send(d, Dune::right, l, ex);
        Recv(d, Dune::left, l, ex);
        Send(d, Dune::left, l, ex);
      }
    }
    //    TIME_EX += MPI_Wtime();
  }; /* end exchange() */

  /**
     Send Overlap on face [dir,s] to neigbour [dir,s]
   */
  template <int DIM>
  void spgrid<DIM>::Send(int dir, Dune::side s,
                         level l, Vector< spgrid<DIM> > & ex) {
    /* Calc the slab which we need to send */
    int sz = 1;
    array<DIM> begin;
    array<DIM> end;
    for (int d=0; d<DIM; d++) {
      if (d!=dir) {
        begin[d] = 0;
        end[d] = front_overlap(l, d) + size(l, d) + end_overlap(l, d);
      }
      else {
        switch (s) {
        case left :
          begin[d] = front_overlap(l, d);
          end[d] = 2*front_overlap(l, d);
          break;
        case right :
          begin[d] = front_overlap(l, d) + size(l, d) - end_overlap(l, d);
          end[d] = front_overlap(l, d) + size(l, d);
          break;
        default :
          throw std::string("Invalid Side");
        }
      }
      sz *= end[d] - begin[d];
    }
    /* Do we have anything to send?! */
    if (sz > 0) {
      /* Collect the data */
      double * buffer = new double[sz];
      SPGridStubs::Vec2Buf<DIM> stub(ex, buffer, sz);
      loop3D(l, begin, end, end, end, stub);
      /* Calc the remote rank */
      array<DIM> remote_process = process();
      remote_process[dir] += s;
      int remote_rank;
      MPI_Cart_rank(comm_, remote_process, &remote_rank);
      if (remote_rank < 0 ) throw std::string("Remote Rank wrong");
      /* Send the data */
      MPI_Send( buffer, sz, MPI_DOUBLE, remote_rank,
                exchange_tag, comm_);
      /* Clean up */
      delete[] buffer;
    }
  }; /* end Send() */

  /**
     Recv Overlap on face [dir,s] from neigbour [dir,s]
   */
  template <int DIM>
  void spgrid<DIM>::Recv(int dir, Dune::side s,
                         level l, Vector< spgrid<DIM> > & ex) {
    /* Calc the slab which we need to send */
    int sz = 1;
    array<DIM> begin;
    array<DIM> end;
    for (int d=0; d<DIM; d++) {
      if (d!=dir) {
        begin[d] = 0;
        end[d] = front_overlap(l, d) + size(l, d) + end_overlap(l, d);
      }
      else {
        switch (s) {
        case left :
          begin[d] = 0;
          end[d] = front_overlap(l, d);
          break;
        case right :
          begin[d] = front_overlap(l, d) + size(l, d);
          end[d] = front_overlap(l, d) + size(l, d) + end_overlap(l, d);
          break;
        default :
          throw std::string("Invalid Side");
        }
      }
      sz *= end[d] - begin[d];
    }
    /* Do we have anything to send?! */
    if (sz > 0) {
      double * buffer = new double[sz];
      /* Calc the remote rank */
      array<DIM> remote_process = process();
      remote_process[dir] += s;
      int remote_rank;
      MPI_Cart_rank(comm_, remote_process, &remote_rank);
      if (remote_rank < 0 ) throw std::string("Remote Rank wrong");
      /* Recv the data */
      MPI_Recv( buffer, sz, MPI_DOUBLE, remote_rank,
                exchange_tag, comm_, &mpi_status);
      /* Store the data */
      SPGridStubs::Buf2Vec<DIM> stub(buffer, sz, ex);
      loop3D(l, begin, end, end, end, stub);
      /* Clean up */
      delete[] buffer;
    }
  } /* end Recv() */
}
