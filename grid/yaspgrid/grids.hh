// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_GRIDS_HH__
#define __DUNE_GRIDS_HH__

// C++ includes
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <deque>

// C includes
#include <mpi.h>
#include <string.h>

// local inclides
#include "dune/common/array.hh"


namespace Dune {

  /** @defgroup yaspgrid Yet Another Structured Parallel Grid
      \ingroup GridCommon

     This is the basis of a parallel implementation of the dune grid interface
     supporting codim 0 and dim.

     You can also use the structured interface and write fast code.

     @{
   */


  // forward declarations
  template<int d, typename ct> class Grid;
  template<int d, typename ct> class SubGrid;

  const double tolerance=1E-13;

  /*! The Grid considered here describes a finite set \f$d\f$-tupels of the form
      \f[ G = \{ (k_0,\ldots,k_{d-1}) | o_o \leq k_i < o_i+s_i \}  \f]

          togehter with an affine mapping

          \f[ t : G \to R^d, \ \ \ t(k)_i = k_i h_i + r_i \f].

          Therefore a Grid is characterized by the following four quantities:

          - The origin \f$ o=(o_0,\ldots,o_{d-1}) \in Z^d\f$,
          - the size \f$ s=(s_0,\ldots,s_{d-1}) \in Z^d\f$,
          - the mesh width \f$ h=(h_0,\ldots,h_{d-1}) \in R^d\f$,
          - The shift \f$ r=(r_0,\ldots,r_{d-1}) \in R^d\f$. The shift can be used to interpret the
        points of a grid as midpoints of cells, faces, edges, etc.

                The Grid can be parametrized by the dimension d and the type to be used for the coordinates.

     Here is a graphical illustration of a grid:

        \image html  grid.png "A Grid."
        \image latex grid.eps "A Grid." width=\textwidth

        A grid can be manipulated either in the origin/size representation or in the
     min index / max index representation.

        A Grid allows to iterate over all its cells with an Iterator class.
   */
  template<int d, typename ct>
  class Grid {
  public:
    //! define types used for arguments
    typedef Dune::FixedArray<int,d>  iTupel;
    typedef Dune::FixedArray<ct,d> fTupel;
    typedef Dune::FixedArray<bool,d> bTupel;

    //! Make an empty Grid with origin 0
    Grid ()
    {
      _origin = 0;
      _size = 0;
      _h = 0;
      _r = 0;
    }

    //! Make Grid from origin and size arrays
    Grid (iTupel o, iTupel s, fTupel h, fTupel r)
    {
      for (int i=0; i<d; ++i)
      {
        _origin[i] = o[i];
        _size[i] = s[i];
        _h[i] = h[i];
        _r[i] = r[i];
      }
    }

    //! Return origin in direction i
    int origin (int i)
    {
      return _origin[i];
    }

    //! Set origin in direction i
    void origin (int i, int oi)
    {
      _origin[i] = oi;
    }

    //! return reference to origin
    iTupel& origin ()
    {
      return _origin;
    }

    //! Return size in direction i
    int size (int i)
    {
      return _size[i];
    }

    //! Set size in direction i
    void size (int i, int si)
    {
      _size[i] = oi;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return reference to size tupel
    iTupel& size ()
    {
      return _size;
    }

    //! Return total size of index set which is the product of all size per direction.
    int totalsize ()
    {
      int s=1;
      for (int i=0; i<d; ++i) s=s*_size[i];
      return s;
    }

    //! Return minimum index in direction i
    int min (int i)
    {
      return _origin[i];
    }

    //! Set minimum index in direction i
    void min (int i, int mi)
    {
      _size[i] = max(i)-mi+1;
      _origin[i] = mi;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return maximum index in direction i
    int max (int i)
    {
      return _origin[i]+_size[i]-1;
    }

    //! Set maximum index in direction i
    void max (int i, int mi)
    {
      _size[i] = mi-min(i)+1;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return reference to mesh size tupel for read write access
    fTupel& meshsize ()
    {
      return _h;
    }

    //! Return mesh size in direction i
    ct meshsize (int i) const
    {
      return _h[i];
    }

    //! Set mesh size in direction i
    void meshsize (int i, int hi)
    {
      _h[i] = hi;
    }

    //! Return shift tupel
    fTupel& shift ()
    {
      return _r;
    }

    //! Return shift in direction i
    ct shift (int i)
    {
      return _r[i];
    }

    //! Set shift in direction i
    void shift (int i, int ri)
    {
      _r[i] = ri;
    }

    //! Return true if Grid is empty, i.e. has size 0 in all directions.
    bool empty ()
    {
      for (int i=0; i<d; ++i) if (_size[i]<=0) return true;
      return false;
    }

    //! given a tupel compute its index in the lexicographic numbering
    int index (const iTupel& coord) const
    {
      int index = (coord[d-1]-_origin[d-1]);

      for (int i=d-2; i>=0; i--)
        index = index*_size[i] + (coord[i]-_origin[i]);

      return index;
    }

    //! given a tupel compute its index in the lexicographic numbering
    bool inside (const iTupel& coord) const
    {
      for (int i=0; i<d; i++)
      {
        if (coord[i]<_origin[i] || coord[i]>=_origin[i]+_size[i]) return false;
      }
      return true;
    }

    //! Return new SubGrid of self which is the intersection of self and another Grid
    SubGrid<d,ct> intersection ( Grid<d,ct>& r)
    {
      // check if the two grids can be intersected, must have same mesh size and shift
      for (int i=0; i<d; i++)
        if (fabs(meshsize(i)-r.meshsize(i))>tolerance) return SubGrid<d,ct>();
      for (int i=0; i<d; i++)
        if (fabs(shift(i)-r.shift(i))>tolerance) return SubGrid<d,ct>();

      iTupel neworigin;
      iTupel newsize;
      iTupel offset;

      for (int i=0; i<d; ++i)
      {
        // intersect
        neworigin[i] = std::max(min(i),r.min(i));
        newsize[i] = std::min(max(i),r.max(i))-neworigin[i]+1;
        if (newsize[i]<0) {
          newsize[i] = 0;
          neworigin[i] = min(i);
        }

        // offset to own origin
        offset[i] = neworigin[i]-_origin[i];
      }
      return SubGrid<d,ct>(neworigin,newsize,offset,_size,_h,_r);
    }

    //! return grid moved by the vector v
    Grid<d,ct> move (iTupel v)
    {
      for (int i=0; i<d; i++) v[i] += _origin[i];
      return Grid<d,ct>(v,_size,_h,_r);
    }

    /*! Iterator class allows one to run over all cells of a grid.
       The cells of the grid to iterate over are numbered consecutively starting
       with zero. Via the index() method the iterator provides a mapping of the
       cells of the grid to a one-dimensional array. The number of entries
       in this array must be the size of the grid.
     */
    class Iterator {
    public:
      //! Make iterator pointing to first cell in a grid.
      Iterator (Grid<d,ct>& r)
      {
        // copy data coming from grid to iterate over
        for (int i=0; i<d; ++i) _origin[i] = r.origin(i);
        for (int i=0; i<d; ++i) _end[i] = r.origin(i)+r.size(i)-1;

        // initialize to first position in index set
        for (int i=0; i<d; ++i) _coord[i] = _origin[i];
        _index = 0;

        // compute increments;
        int inc = 1;
        for (int i=0; i<d; ++i)
        {
          _increment[i] = inc;
          inc *= r.size(i);
        }
      }

      //! Make iterator pointing to one past the last cell of a grid
      Iterator (int last)
      {
        _index = last;
      }

      //! Return true when two iterators over the same grid are equal (!).
      bool operator== (const Iterator& i)
      {
        return _index == i._index;
      }

      //! Return true when two iterators over the same grid are not equal (!).
      bool operator!= (const Iterator& i)
      {
        return _index != i._index;
      }

      //! Return index of the current cell in the consecutive numbering.
      int index ()
      {
        return _index;
      }

      //! Return coordinate of the cell in direction i.
      int coord (int i)
      {
        return _coord[i];
      }

      //! Return coordinate of the cell as reference (do not modify).
      iTupel& coord ()
      {
        return _coord;
      }

      //! Get index of cell which is dist cells away in direction i.
      int neighbor (int i, int dist)
      {
        return _index+dist*_increment[i];
      }

      //! Get index of neighboring cell which is -1 away in direction i.
      int down (int i)
      {
        return _index-_increment[i];
      }

      //! Get index of neighboring cell which is +1 away in direction i.
      int up (int i)
      {
        return _index+_increment[i];
      }

      //! Increment iterator to next cell.
      Iterator& operator++ ()
      {
        ++_index;
        for (int i=0; i<d; i++)
          if (++(_coord[i])<=_end[i])
            return *this;
          else { _coord[i]=_origin[i]; }
        return *this;
      }

      //! Print position of iterator
      void print (std::ostream& s)
      {
        s << index() << " : [";
        for (int i=0; i<d-1; i++) s << coord(i) << ",";
        s << coord(d-1) << "]";
      }

    protected:
      int _index;              //< current lexicographic position in index set
      iTupel _coord;           //< current position in index set
      iTupel _increment;       //< increment for next neighbor in direction i
      iTupel _origin;          //< origin and
      iTupel _end;             //< last index in direction i
    };

    //! return iterator to first element of index set
    Iterator begin () {return Iterator(*this);}

    //! return iterator to one past the last element of index set
    Iterator end () {return Iterator(totalsize());}

    /*! TransformingIterator is an Iterator providing in addition a linear transformation
       of the coordinates of the grid in the form \f$ y_i = x_i h_i + s_i \f$.
       This can be used to interpret the grid cells as vertices, edges, faces, etc.
     */
    class TransformingIterator : public Iterator {
    public:
      //! Make iterator pointing to first cell in a grid.
      TransformingIterator (Grid<d,ct>& r) : Iterator(r)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = _begin[i];
      }

      //! Increment iterator to next cell with position.
      TransformingIterator& operator++ ()
      {
        ++_index;
        for (int i=0; i<d; i++)
          if (++(_coord[i])<=_end[i])
          {
            _position[i] += _h[i];
            return *this;
          }
          else
          {
            _coord[i]=_origin[i];
            _position[i] = _begin[i];
          }
        return *this;
      }

      //! Return position of current cell in direction i.
      ct position (int i)
      {
        return _position[i];
      }

      //! Return position of current cell as reference.
      fTupel position ()
      {
        return _position;
      }

      //! Move cell position by dist cells in direction i.
      void move (int i, int dist)
      {
        _position[i] += dist*_h[i];
      }

      //! Print contents of iterator
      void print (std::ostream& s)
      {
        Iterator::print(s);
        s << " " << _position;
      }

    private:
      fTupel _h;            //!< mesh size per direction
      fTupel _begin;        //!< position of origin of grid
      fTupel _position;     //!< current position
    };

    //! return iterator to first element of index set
    TransformingIterator tbegin ()
    {
      return TransformingIterator(*this);
    }

  protected:
    //! internal representation uses origin/size
    iTupel _origin;
    iTupel _size;
    fTupel _h;            //!< mesh size per direction
    fTupel _r;            //!< shift per direction
  };

  //! Output operator for grids
  template <int d, typename ct>
  inline std::ostream& operator<< (std::ostream& s, Grid<d,ct> e)
  {
    s << "{";
    for (int i=0; i<d-1; i++)
      s << "[" << e.min(i) << "," << e.max(i) << "]x";
    s << "[" << e.min(d-1) << "," << e.max(d-1) << "]";
    s << " = [";
    for (int i=0; i<d-1; i++) s << e.origin(i) << ",";
    s << e.origin(d-1) << "]x[";
    for (int i=0; i<d-1; i++) s << e.size(i) << ",";
    s << e.size(d-1) << "]";
    s << " h=[";
    for (int i=0; i<d-1; i++) s << e.meshsize(i) << ",";
    s << e.meshsize(d-1) << "]";
    s << " r=[";
    for (int i=0; i<d-1; i++) s << e.shift(i) << ",";
    s << e.shift(d-1) << "]";
    s << "}";
    return s;
  }


  /*! A SubGrid is a grid that is embedded in a larger grid
     It is characterized by an offset and an enclosing grid as
     shown in the following picture:

        \image html  subgrid.png "The SubGrid is shown in red, blue is the enclosing grid."
        \image latex subgrid.eps "The SubGrid is shown in red, blue is the enclosing grid." width=\textwidth

     SubGrid has additional iterators that provide a mapping to
     the consecutive index in the enclosing grid.
   */
  template<int d, typename ct>
  class SubGrid : public Grid<d,ct> {
  public:
    typedef typename Grid<d,ct>::iTupel iTupel;
    typedef typename Grid<d,ct>::fTupel fTupel;
    typedef typename Grid<d,ct>::bTupel bTupel;

    //! make uninitialized subgrid
    SubGrid () {}

    //! Make SubGrid from origin, size, offset and supersize
    SubGrid (iTupel origin, iTupel size, iTupel offset, iTupel supersize, fTupel h, fTupel r) : Grid<d,ct>::Grid(origin,size,h,r)
    {
      for (int i=0; i<d; ++i)
      {
        _offset[i] = offset[i];
        _supersize[i] = supersize[i];
        if (offset[i]<0)
          std::cout << "warning: offset["
          << i <<"] negative in SubGrid"
          << std::endl;
        if (-offset[i]+supersize[i]<size[i])
          std::cout << "warning: subgrid larger than enclosing grid in direction "
          << i <<" in SubGrid"
          << std::endl;
      }
    }

    //! Return offset to origin of enclosing grid
    int offset (int i)
    {
      return _offset[i];
    }

    //! Return offset to origin of enclosing grid
    iTupel offset ()
    {
      return _offset;
    }

    //! return size of enclosing grid
    int supersize (int i)
    {
      return _supersize[i];
    }

    //! return size of enclosing grid
    iTupel supersize ()
    {
      return _supersize;
    }

    //! Return SubGrid of supergrid of self which is the intersection of self and another Grid
    SubGrid<d,ct> intersection ( Grid<d,ct>& r)
    {
      // check if the two grids can be intersected, must have same mesh size and shift
      for (int i=0; i<d; i++)
        if (fabs(meshsize(i)-r.meshsize(i))>tolerance) return SubGrid<d,ct>();
      for (int i=0; i<d; i++)
        if (fabs(shift(i)-r.shift(i))>tolerance) return SubGrid<d,ct>();

      iTupel neworigin;
      iTupel newsize;
      iTupel offset;

      for (int i=0; i<d; ++i)
      {
        // intersect
        neworigin[i] = std::max(min(i),r.min(i));
        newsize[i] = std::min(max(i),r.max(i))-neworigin[i]+1;
        if (newsize[i]<0) {
          newsize[i] = 0;
          neworigin[i] = min(i);
        }

        // offset to my supergrid
        offset[i] = _offset[i]+neworigin[i]-origin(i);
      }
      return SubGrid<d,ct>(neworigin,newsize,offset,_supersize,meshsize(),shift());
    }

    /*! SubIterator is an Iterator that provides in addition the consecutive
       index in the enclosing grid.
     */
    class SubIterator : public Grid<d,ct>::Iterator {
    public:
      //! Make iterator pointing to first cell in subgrid.
      SubIterator (SubGrid<d,ct>& r) : Grid<d,ct>::Iterator::Iterator (r)
      {
        //! store some grid information
        for (int i=0; i<d; ++i) _size[i] = r.size(i);

        // compute superincrements
        int inc = 1;
        for (int i=0; i<d; ++i)
        {
          _superincrement[i] = inc;
          inc *= r.supersize(i);
        }

        // move superindex to first cell in subgrid
        _superindex = 0;
        for (int i=0; i<d; ++i)
          _superindex += r.offset(i)*_superincrement[i];
      }

      //! Return consecutive index in enclosing grid
      int superindex ()
      {
        return _superindex;
      }

      //! Get index of cell which is dist cells away in direction i in enclosing grid.
      int superneighbor (int i, int dist)
      {
        return _superindex+dist*_superincrement[i];
      }

      //! Get index of neighboring cell which is -1 away in direction i in enclosing grid.
      int superdown (int i)
      {
        return _superindex-_superincrement[i];
      }

      //! Get index of neighboring cell which is +1 away in direction i in enclosing grid.
      int superup (int i)
      {
        return _superindex+_superincrement[i];
      }

      //! Increment iterator to next cell in subgrid
      SubIterator& operator++ ()
      {
        ++_index;                               // update consecutive index in grid
        for (int i=0; i<d; i++)                 // check for wrap around
        {
          _superindex += _superincrement[i];               // move on cell in direction i
          if (++(_coord[i])<=_end[i])
            return *this;
          else
          {
            _coord[i]=_origin[i];                         // move back to origin in direction i
            _superindex -= _size[i]*_superincrement[i];
          }
        }
        return *this;
      }

      //! Print position of iterator
      void print (std::ostream& s)
      {
        Grid<d,ct>::Iterator::print(s);
        s << " super=" << superindex();
      }

    protected:
      int _superindex;            //!< consecutive index in enclosing grid
      iTupel _superincrement;     //!< moves consecutive index by one in this direction in supergrid
      iTupel _size;               //!< size of subgrid
    };

    //! return subiterator to first element of index set
    SubIterator subbegin () {return SubIterator(*this);}

    /*! TransformingSubIterator is a SubIterator providing in addition a linear transformation
       of the coordinates of the grid in the form \f$ y_i = x_i h_i + s_i \f$.
       This can be used to interpret the grid cells as vertices, edges, faces, etc.
     */
    class TransformingSubIterator : public SubIterator {
    public:
      //! Make iterator pointing to first cell in a grid.
      TransformingSubIterator (SubGrid<d,ct>& r) : SubIterator(r)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = _begin[i];
      }

      //! Increment iterator to next cell with position.
      TransformingSubIterator& operator++ ()
      {
        ++_index;                               // update consecutive index in subgrid
        for (int i=0; i<d; i++)                 // check for wrap around
        {
          _superindex += _superincrement[i];               // move on cell in direction i
          if (++(_coord[i])<=_end[i])
          {
            _position[i] += _h[i];
            return *this;
          }
          else
          {
            _coord[i]=_origin[i];                         // move back to origin in direction i
            _superindex -= _size[i]*_superincrement[i];
            _position[i] = _begin[i];
          }
        }
        return *this;
      }

      //! Return position of current cell in direction i.
      ct position (int i)
      {
        return _position[i];
      }

      //! Return position of current cell as an array.
      fTupel position ()
      {
        return _position;
      }

      //! Move cell position by dist cells in direction i.
      void move (int i, int dist)
      {
        _position[i] += dist*_h[i];
      }

      //! Print contents of iterator
      void print (std::ostream& s)
      {
        SubIterator::print(s);
        s << " [";
        for (int i=0; i<d-1; i++) s << position(i) << ",";
        s << position(d-1) << "]";
      }

    private:
      fTupel _h;            //!< mesh size per direction
      fTupel _begin;        //!< position of origin of grid
      fTupel _position;     //!< current position
    };

    //! return iterator to first element of index set
    TransformingSubIterator tsubbegin ()
    {
      return TransformingSubIterator(*this);
    }

  private:
    iTupel _offset;        //!< offset to origin of the enclosing grid
    iTupel _supersize;     //!< size of the enclosing grid
  };


  //! Output operator for subgrids
  template <int d, typename ct>
  inline std::ostream& operator<< (std::ostream& s, SubGrid<d,ct> e)
  {
    Grid<d,ct> x = e;
    s << x << " ofs=" << e.offset() << " ss=" << e.supersize();
    return s;
  }


  /*! Torus provides all the functionality to handle a toroidal communication structure:

     - Map a set of processes (given by an MPI communicator) to a torus of dimension d. The "optimal"
     torus dimensions are determined by a coarse mesh. The maximum side length is minimized.

     - Provide lists of neighboring processes and a method for nearest neighbor exchange
     using asynchronous communication with MPI. The periodic case is handled where one process
     might have to exchange several messages with the same process. (Logically, a process has always
     \f$3^d-1\f$ neighbors, but several of these logical neighbors might be identical)

     - Provide means to partition a grid to the torus.

   */
  template<int d>
  class Torus {
  public:
    //! type used to pass tupels in and out
    typedef Dune::FixedArray<int,d> iTupel;
    typedef Dune::FixedArray<bool,d> bTupel;


  private:
    struct CommPartner {
      int rank;
      iTupel delta;
      int index;
    };

    struct CommTask {
      int rank;          // process to send to / receive from
      void *buffer;      // buffer to send / receive
      int size;          // size of buffer
      MPI_Request request;     // used by MPI to handle request
      int flag;          // used by MPI
    };

  public:
    //! constructor making uninitialized object
    Torus ()
    {  }

    //! make partitioner from communicator and coarse mesh size
    Torus (MPI_Comm comm, int tag, iTupel size)
    {
      // MPI stuff
      _comm = comm;
      MPI_Comm_size(comm,&_procs);
      MPI_Comm_rank(comm,&_rank);
      _tag = tag;

      // determine dimensions
      iTupel dims;
      double opt=1E100;
      optimize_dims(d-1,size,_procs,dims,opt);
      if (_rank==0) std::cout << "Torus<" << d
        << ">: mapping " << _procs << " processes onto "
        << _dims << " torus." << std::endl;

      // compute increments for lexicographic ordering
      int inc = 1;
      for (int i=0; i<d; i++)
      {
        _increment[i] = inc;
        inc *= _dims[i];
      }

      // make full schedule
      proclists();
    }

    //! return own rank
    int rank ()
    {
      return _rank;
    }

    //! return own coordinates
    iTupel coord ()
    {
      return rank_to_coord(_rank);
    }

    //! return number of processes
    int procs ()
    {
      return _procs;
    }

    //! return dimensions of torus
    iTupel dims ()
    {
      return _dims;
    }

    //! return dimensions of torus in direction i
    int dims (int i)
    {
      return _dims[i];
    }

    //! return MPI communicator
    MPI_Comm comm ()
    {
      return _comm;
    }

    //! return tag used by torus
    int tag ()
    {
      return _tag;
    }

    //! return true if coordinate is inside torus
    bool inside (iTupel c)
    {
      for (int i=d-1; i>=0; i--)
        if (c[i]<0 || c[i]>=_dims[i]) return false;
      return true;
    }

    //! map rank to coordinate in torus using lexicographic ordering
    iTupel rank_to_coord (int rank)
    {
      iTupel coord;
      rank = rank%_procs;
      for (int i=d-1; i>=0; i--)
      {
        coord[i] = rank/_increment[i];
        rank = rank%_increment[i];
      }
      return coord;
    }

    //! map coordinate in torus to rank using lexicographic ordering
    int coord_to_rank (iTupel coord)
    {
      for (int i=0; i<d; i++) coord[i] = coord[i]%_dims[i];
      int rank = 0;
      for (int i=0; i<d; i++) rank += coord[i]*_increment[i];
      return rank;
    }

    //! return rank of process where its coordinate in direction dir has offset cnt (handles periodic case)
    int rank_relative (int rank, int dir, int cnt)
    {
      iTupel coord = rank_to_coord(rank);
      coord[dir] = (coord[dir]+dims[dir]+cnt)%dims[dir];
      return coord_to_rank(coord);
    }

    //! assign color to given coordinate
    int color (iTupel coord)
    {
      int c = 0;
      int power = 1;

      // interior coloring
      for (int i=0; i<d; i++)
      {
        if (coord[i]%2==1) c += power;
        power *= 2;
      }

      // extra colors for boundary processes
      for (int i=0; i<d; i++)
      {
        if (_dims[i]>1 && coord[i]==_dims[i]-1) c += power;
        power *= 2;
      }

      return c;
    }

    //! assign color to given rank
    int color (int rank)
    {
      return color(rank_to_coord(rank));
    }

    //! return the number of neighbors, which is \f$3^d-1\f$
    int neighbors ()
    {
      int n=1;
      for (int i=0; i<d; ++i)
        n *= 3;
      return n-1;
    }

    //! return true if neighbor with given delta is a neighbor under the given periodicity
    bool is_neighbor (iTupel delta, bTupel periodic)
    {
      iTupel coord = rank_to_coord(_rank);     // my own coordinate with 0 <= c_i < dims_i


      for (int i=0; i<d; i++)
      {
        if (delta[i]<0)
        {
          // if I am on the boundary and domain is not periodic => no neighbor
          if (coord[i]==0 && periodic[i]==false) return false;
        }
        if (delta[i]>0)
        {
          // if I am on the boundary and domain is not periodic => no neighbor
          if (coord[i]==_dims[i]-1 && periodic[i]==false) return false;
        }
      }
      return true;
    }

    //! partition the given grid onto the torus and return the piece of the process with given rank; returns load imbalance
    double partition (int rank, iTupel origin_in, iTupel size_in, iTupel& origin_out, iTupel& size_out)
    {
      iTupel coord = rank_to_coord(rank);
      double maxsize = 1;
      double sz = 1;

      // make a tensor product partition
      for (int i=0; i<d; i++)
      {
        // determine
        int m = size_in[i]/_dims[i];
        int r = size_in[i]%_dims[i];

        sz *= size_in[i];

        if (coord[i]<_dims[i]-r)
        {
          origin_out[i] = origin_in[i] + coord[i]*m;
          size_out[i] = m;
          maxsize *= m;
        }
        else
        {
          origin_out[i] = origin_in[i] + (_dims[i]-r)*m + (coord[i]-(_dims[i]-r))*(m+1);
          size_out[i] = m+1;
          maxsize *= m+1;
        }
      }
      return maxsize/(sz/_procs);
    }

    /*!
       ProcListIterator provides access to a list of neighboring processes. There are always
       \f$ 3^d-1 \f$ entries in such a list. Two lists are maintained, one for sending and one for
       receiving. The lists are sorted in such a way that in sequence message delivery ensures that
       e.g. a message send to the left neighbor is received as a message from the right neighbor.
     */
    class ProcListIterator {
    public:
      //! make an iterator
      ProcListIterator (typename std::deque<CommPartner>::iterator iter)
      {
        i = iter;
      }

      //! return rank of neighboring process
      int rank ()
      {
        return i->rank;
      }

      //! return distance vector
      iTupel delta ()
      {
        return i->delta;
      }

      //! return index in proclist
      int index ()
      {
        return i->index;
      }

      //! return 1-norm of distance vector
      int distance ()
      {
        int dist = 0;
        iTupel delta=i->delta;
        for (int i=0; i<d; ++i)
          dist += std::abs(delta[i]);
        return dist;
      }

      //! Return true when two iterators point to same member
      bool operator== (const ProcListIterator& iter)
      {
        return i == iter.i;
      }


      //! Return true when two iterators do not point to same member
      bool operator!= (const ProcListIterator& iter)
      {
        return i != iter.i;
      }

      //! Increment iterator to next cell.
      ProcListIterator& operator++ ()
      {
        ++i;
        return *this;
      }

    private:
      typename std::deque<CommPartner>::iterator i;
    };

    //! first process in send list
    ProcListIterator sendbegin ()
    {
      return ProcListIterator(_sendlist.begin());
    }

    //! end of send list
    ProcListIterator sendend ()
    {
      return ProcListIterator(_sendlist.end());
    }

    //! first process in receive list
    ProcListIterator recvbegin ()
    {
      return ProcListIterator(_recvlist.begin());
    }

    //! last process in receive list
    ProcListIterator recvend ()
    {
      return ProcListIterator(_recvlist.end());
    }

    //! store a send request; buffers are sent in order; handles also local requests with memcpy
    void send (int rank, void* buffer, int size)
    {
      CommTask task;
      task.rank = rank;
      task.buffer = buffer;
      task.size = size;
      if (rank!=_rank)
        _sendrequests.push_back(task);
      else
        _localsendrequests.push_back(task);
    }

    //! store a receive request; buffers are received in order; handles also local requests with memcpy
    void recv (int rank, void* buffer, int size)
    {
      CommTask task;
      task.rank = rank;
      task.buffer = buffer;
      task.size = size;
      if (rank!=_rank)
        _recvrequests.push_back(task);
      else
        _localrecvrequests.push_back(task);
    }

    //! exchange messages stored in request buffers; clear request buffers afterwards
    void exchange ()
    {
      // handle local requests first
      if (_localsendrequests.size()!=_localrecvrequests.size())
      {
        std::cout << "[" << rank() << "]: ERROR: local sends/receives do not match in exchange!" << std::endl;
        return;
      }
      for (int i=0; i<_localsendrequests.size(); i++)
      {
        if (_localsendrequests[i].size!=_localrecvrequests[i].size)
        {
          std::cout << "[" << rank() << "]: ERROR: size in local sends/receive does not match in exchange!" << std::endl;
          return;
        }
        memcpy(_localrecvrequests[i].buffer,_localsendrequests[i].buffer,_localsendrequests[i].size);
      }
      _localsendrequests.clear();
      _localrecvrequests.clear();

      // handle foreign requests
      int sends=0;
      int recvs=0;

      // issue sends to foreign processes
      for (int i=0; i<_sendrequests.size(); i++)
        if (_sendrequests[i].rank!=rank())
        {
          MPI_Isend(_sendrequests[i].buffer, _sendrequests[i].size, MPI_BYTE,
                    _sendrequests[i].rank, _tag, _comm, &(_sendrequests[i].request));
          _sendrequests[i].flag = false;
          sends++;
        }

      // issue receives from foreign processes
      for (int i=0; i<_recvrequests.size(); i++)
        if (_recvrequests[i].rank!=rank())
        {
          MPI_Irecv(_recvrequests[i].buffer, _recvrequests[i].size, MPI_BYTE,
                    _recvrequests[i].rank, _tag, _comm, &(_recvrequests[i].request));
          _recvrequests[i].flag = false;
          recvs++;
        }

      // poll sends
      while (sends>0)
      {
        for (int i=0; i<_sendrequests.size(); i++)
          if (!_sendrequests[i].flag)
          {
            MPI_Status status;
            MPI_Test( &(_sendrequests[i].request), &(_sendrequests[i].flag), &status);
            if (_sendrequests[i].flag)
              sends--;
          }
      }

      // poll receives
      while (recvs>0)
      {
        for (int i=0; i<_recvrequests.size(); i++)
          if (!_recvrequests[i].flag)
          {
            MPI_Status status;
            MPI_Test( &(_recvrequests[i].request), &(_recvrequests[i].flag), &status);
            if (_recvrequests[i].flag)
              recvs--;
          }
      }

      // clear request buffers
      _sendrequests.clear();
      _recvrequests.clear();
    }

    //! global sum
    double global_sum (double x)
    {
      double res;

      if (_procs==1) return x;
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_SUM,_comm);
      return res;
    }

    //! global max
    double global_max (double x)
    {
      double res;

      if (_procs==1) return x;
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_MAX,_comm);
      return res;
    }

    //! global min
    double global_min (double x)
    {
      double res;

      if (_procs==1) return x;
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_MIN,_comm);
      return res;
    }


    //! print contents of torus object
    void print (std::ostream& s)
    {
      s << "[" << rank() <<  "]: Torus " << procs() << " processor(s) arranged as " << dims() << std::endl;
      for (ProcListIterator i=sendbegin(); i!=sendend(); ++i)
      {
        s << "[" << rank() <<  "]: send to   "
        << "rank=" << i.rank()
        << " index=" << i.index()
        << " delta=" << i.delta() << " dist=" << i.distance() << std::endl;
      }
      for (ProcListIterator i=recvbegin(); i!=recvend(); ++i)
      {
        s << "[" << rank() <<  "]: recv from "
        << "rank=" << i.rank()
        << " index=" << i.index()
        << " delta=" << i.delta() << " dist=" << i.distance() << std::endl;
      }
    }

  private:

    void optimize_dims (int i, iTupel& size, int P, iTupel& dims, double &opt )
    {
      if (i>0)     // test all subdivisions recursively
      {
        for (int k=1; k<=P; k++)
          if (P%k==0)
          {
            // P divisible by k
            dims[i] = k;
            optimize_dims(i-1,size,P/k,dims,opt);
          }
      }
      else
      {
        // found a possible combination
        dims[0] = P;

        // check for optimality
        double m = -1.0;

        for (int k=0; k<d; k++)
          if ( ((double)size[k])/((double)dims[k]) > m )
            m = ((double)size[k])/((double)dims[k]);
        // if (_rank==0) std::cout << "testing " << dims << " norm=" << m << std::endl;

        if (m<opt)
        {
          opt = m;
          _dims = dims;
        }
      }
    }

    void proclists ()
    {
      // compile the full neighbor list
      CommPartner cp;
      iTupel delta;

      delta = -1;
      bool ready = false;
      iTupel me, nb;
      me = rank_to_coord(_rank);
      int index = 0;
      int last = neighbors()-1;
      while (!ready)
      {
        // find neighbors coordinates
        for (int i=0; i<d; i++)
          nb[i] = ( me[i]+_dims[i]+delta[i] ) % _dims[i];

        // find neighbors rank
        int nbrank = coord_to_rank(nb);

        // check if delta is not zero
        for (int i=0; i<d; i++)
          if (delta[i]!=0)
          {
            cp.rank = nbrank;
            cp.delta = delta;
            cp.index = index;
            _recvlist.push_back(cp);
            cp.index = last-index;
            _sendlist.push_front(cp);
            index++;
            break;
          }

        // next neighbor
        ready = true;
        for (int i=0; i<d; i++)
          if (delta[i]<1)
          {
            (delta[i])++;
            ready=false;
            break;
          }
          else
          {
            delta[i] = -1;
          }
      }

    }

    MPI_Comm _comm;
    int _rank;
    int _procs;
    iTupel _dims;
    iTupel _increment;
    int _tag;
    std::deque<CommPartner> _sendlist;
    std::deque<CommPartner> _recvlist;

    std::vector<CommTask> _sendrequests;
    std::vector<CommTask> _recvrequests;
    std::vector<CommTask> _localsendrequests;
    std::vector<CommTask> _localrecvrequests;
  };

  //! Output operator for Torus
  template <int d>
  inline std::ostream& operator<< (std::ostream& s, Torus<d> t)
  {
    t.print(s);
    return s;
  }


  /*! MultiGrid manages a d-dimensional grid mapped to a set of processes.
   */
  template<int d, typename ct>
  class MultiGrid {
  private:
    // some data types
    struct Intersection {
      SubGrid<d,ct> grid;     // the intersection as a subgrid of local grid
      int rank;               // rank of process where other grid is stored
      int distance;           // manhattan distance to other grid
    };

    struct GridLevel {
      Grid<d,ct> cell_global;           // the whole grid on that level
      Grid<d,ct> cell_local;            // our part on that level;
      SubGrid<d,ct> cell_master;        // points associated uniquely to this process; subgrid of local
      std::deque<Intersection> send_cell_local_local;      // each intersection is a subgrid of local
      std::deque<Intersection> recv_cell_local_local;      // each intersection is a subgrid of local
      std::deque<Intersection> send_cell_master_local;      // each intersection is a subgrid of local
      std::deque<Intersection> recv_cell_local_master;      // each intersection is a subgrid of local

      Grid<d,ct> vertex_global;           // the whole grid on that level
      Grid<d,ct> vertex_local;            // our part on that level;
      SubGrid<d,ct> vertex_master;        // points associated uniquely to this process; subgrid of local
      std::deque<Intersection> send_vertex_local_local;      // each intersection is a subgrid of local
      std::deque<Intersection> recv_vertex_local_local;      // each intersection is a subgrid of local
      std::deque<Intersection> send_vertex_master_local;      // each intersection is a subgrid of local
      std::deque<Intersection> recv_vertex_local_master;      // each intersection is a subgrid of local

      MultiGrid<d,ct>* mg;     // each grid level knows its multigrid
      int overlap;            // on this level
    };

  public:
    //! define types used for arguments
    typedef Dune::FixedArray<int,d> iTupel;
    typedef Dune::FixedArray<ct,d> fTupel;
    typedef Dune::FixedArray<bool,d> bTupel;

    // communication tag used by multigrid
    enum { tag = 17 };

    MultiGrid (MPI_Comm comm, fTupel L)
    {}

    //! constructor making a grid
    MultiGrid (MPI_Comm comm, fTupel L, iTupel s, bTupel periodic, int overlap)
      : _torus(comm,tag,s)
    {
      // store parameters
      _L = L;
      _s = s;
      _periodic = periodic;
      _overlap = overlap;

      // make the first mesh level
      GridLevel g;
      g.overlap = overlap;
      g.mg = this;

      // the global cell grid
      iTupel o = 0;
      fTupel h;
      fTupel r;
      for (int i=0; i<d; i++) h[i] = L[i]/s[i];
      for (int i=0; i<d; i++) r[i] = 0.5*h[i];
      g.cell_global = Grid<d,ct>(o,s,h,r);

      // the local cell master grid obtained through partitioning of global grid
      iTupel o_master;
      iTupel s_master;
      double imbal = _torus.partition(_torus.rank(),o,s,o_master,s_master);
      imbal = _torus.global_max(imbal);

      // extend the cell master grid by overlap considering periodicity
      iTupel o_local;
      iTupel s_local;
      for (int i=0; i<d; i++)
      {
        if (periodic[i])
        {
          // easy case, extend by 2 overlaps
          o_local[i] = o_master[i]-overlap;
          s_local[i] = s_master[i]+2*overlap;
        }
        else
        {
          // nonperiodic case, lower boundary
          int min = std::max(0,o_master[i]-overlap);
          int max = std::min(s[i],o_master[i]+s_master[i]+overlap);
          o_local[i] = min;
          s_local[i] = max-min;
        }
      }
      g.cell_local = Grid<d,ct>(o_local,s_local,h,r);

      // now make the master grid a subgrid of the local grid
      iTupel offset;
      for (int i=0; i<d; i++) offset[i] = o_master[i]-o_local[i];
      g.cell_master = SubGrid<d,ct>(o_master,s_master,offset,s_local,h,r);

      // compute intersections
      cell_intersections(g);

      // add level
      _maxlevel = 0;
      _levels[_maxlevel] = g;

      // output
      if (_torus.rank()==0) std::cout << "MultiGrid<" << d
        << ">: coarse grid with size " << s
        << " imbalance=" << (imbal-1)*100 << "%" << std::endl;
    }

    //! return the maximum level index (number of levels is maxlevel()+1)
    int maxlevel ()
    {
      return _maxlevel;
    }

    //! return true if grid is periodic in given direction
    bool periodic (int i)
    {
      return _periodic[i];
    }

    //! do a global mesh refinement; true: keep overlap in absolute size; false: keep overlap in mesh cells
    void refine (bool keep_overlap)
    {
      // access to coarser grid level
      GridLevel& cg = _levels[maxlevel()];

      // make new mesh level
      GridLevel g;
      if (keep_overlap) g.overlap = 2*cg.overlap;else g.overlap = cg.overlap;
      g.mg = this;

      // the global cell grid
      iTupel o = 0;
      iTupel s;
      fTupel h;
      fTupel r;
      for (int i=0; i<d; i++) s[i] = 2*cg.cell_global.size(i);
      for (int i=0; i<d; i++) h[i] = 0.5*cg.cell_global.meshsize(i);
      for (int i=0; i<d; i++) r[i] = 0.5*h[i];
      g.cell_global = Grid<d,ct>(o,s,h,r);

      // output
      if (_torus.rank()==0) std::cout << "MultiGrid<" << d << ">: refined to size " << s << std::endl;

      // the local cell master grid obtained from coarse grid cell master
      iTupel o_master;
      iTupel s_master;
      for (int i=0; i<d; i++) o_master[i] = 2*cg.cell_master.origin(i);
      for (int i=0; i<d; i++) s_master[i] = 2*cg.cell_master.size(i);

      // extend the cell master grid by overlap considering periodicity
      iTupel o_local;
      iTupel s_local;
      for (int i=0; i<d; i++)
      {
        if (_periodic[i])
        {
          // easy case, extend by 2 overlaps
          o_local[i] = o_master[i]-g.overlap;
          s_local[i] = s_master[i]+2*g.overlap;
        }
        else
        {
          // nonperiodic case, lower boundary
          int min = std::max(0,o_master[i]-g.overlap);
          int max = std::min(s[i],o_master[i]+s_master[i]+g.overlap);
          o_local[i] = min;
          s_local[i] = max-min;
        }
      }
      g.cell_local = Grid<d,ct>(o_local,s_local,h,r);

      // now make the master grid a subgrid of the local grid
      iTupel offset;
      for (int i=0; i<d; i++) offset[i] = o_master[i]-o_local[i];
      g.cell_master = SubGrid<d,ct>(o_master,s_master,offset,s_local,h,r);

      // compute intersections
      cell_intersections(g);

      // add level
      _maxlevel++;
      _levels[_maxlevel] = g;
    }

    //! return reference to torus
    Torus<d>& torus ()
    {
      return _torus;
    }

    //! provides access to intersections of grids
    class IntersectionIterator {
    public:
      //! make IntersectionIterator from stl iterator
      IntersectionIterator (typename std::deque<Intersection >::iterator iter)
      {
        i = iter;
      }

      //! get rank of neighboring process where other partner of intersection is stored
      int rank ()
      {
        return i->rank;
      }

      //! get distance to neighboring process
      int distance ()
      {
        return i->distance;
      }

      //! reference to subgrid in local grid
      SubGrid<d,ct>& operator*()
      {
        return i->grid;
      }

      //! pointer to subgrid in local grid
      SubGrid<d,ct>* operator->()
      {
        return &(i->grid);
      }

      //! Return true when two iterators point to same member
      bool operator== (const IntersectionIterator& iter) const
      {
        return i == iter.i;
      }

      //! Return true when two iterators do not point to same member
      bool operator!= (const IntersectionIterator& iter) const
      {
        return i != iter.i;
      }

      //! Increment iterator to next cell.
      IntersectionIterator& operator++ ()
      {
        ++i;
        return *this;
      }

    private:
      typename std::deque<Intersection>::iterator i;
    };

    //! provides access to a given grid level
    class GridLevelIterator {
    public:
      //! make iterator pointing to level 0 (coarsest grid)
      GridLevelIterator ()
      {       }

      //! make iterator pointing to level k (no check made)
      GridLevelIterator (GridLevel* start, int level)
      {
        i=start; l=level;
      }

      //! return number of this grid level
      int level ()
      {
        return l;
      }

      //! return pointer to multigrid object that contains this level
      MultiGrid<d,ct>* mg ()
      {
        return i->mg;
      }

      //! reference to global cell grid
      Grid<d,ct>& cell_global ()
      {
        return i->cell_global;
      }

      //! reference to local cell grid which is a subgrid of the global cell grid
      Grid<d,ct>& cell_local ()
      {
        return i->cell_local;
      }

      //! reference to cell master grid which is a subgrid of the local cell grid
      SubGrid<d,ct>& cell_master ()
      {
        return i->cell_master;
      }

      //! iterator access to intersection list
      IntersectionIterator send_cell_local_local_begin ()
      {
        return IntersectionIterator(i->send_cell_local_local.begin());
      }

      //! iterator access to intersection list
      IntersectionIterator send_cell_local_local_end ()
      {
        return IntersectionIterator(i->send_cell_local_local.end());
      }

      //! iterator access to intersection list
      IntersectionIterator recv_cell_local_local_begin ()
      {
        return IntersectionIterator(i->recv_cell_local_local.begin());
      }

      //! iterator access to intersection list
      IntersectionIterator recv_cell_local_local_end ()
      {
        return IntersectionIterator(i->recv_cell_local_local.end());
      }


      //! iterator access to intersection list
      IntersectionIterator send_cell_master_local_begin ()
      {
        return IntersectionIterator(i->send_cell_master_local.begin());
      }


      //! iterator access to intersection list
      IntersectionIterator send_cell_master_local_end ()
      {
        return IntersectionIterator(i->send_cell_master_local.end());
      }


      //! iterator access to intersection list
      IntersectionIterator recv_cell_local_master_begin ()
      {
        return IntersectionIterator(i->recv_cell_local_master.begin());
      }


      //! iterator access to intersection list
      IntersectionIterator recv_cell_local_master_end ()
      {
        return IntersectionIterator(i->recv_cell_local_master.end());
      }

      //! reference to global vertex grid
      Grid<d,ct>& vertex_global ()
      {
        return i->vertex_global;
      }

      //! reference to local vertex grid; is subgrid of global vertex grid
      Grid<d,ct>& vertex_local ()
      {
        return i->vertex_local;
      }

      //! reference to vertex master grid; is subgrid of local vertex grid
      SubGrid<d,ct>& vertex_master ()
      {
        return i->vertex_master;
      }

      //! iterator access to intersection list
      IntersectionIterator send_vertex_local_local_begin ()
      {
        return IntersectionIterator(i->send_vertex_local_local.begin());
      }

      //! iterator access to intersection list
      IntersectionIterator send_vertex_local_local_end ()
      {
        return IntersectionIterator(i->send_vertex_local_local.end());
      }

      //! iterator access to intersection list
      IntersectionIterator recv_vertex_local_local_begin ()
      {
        return IntersectionIterator(i->recv_vertex_local_local.begin());
      }

      //! iterator access to intersection list
      IntersectionIterator recv_vertex_local_local_end ()
      {
        return IntersectionIterator(i->recv_vertex_local_local.end());
      }


      //! iterator access to intersection list
      IntersectionIterator send_vertex_master_local_begin ()
      {
        return IntersectionIterator(i->send_vertex_master_local.begin());
      }


      //! iterator access to intersection list
      IntersectionIterator send_vertex_master_local_end ()
      {
        return IntersectionIterator(i->send_vertex_master_local.end());
      }


      //! iterator access to intersection list
      IntersectionIterator recv_vertex_local_master_begin ()
      {
        return IntersectionIterator(i->recv_vertex_local_master.begin());
      }


      //! iterator access to intersection list
      IntersectionIterator recv_vertex_local_master_end ()
      {
        return IntersectionIterator(i->recv_vertex_local_master.end());
      }

      //! Return true when two iterators point to same member
      bool operator== (const GridLevelIterator& iter) const
      {
        return i == iter.i;
      }

      //! Return true when two iterators do not point to same member
      bool operator!= (const GridLevelIterator& iter) const
      {
        return i != iter.i;
      }

      //! Increment iterator to next finer grid level
      GridLevelIterator& operator++ ()
      {
        ++i;
        ++l;
        return *this;
      }

      //! Increment iterator to coarser grid level
      GridLevelIterator& operator-- ()
      {
        --i;
        --l;
        return *this;
      }

    private:
      int l;
      GridLevel* i;
    };

    //! return iterator pointing to coarsest level
    GridLevelIterator begin ()
    {
      return GridLevelIterator(_levels,0);
    }

    //! return iterator pointing to coarsest level
    GridLevelIterator begin (int i)
    {
      return GridLevelIterator(_levels+i,i);
    }

    //! return iterator pointing to one past the finest level
    GridLevelIterator end ()
    {
      return GridLevelIterator(_levels+(_maxlevel+1),_maxlevel+1);
    }

    //! return iterator pointing to the finest level
    GridLevelIterator rbegin ()
    {
      return GridLevelIterator(_levels+_maxlevel,_maxlevel);
    }

    //! return iterator pointing to one before the coarsest level
    GridLevelIterator rend ()
    {
      return GridLevelIterator(_levels-1,-1);
    }

  private:

    void cell_intersections (GridLevel& g)
    {
      // clear all cell intersections
      g.send_cell_local_local.clear();
      g.recv_cell_local_local.clear();
      g.send_cell_master_local.clear();
      g.recv_cell_local_master.clear();

      // the exchange buffers
      std::vector<Grid<d,ct> > send_cell_local(_torus.neighbors());
      std::vector<Grid<d,ct> > recv_cell_local(_torus.neighbors());

      std::vector<Grid<d,ct> > send_cell_master(_torus.neighbors());
      std::vector<Grid<d,ct> > recv_cell_master(_torus.neighbors());

      // fill send buffers; iterate over all neighboring processes
      // non-periodic case is handled automatically because intersection will be zero
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
      {
        // determine (moved) grid to be sent to neighbor
        iTupel coord = _torus.coord();           // my coordinates
        iTupel delta = i.delta();                // delta to neighbor
        iTupel nb = coord;                       // the neighbor
        for (int k=0; k<d; k++) nb[k] += delta[k];
        iTupel v = 0;                            // grid movement

        for (int k=0; k<d; k++)
        {
          if (nb[k]<0) v[k] += g.cell_global.size(k);
          if (nb[k]>=_torus.dims(k)) v[k] -= g.cell_global.size(k);
        }

        // store moved grids in send buffers
        send_cell_local[i.index()] = g.cell_local.move(v);             // local grid
        send_cell_master[i.index()] = g.cell_master.move(v);           // master grid
      }

      // issue send requests for cell_local
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
        _torus.send(i.rank(), &send_cell_local[i.index()], sizeof(Grid<d,ct>));

      // issue recv requests for cell_local
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
        _torus.recv(i.rank(), &recv_cell_local[i.index()], sizeof(Grid<d,ct>));

      // exchange cell_local
      _torus.exchange();


      // issue send requests for cell_master
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
        _torus.send(i.rank(), &send_cell_master[i.index()], sizeof(Grid<d,ct>));

      // issue recv requests for cell_master
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
        _torus.recv(i.rank(), &recv_cell_master[i.index()], sizeof(Grid<d,ct>));

      // exchange cell_local
      _torus.exchange();

      // process receive buffers; compute intersections
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
      {
        // local<->local intersection
        Intersection intersection;
        intersection.grid = g.cell_local.intersection(recv_cell_local[i.index()]);
        intersection.rank = i.rank();
        intersection.distance = i.distance();
        if (!intersection.grid.empty()) g.recv_cell_local_local.push_back(intersection);
        if (!intersection.grid.empty()) g.send_cell_local_local.push_front(intersection);

        // master<->local intersections
        Intersection master_local;
        master_local.grid = g.cell_master.intersection(recv_cell_local[i.index()]);
        master_local.rank = i.rank();
        master_local.distance = i.distance();
        if (!master_local.grid.empty()) g.send_cell_master_local.push_front(master_local);

        Intersection local_master;
        local_master.grid = g.cell_local.intersection(recv_cell_master[i.index()]);
        local_master.rank = i.rank();
        local_master.distance = i.distance();
        if(!local_master.grid.empty()) g.recv_cell_local_master.push_back(local_master);
      }
    }

    fTupel _L;
    iTupel _s;
    bTupel _periodic;
    int _maxlevel;
    GridLevel _levels[32];
    int _overlap;
    Torus<d> _torus;
  };

  //! Output operator for multigrids
  template <int d, class ct>
  inline std::ostream& operator<< (std::ostream& s, MultiGrid<d,ct>& mg)
  {
    int rank = mg.torus().rank();

    s << "[" << rank << "]:" << " MultiGrid maxlevel=" << mg.maxlevel() << std::endl;

    for (typename MultiGrid<d,ct>::GridLevelIterator g=mg.begin(); g!=mg.end(); ++g)
    {
      s << "[" << rank << "]: " << "level=" << g.level() << std::endl;
      s << "[" << rank << "]:   " << "cell_global=" << g.cell_global() << std::endl;
      s << "[" << rank << "]:   " << "cell_local=" << g.cell_local() << std::endl;
      s << "[" << rank << "]:   " << "cell_master=" << g.cell_master() << std::endl;
      for (typename MultiGrid<d,ct>::IntersectionIterator i=g.send_cell_local_local_begin();
           i!=g.send_cell_local_local_end(); ++i)
      {
        s << "[" << rank << "]:    " << " s_c_l_l "
        << i.rank() << " " << *i << std::endl;
      }
      for (typename MultiGrid<d,ct>::IntersectionIterator i=g.recv_cell_local_local_begin();
           i!=g.recv_cell_local_local_end(); ++i)
      {
        s << "[" << rank << "]:    " << " r_c_l_l "
        << i.rank() << " " << *i << std::endl;
      }
      for (typename MultiGrid<d,ct>::IntersectionIterator i=g.send_cell_master_local_begin();
           i!=g.send_cell_master_local_end(); ++i)
      {
        s << "[" << rank << "]:    " << " s_c_m_l "
        << i.rank() << " " << *i << std::endl;
      }
      for (typename MultiGrid<d,ct>::IntersectionIterator i=g.recv_cell_local_master_begin();
           i!=g.recv_cell_local_master_end(); ++i)
      {
        s << "[" << rank << "]:    " << " r_c_l_m "
        << i.rank() << " " << *i << std::endl;
      }
    }

    s << std::endl;
    return s;
  }

  /** @} end documentation group */

}

#endif
