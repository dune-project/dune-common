// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_SPGRID_HH
#define DUNE_SPGRID_HH

/**
 * Structured parallel Grid
 *   spgrid<DIM>
 *   - template parameter for Dimension
 *   - choice of periodic boundry
 *   - represents a vertex centered grid
 *   - index does _not_ represent one element, but one degree of freedom
 */

#include <mpi.h>

#include <string>

#include "spgrid/array.hh"

typedef int ID_t;

namespace Dune {

  // consts for flag-index
  const int f_owner=0;
  const int f_border=1;
  const int f_overlap=2;

  ////////////////////////////////////////////////////////////////////////////
  /// The Structured parallel Grid

  template<int DIM>
  class spgrid
  {
  public:
    // forward declarations
    typedef int level;
    class iterator;
    class index;
    class remoteindex;
    class remotelist;
    /// fixed number of max levels
    enum {
      maxlevels=64
    };
    /// define the way to refine our grid
    enum refStrategy {
      keepNumber=1,
      keepSize=2
    };
  private:
    array<DIM> globalsize_;    ///< globale size of level 0
    array<DIM> size_;          ///< size of level 0
    array<DIM> size_add_;      ///< additional size for right_end processors
    array<DIM> dim_;           ///< global arrangement of the processors
    array<DIM> process_;       ///< locatation of this processor
    /// where do we have periodic boundry conditions?
    array<DIM,bool> periodic_;
    int rank_;                 ///< my rank
    MPI_Comm comm_;            ///< my MPI Communicator
    int overlap_[maxlevels+1]; ///< size of Overlap for each level
    int levels;
    int max_[maxlevels+1];

    /* Buffers */
    array<DIM> coord_buffer;
    array<DIM> add_buffer;
  private:
    friend class iterator;
    friend class index;
    int has_coord_shift(level, int d) const;
    //! local maximum index on level l (lookup)
    int max(level l) const
    { assert(l>=-1 && l<=maxlevels); return max_[l+1]; };
    //! remote maximum index on level l (dyn. calculated)
    int max(level l, const array<DIM> &process) const;
    //! global maximum index on level l
    int globalmax(level l) const;
    //! global grid size in direction d on level l
    int globalsize(level l, int d) const;
    //! get id from coord
    int coord_to_id(level, const array<DIM>&, const array<DIM>&) const;
    int coord_to_id(level, const array<DIM>&) const;
    int coord_to_id_impl(level, const array<DIM>&) const;
    //! get local coord from local id
    const array<DIM> &id_to_coord(level l, int id) const;
    const array<DIM> &id_to_coord(level l, int id, const array<DIM> &) const;
    const array<DIM> &id_to_coord_impl(level l, int id) const;
    //! initialize the grid
    void init();
    //! Allpurpose 3D loop
    template <class stubEngine>
    void loop3D(level l,
                array<DIM> & begin_f, array<DIM> & end_f,
                array<DIM> & begin_e, array<DIM> & end_e,
                stubEngine & );
  public:
    // Constructors
    spgrid(array<DIM> size, int o, bool periodic) :
      globalsize_(size), periodic_(periodic), levels(0)
    {
      for (int d=0; d<DIM; d++)
        if (size[d]<=0) throw std::string("Size Error");
      overlap_[0]=0;
      levels++;
      overlap_[levels]=o; overlap_[levels+1]=0;
      init();
    };
    spgrid(array<DIM> size, int o, array<DIM,bool> periodic) :
      globalsize_(size), periodic_(periodic), levels(0)
    {
      for (int d=0; d<DIM; d++)
        if (size[d]<=0) throw std::string("Size Error");
      overlap_[0]=0;
      levels++;
      overlap_[levels]=o; overlap_[levels+1]=0;
      init();
    };
    //! our dimension
    // const static int griddim = DIM;
    enum { griddim = DIM };
    //! give access to our communicator
    const MPI_Comm & comm() const { return comm_; };
    // inherited member functions
    void refine(refStrategy r);
    level roughest() const { return 0; };
    level smoothest() const { return levels-1; };
    iterator begin(level l=0) const { return iterator(max(l-1),*this); };
    iterator end(level l=0) const { return iterator(max(l),*this); };
    //! loop over all vertices
    template <class stubEngine>
    void loop_all(level l, stubEngine & );
    //! loop over all owner vertices
    template <class stubEngine>
    void loop_owner(level l, stubEngine & );
    //! loop over all ! overlap vertices
    template <class stubEngine>
    void loop_not_overlap(level l, stubEngine & );
    //! loop over all border vertices
    template <class stubEngine>
    void loop_border(level l, stubEngine & stub) {
      static array<2> a(0);
      static array<DIM, array<2> > b(a);
      loop_border(l, stub, a);
    };
    template <class stubEngine>
    void loop_border(level l, stubEngine & ,
                     const array<DIM, array<2> > & );
    //! loop over all overlap vertices
    template <class stubEngine>
    void loop_overlap(level l, stubEngine & );
    //! this feature is deprecated we should put this in private
    const array<DIM> &init_add(level l) const {
      array<DIM> & add = ((spgrid<DIM>*) this)->add_buffer;
      add = 0;
      for (int d=0; d<DIM; d++) {
        add[d] = 1;
        for (int dd=d+1; dd<DIM; dd++)
          add[d] *= front_overlap(l,dd) + size(l,dd) + end_overlap(l,dd);
      }
      return add;
    }
    const array<DIM> &init_add(level l, const array<DIM> &process) const {
      array<DIM> & add = ((spgrid<DIM>*) this)->add_buffer;
      add = 0;
      for (int d=0; d<DIM; d++) {
        add[d] = 1;
        for (int dd=d+1; dd<DIM; dd++)
          add[d] *= front_overlap(l,dd,process)
                    + size(l,dd,process) + end_overlap(l,dd,process);
      }
      return add;
    }
    //! overlap size at front
    int front_overlap(level l, int d, const array<DIM> &process) const;
    int front_overlap(level l, int d) const;
    //! overlap size at end
    int end_overlap(level l, int d, const array<DIM> &process) const;
    int end_overlap(level l, int d) const;
    //! local grid size in direction d on level l (no overlap)
    int size(level l, int d, const array<DIM> &process) const;
    int size(level l, int d) const;
    //! do we have a left neigbour in direction d?
    bool do_front_share(int d) const {
      return (dim_[d] > 1) &&
             !((periodic_[d]==false)&&(process_[d]==0));
    };
    //! does process have a left neigbour in direction d?
    bool do_front_share(int d, const array<DIM> &process) const {
      return (((dim_[d] > 1) && (process[d]!=0)) ||
              (periodic_[d]==true));
    };
    //! do we have a right neigbour in direction d?
    bool do_end_share(int d) const {
      return (dim_[d] > 1) &&
             !((periodic_[d]==false)&&(process_[d]==dim_[d]-1));
    };
    //! does process have a right neigbour in direction d?
    bool do_end_share(int d, const array<DIM> &process) const {
      return (((dim_[d] > 1) && (process[d]!=dim_[d]-1)) ||
              (periodic_[d]==true));
    };
    const array<DIM> & process() { return process_; }
  private:
    int father_id(level l, const array<DIM> & coord) const;
  }; /* end spgrid */

  ////////////////////////////////////////////////////////////////////////////
  /// The Remote-Index for the structured parallel Grid

  template<int DIM>
  class spgrid<DIM>::remoteindex
  {
  private:
    remoteindex() {};
    friend class index;
    friend class remotelist;
  private:
    ID_t id_;
    int processor_;
    array<3,bool> flags_;
  public:
    array<DIM> coord_;
  public:
    remoteindex (const array<DIM> &coord, int p, int i,
                 const array<3,bool> &f) :
      id_(i), processor_(p), flags_(f), coord_(coord) {};
    int process() const { return processor_; };
    int id() const { return id_; };
    bool owner() const { return flags_[f_owner]; };
    bool border() const { return flags_[f_border]; };
    bool overlap() const { return flags_[f_overlap]; };
  }; /* end remoteindex */

  ////////////////////////////////////////////////////////////////////////////
  /// The Index in the structured parallel Grid

  template<int DIM>
  class spgrid<DIM>::remotelist {
  private:
    friend class index;
    remotelist(int s) : size(s) { list = new remoteindex[size]; };
  public:
    remoteindex *list;
    int size;
    remotelist(const remotelist &rl) : size(rl.size) {
      list = new remoteindex[size];
      for (int i=0; i<size; i++) {
        list[i]= rl.list[i];
      }
    };
    ~remotelist() {
      delete[] list;
    };
  };

  template<int DIM>
  class spgrid<DIM>::index
  {
  private:
    int id_;
    const spgrid<DIM> &g;
    level l_;
    index(const spgrid<DIM> &grid) : g(grid) {};
    index& operator = (const iterator &i) {
      id_=i.id_; l_=i.l_; return (*this);
    };
    friend class spgrid<DIM>::iterator;
  public:
    index (const iterator &i) : id_(i.id_), g(i.g), l_(i.l_) {};
    int id() const { return id_; };
    array<DIM>& coord() const { return g.id_to_coord(l_,id_); };
    int globalid() const;
    bool owner() const;
    bool border() const;
    bool overlap() const;
    remotelist remote() const;
  }; /* end index */

  ////////////////////////////////////////////////////////////////////////////
  /// The Iterator for the structured parallel Grid

  template<int DIM>
  class spgrid<DIM>::iterator
  {
    int id_;
    const spgrid<DIM> &g;
    level l_;
    index referenz;
  private:
    // calc overlap in direction d in this level
    // this levels overlap for a special dimension and process
    const MPI_Comm & comm() const { return g.comm_; };
    //! set the right level for my id
    void recalc_level() {
      for (l_=0; id_>=g.max(l_); l_++) ;
    };
  public:
    /*
     * Constructors
     */
    iterator (const iterator &i) :
      id_(i.id_), g(i.g), l_(i.l_), referenz(i.referenz), changed(i.changed)
    { };
    iterator (int i, const spgrid<DIM> &grid) :
      id_(i), g(grid), l_(0), referenz(grid), changed(true)
    {
      recalc_level();
    };
    iterator (level l,const array<DIM> &coord, const spgrid<DIM> &grid)
      : g(grid), l_(l), referenz(grid), changed(true)
    {
      assert (l_ < g.levels);
      id_=g.coord_to_id(l_,coord);
      level oldlevel=l_;
      recalc_level();
      if (oldlevel!=l_) {
        cout << g.rank_ << ": Constructor for element " << coord
             << " on level " << oldlevel << ", but lemt would be on level "
             << l_ << endl;
      }
    };
  public:
    /*
     * inherited member functions
     */
    iterator& operator ++() {
      ++id_; changed=true;
      if(id_>=g.max(l_)) l_++;
      return (*this);
    };
    iterator& operator --() {
      --id_; changed=true;
      if(id_<g.max(l_-1)) l_--;
      return (*this);
    };
    iterator& operator = (const iterator &i) {
      id_=i.id_; l_=i.l_; changed=true; return (*this);
    };
    bool operator ==(const iterator& I) const { return id_==I.id_; };
    bool operator !=(const iterator& I) const { return id_!=I.id_; };
    index& operator *() const;
    index* operator ->() const;
    bool changed;
#ifdef QUICKHACKNOELEMENTS
    iterator father() const;
    iterator left(int dim) const;
    iterator right(int dim) const;
    level lvl() const ;
    const array<DIM> &coord() const;
    int coord(int) const;
    int id() const {return id_;};
#endif // QUICKHACKNOELEMENTS
  }; /* end iterator */
     //////////////////////////////////////////////////////////////////////////

}; // namespace Dune

#include "spgrid/spgrid.cc"
#include "spgrid/spgrid_iterator.cc"
#include "spgrid/spgrid_index.cc"
#include "spgrid/spgrid_loop.cc"

#endif // SPGRID_HH
