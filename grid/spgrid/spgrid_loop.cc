// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../spgrid.hh"

#include <string>

namespace Dune {

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over all vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop_all(level l, stubEngine & stub) {
    switch(DIM) {
    case 3 :
    {
      array<DIM> begin;
      array<DIM> end;

      for (int d=0; d<DIM; d++) {
        begin[d] = 0;
        end[d] = size(l,d) + front_overlap(l,d) + do_end_share(d);
      }
      return loop3D(l,begin,end,end,end,stub);
    }
    default :
    {
      iterator gEnd=end(l);
      for (iterator it=begin(l); it != gEnd; ++it) {
        /* do something */
        stub.evaluate(l, it.coord(), it.id());
      }
    }
    }
  };

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over all owner vertices without border vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop_owner(level l, stubEngine & stub) {
    switch(DIM) {
    case 3 :
    {
      array<DIM> begin;
      array<DIM> end;

      for (int d=0; d<DIM; d++) {
        begin[d] = 0 + front_overlap(l,d) + ! do_front_share(d);
        end[d] = size(l,d) + front_overlap(l,d) - ! do_end_share(d);
      }
      return loop3D(l,begin,end,end,end,stub);
    }
    default :
    {
      iterator gEnd=end(l);
      for (iterator it=begin(l); it != gEnd; ++it) {
        if (! (*it).owner() ) continue;
        if ( (*it).border() ) continue;
        /* do something */
        stub.evaluate(l, it.coord(), it.id());
      }
    }
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over all not overlap vertices without border vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop_not_overlap(level l, stubEngine & stub) {
    iterator gEnd=end(l);
    for (iterator it=begin(l); it != gEnd; ++it) {
      if ( (*it).overlap() ) continue;
      if ( (*it).border() ) continue;
      /* do something */
      stub.evaluate(l, it.coord(), it.id());
    }
  };

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over all border vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop_border(level l, stubEngine & stub,
              const array<DIM, array<2> > & skip) {
    switch(DIM) {
    case 3 :
    {
      array<DIM> begin_f;
      array<DIM> end_f;
      array<DIM> begin_e;
      array<DIM> end_e;

      for (int d=0; d<DIM; d++) {
        begin_f[d] = 0 + (do_front_share(d) && skip[d][0]);
        end_f[d] = 0 + !do_front_share(d);
        begin_e[d] = size(l,d) + front_overlap(l,d)
                     + end_overlap(l,d) - !do_end_share(d);
        end_e[d] = size(l,d) + front_overlap(l,d) + end_overlap(l,d)
                   - (do_end_share(d) && skip[d][1]);
      }

      return loop3D(l,begin_f,end_f,begin_e,end_e,stub);
    }
    default :
    {
      iterator gEnd=end(l);
      for (iterator it=begin(l); it != gEnd; ++it) {
        /* skip inner vertices */
        if (! (*it).border() ) continue;
        /* do something */
        stub.evaluate(l, it.coord(), it.id());
      }
      return;
    }
    }
  };

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over all overlap vertices including border vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop_overlap(level l, stubEngine & stub) {
    switch(DIM) {
    case 3 :
    {
      array<DIM> begin_f;
      array<DIM> end_f;
      array<DIM> begin_e;
      array<DIM> end_e;

      for (int d=0; d<DIM; d++) {
        begin_f[d] = 0;
        end_f[d] = 2*front_overlap(l,d);
        begin_e[d] = size(l,d) + front_overlap(l,d) - end_overlap(l,d);
        end_e[d] = size(l,d) + front_overlap(l,d)  + end_overlap(l,d);
      }

      return loop3D(l,begin_f,end_f,begin_e,end_e,stub);
    }
    default :
    {
      iterator gEnd=end(l);
      for (iterator it=begin(l); it != gEnd; ++it) {
        if (! (*it).overlap() ) continue;
        /* do something */
        stub.evaluate(l, it.coord(), it.id());
      }
    }
    }
  };

  ////////////////////////////////////////////////////////////////////////////

  /**
     loop over an arbitary subset of vertices
     @param stub a class derived from loopstub implementing the operation
   */
  template<int DIM> template <class stubEngine> inline
  void spgrid<DIM>::
  loop3D(level l,
         array<DIM> & begin_f, array<DIM> & end_f,
         array<DIM> & begin_e, array<DIM> & end_e,
         stubEngine & stub) {
    array<DIM> coord;
    array<DIM> id=0;
    array<DIM> add=init_add(l);

    /* All Selected Vertices () */
    /* 0: front */
    id[0] = max(l-1) + begin_f[0]*add[0];
    for (coord[0]=begin_f[0]; coord[0]<end_f[0]; coord[0]++) {
      /* 1: all */
      id[1] = id[0] + begin_f[1]*add[1];
      for (coord[1]=begin_f[1]; coord[1]<end_e[1]; coord[1]++) {
        /* 2: all */
        id[2] = id[1] + begin_f[2]*add[2];
        for (coord[2]=begin_f[2]; coord[2]<end_e[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        id[1] += add[1];
      }
      id[0] += add[0];
    }
    /* 0: middle */
    for (coord[0]=end_f[0]; coord[0]<begin_e[0]; coord[0]++) {
      /* 1: front */
      id[1] = id[0] + begin_f[1]*add[1];
      for (coord[1]=begin_f[1]; coord[1]<end_f[1]; coord[1]++) {
        /* 2: all */
        id[2] = id[1] + begin_f[2]*add[2];
        for (coord[2]=begin_f[2]; coord[2]<end_e[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        id[1] += add[1];
      }
      /* 1: middle */
      for (coord[1]=end_f[1]; coord[1]<begin_e[1]; coord[1]++) {
        /* 2: front */
        id[2] = id[1] + begin_f[2]*add[2];
        for (coord[2]=begin_f[2]; coord[2]<end_f[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        /* 2: end */
        id[2] = id[1] + begin_e[2]*add[2];
        for (coord[2]=begin_e[2]; coord[2]<end_e[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        id[1] += add[1];
      }
      /* 1: end */
      for (coord[1]=begin_e[1]; coord[1]<end_e[1]; coord[1]++) {
        /* 2: all */
        id[2] = id[1] + begin_f[2]*add[2];
        for (coord[2]=begin_f[2]; coord[2]<end_e[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        id[1] += add[1];
      }
      id[0] += add[0];
    }
    /* 0: end */
    for (coord[0]=begin_e[0]; coord[0]<end_e[0]; coord[0]++) {
      /* 1: all */
      id[1] = id[0] + begin_f[1]*add[1];
      for (coord[1]=begin_f[1]; coord[1]<end_e[1]; coord[1]++) {
        /* 2: all */
        id[2] = id[1] + begin_f[2]*add[2];
        for (coord[2]=begin_f[2]; coord[2]<end_e[2]; coord[2]++) {
          /* do something */
          stub.evaluate(l, coord, id[2]);
          id[2] += add[2];
        }
        id[1] += add[1];
      }
      id[0] += add[0];
    }
  }

};
