// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../spgrid.hh"

#include "math_templ.hh"

namespace Dune {

  template<int DIM> inline
  int spgrid<DIM>::index::
  globalid() const {
    /*
       #warning only for one processor
       return id_;

       throw string ("globalid is obsolete");
     */
    /*
     * Set my local coord
     */
    array<DIM> coord = g.id_to_coord(l_,id_);
    assert (g.coord_to_id(l_,coord)==id_);
    array<DIM> g_coord;
    /*
     * my position in the global grid
     */
    for (int d=0; d<DIM; d++) {
      g_coord[d] = coord[d]
                   - g.front_overlap(l_,d)
                   + g.process_[d]*g.size(l_,d, array<DIM>(0));
      // wrap around
      if (g_coord[d]<0)
        g_coord[d]+=g.globalsize(l_,d);
      if (g_coord[d]>=g.globalsize(l_,d))
        g_coord[d]-=g.globalsize(l_,d);
    }
    /*
     * my global id in this level
     */
    int g_id=g_coord[0];
    for (int d=1; d<DIM; d++)
      g_id=g_coord[d]+g.globalsize(l_,d)*(g_id);
#ifndef NDEBUG
    if (g_id<0) {
      std::cout << "ERROR on level " << l_ << " End=" << g.max(l_) << std::endl
                << coord << g_coord << std::endl
                << id_ << " " << g_id << std::endl << "F_OVL:";
      for (int d=0; d<DIM; d++) std::cout << g.front_overlap(l_,d) << " ";
      std::cout << "\n Grid->F_OVL:";
      for (int d=0; d<DIM; d++) std::cout << g.front_overlap(l_,d) << " ";
      std::cout << "\n Grid->ovl:"
                << g.overlap_[l_] << " ";
      std::cout << std::endl;
    }
    assert (g_id>=0);
#endif
    /*
     * add the level-offset
     */
    g_id+=g.globalmax(l_-1);
    return g_id;
  };

  template<int DIM> inline
  bool spgrid<DIM>::index::
  owner() const {
    array<DIM> coord = g.id_to_coord(l_,id_);
    assert (g.coord_to_id(l_,coord)==id_);
    for (int d=0; d<DIM; d++) {
      // do we share this dimension? and am I an overlap-index at the front?
      if (coord[d] < g.front_overlap(l_,d))
        return false;
      // am I an overlap-index at the end?
      if (coord[d] >= g.size(l_,d) + g.front_overlap(l_,d))
        return false;
    };
    return true;
  };

  template<int DIM> inline
  bool spgrid<DIM>::index::
  border() const {
    array<DIM> coord = g.id_to_coord(l_,id_);
    for (int d=0; d<DIM; d++) {
      if (coord[d]==0 && !g.do_front_share(d) ) return true;
      if (coord[d]==
          g.front_overlap(l_,d) + g.end_overlap(l_,d) + g.size(l_,d) -1
          && !g.do_end_share(d) ) return true;
    }
    return false;
  };

  template<int DIM> inline
  bool spgrid<DIM>::index::
  overlap() const {
    array<DIM> coord = g.id_to_coord(l_,id_);
    assert (g.coord_to_id(l_,coord)==id_);
    for (int d=0; d<DIM; d++) {
      // do we share this dimension? and am I an overlap-index at the front?
      if (g.do_front_share(d) && (coord[d] < 2*g.front_overlap(l_,d)))
        return true;
      // am I an overlap-index at the end?
      if (g.do_end_share(d) &&
          (coord[d] >=
           g.size(l_,d) + g.front_overlap(l_,d) - g.end_overlap(l_,d)))
        return true;
    };
    return false;
  };

  /** calculate the remote indices */
  template<int DIM> inline
  typename spgrid<DIM>::remotelist spgrid<DIM>::index::
  remote() const {
    remotelist remote(pow<DIM>(3)); // 3^DIM
    remote.size=0;

    if (!overlap()) return remote;

    /*
     * Set my local coord
     */
    array<DIM> coord = g.id_to_coord(l_,id_);
    assert (g.coord_to_id(l_,coord)==id_);

    /*
     * Owner Process
     */
    array<DIM> remote_displace = 0;
    array<DIM> owner_displace = 0;
    array<DIM> owner_process;
    int owner_rank;

    for (int d=0; d<DIM; d++) {
      // do we share this dimension?
      if (g.do_front_share(d)) {
        // am I an overlap-index at the front?
        if (coord[d] < g.front_overlap(l_,d))
          owner_displace[d] = - 1;
        // do I have overlap-index at the front?
        if (coord[d] < 2*g.front_overlap(l_,d))
          remote_displace[d] = -1;
      };
      if (g.do_end_share(d)) {
        // am I an overlap-index at the end?
        if (coord[d] >= g.size(l_,d) + g.front_overlap(l_,d))
          owner_displace[d] = + 1;
        // do I have overlap-index at the end?
        if (coord[d] >=
            g.size(l_,d) + g.front_overlap(l_,d) - g.end_overlap(l_,d))
          remote_displace[d] = +1;
      };
    };

    // calc owner rank
    {
      owner_process = g.process_;
      owner_process += owner_displace;
      MPI_Cart_rank(g.comm_, owner_process, &owner_rank);
    }

    // all overlaps
    array< 1<<(DIM*2), array<DIM> > remote_displacements;
    int rd_count=0;

    /*
     * Neighbour Processes
     */
    for (int d=0; d<DIM; d++) {
      // do nothing if we dont overlap in this dim
      if ( remote_displace[d] == 0 ) continue;
      // add displacement relative to me
      array<DIM> base_displace = 0;
      base_displace[d] = remote_displace[d];
      remote_displacements[rd_count++] = base_displace;
      // add displacement relative to all existing displacements
      array<DIM> tmp_displace = 0;
      int rd_max = rd_count;
      for (int rd_index = 0; rd_index < rd_max; rd_index++) {
        tmp_displace = remote_displacements[rd_index];
        tmp_displace[d] = remote_displace[d];
        if (tmp_displace != base_displace)
          remote_displacements[rd_count++] = tmp_displace;
      };
    };

    /*
     * Remote Index
     */
    {
      array<DIM> remote_process;
      array<3,bool> remote_flags(false);
      array<DIM> remote_coord;
      int remote_rank;
      int remote_id;
      for (int rd_index = 0; rd_index < rd_count; rd_index++) {
        // remote_displace -> remote_process
        remote_process = g.process_;
        remote_process += remote_displacements[rd_index];
        MPI_Cart_rank(g.comm_, remote_process, &remote_rank);
        // remote_process -> remote_flags
        remote_flags[f_owner] = (remote_rank == owner_rank);
        // Every Element we share with a remote process
        // must be an overlap element
        remote_flags[f_overlap] = true;
        // remote_process -> remote_coord
        for (int d=0; d<DIM; d++) {
          remote_coord[d] =
            coord[d]
            // add displacement
            // front neighbour
            // + local size + local front_overlap - remote front_overlap
            + (remote_displacements[rd_index][d] == -1)*
            ( + g.size(l_,d,remote_process)
              - g.front_overlap(l_,d)
              + g.front_overlap(l_,d,remote_process) )
            // end neighbour
            // - remote size + local front_overlap - remote front_overlap
            + (remote_displacements[rd_index][d] == +1)*
            ( - g.size(l_,d)
              - g.front_overlap(l_,d)
              + g.front_overlap(l_,d,remote_process) );
        }
        // remote_coord -> remote_id
        remote_id = g.coord_to_id(l_,remote_coord,remote_process);
        // create remote_index
        remote.list[remote.size].id_ = remote_id;
        remote.list[remote.size].processor_ = remote_rank;
        remote.list[remote.size].flags_ = remote_flags;
        remote.list[remote.size].coord_ = remote_coord;
        remote.size++ ;
      };
    };

    return remote;
  }; /* end operator *() */

}; /* end namespace Dune */
