// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_YGRIDS_HH
#define DUNE_YGRIDS_HH

// C++ includes
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <deque>

// C includes
#if HAVE_MPI
#include <mpi.h>
#endif
#include <string.h>

// local includes
#include <dune/common/fvector.hh>
#include <dune/common/stdstreams.hh>

/*! \file grids.hh
   This is the basis for the yaspgrid implementation of the Dune grid interface.
 */

namespace Dune {

  // forward declarations
  template<int d, typename ct> class YGrid;
  template<int d, typename ct> class SubYGrid;

  static const double Ytolerance=1E-13;

  /**
     This is the basis of a parallel implementation of the dune grid interface
     supporting codim 0 and dim.

     You can also use the structured interface and write really fast code.

     The YGrid considered here describes a finite set \f$d\f$-tupels of the form
     \f[ G = \{ (k_0,\ldots,k_{d-1}) | o_i \leq k_i < o_i+s_i \}  \f]

     togehter with an affine mapping

     \f[ t : G \to R^d, \ \ \ t(k)_i = k_i h_i + r_i \f].

     Therefore a YGrid is characterized by the following four quantities:

     - The origin \f$ o=(o_0,\ldots,o_{d-1}) \in Z^d\f$,
     - the size \f$ s=(s_0,\ldots,s_{d-1}) \in Z^d\f$,
     - the mesh width \f$ h=(h_0,\ldots,h_{d-1}) \in R^d\f$,
     - The shift \f$ r=(r_0,\ldots,r_{d-1}) \in R^d\f$. The shift can be used to interpret the
     points of a grid as midpoints of cells, faces, edges, etc.

     The YGrid can be parametrized by the dimension d and the type to be used for the coordinates.

     Here is a graphical illustration of a grid:

     \image html  grid.png "A YGrid."
     \image latex grid.eps "A YGrid." width=\textwidth

     A grid can be manipulated either in the origin/size representation or in the
     min index / max index representation.

     A YGrid allows to iterate over all its cells with an Iterator class.
   */
  template<int d, typename ct>
  class YGrid {
  public:
    //! define types used for arguments
    typedef FieldVector<int, d>  iTupel;
    typedef FieldVector<ct, d> fTupel;
    typedef FieldVector<bool, d> bTupel;

    //! Destructor
    virtual ~YGrid()
    {}

    //! Make an empty YGrid with origin 0
    YGrid ()
    {
      _origin = 0;
      _size = 0;
      _h = 0.0;
      _r = 0.0;
    }

    //! Make YGrid from origin and size arrays
    YGrid (iTupel o, iTupel s, fTupel h, fTupel r)
    {
      for (int i=0; i<d; ++i)
      {
        _origin[i] = o[i];
        _size[i] = s[i];
        if (_size[i]<0) {
          _size[i] = 0;
        }
        _h[i] = h[i];
        _r[i] = r[i];
      }
    }

    //! Return origin in direction i
    int origin (int i) const
    {
      return _origin[i];
    }

    //! Set origin in direction i
    void origin (int i, int oi) const
    {
      _origin[i] = oi;
    }

    //! return reference to origin
    const iTupel& origin () const
    {
      return _origin;
    }

    //! Return size in direction i
    int size (int i) const
    {
      return _size[i];
    }

    //! Set size in direction i
    void size (int i, int si) const
    {
      _size[i] = si;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return reference to size tupel
    const iTupel& size () const
    {
      return _size;
    }

    //! Return total size of index set which is the product of all size per direction.
    int totalsize () const
    {
      int s=1;
      for (int i=0; i<d; ++i) s=s*_size[i];
      return s;
    }

    //! Return minimum index in direction i
    int min (int i) const
    {
      return _origin[i];
    }

    //! Set minimum index in direction i
    void min (int i, int mi) const
    {
      _size[i] = max(i)-mi+1;
      _origin[i] = mi;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return maximum index in direction i
    int max (int i) const
    {
      return _origin[i]+_size[i]-1;
    }

    //! Set maximum index in direction i
    void max (int i, int mi) const
    {
      _size[i] = mi-min(i)+1;
      if (_size[i]<0) _size[i] = 0;
    }

    //! Return reference to mesh size tupel for read write access
    const fTupel& meshsize () const
    {
      return _h;
    }

    //! Return mesh size in direction i
    ct meshsize (int i) const
    {
      return _h[i];
    }

    //! Set mesh size in direction i
    void meshsize (int i, int hi) const
    {
      _h[i] = hi;
    }

    //! Return shift tupel
    const fTupel& shift () const
    {
      return _r;
    }

    //! Return shift in direction i
    ct shift (int i) const
    {
      return _r[i];
    }

    //! Set shift in direction i
    void shift (int i, int ri) const
    {
      _r[i] = ri;
    }

    //! Return true if YGrid is empty, i.e. has size 0 in all directions.
    bool empty () const
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

    //! given a coordinate, return true if it is in the grid
    bool inside (const iTupel& coord) const
    {
      for (int i=0; i<d; i++)
      {
        if (coord[i]<_origin[i] || coord[i]>=_origin[i]+_size[i]) return false;
      }
      return true;
    }

    //! Return new SubYGrid of self which is the intersection of self and another YGrid
    virtual SubYGrid<d,ct> intersection ( YGrid<d,ct>& r) const
    {
      // check if the two grids can be intersected, must have same mesh size and shift
      for (int i=0; i<d; i++)
        if (fabs(meshsize(i)-r.meshsize(i))>Ytolerance) return SubYGrid<d,ct>();
      for (int i=0; i<d; i++)
        if (fabs(shift(i)-r.shift(i))>Ytolerance) return SubYGrid<d,ct>();

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
      return SubYGrid<d,ct>(neworigin,newsize,offset,_size,_h,_r);
    }

    //! return grid moved by the vector v
    YGrid<d,ct> move (iTupel v) const
    {
      for (int i=0; i<d; i++) v[i] += _origin[i];
      return YGrid<d,ct>(v,_size,_h,_r);
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
      Iterator (const YGrid<d,ct>& r)
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

      //! Make iterator pointing to given cell in a grid.
      Iterator (const YGrid<d,ct>& r, const iTupel& coord)
      {
        // copy data coming from grid to iterate over
        for (int i=0; i<d; ++i) _origin[i] = r.origin(i);
        for (int i=0; i<d; ++i) _end[i] = r.origin(i)+r.size(i)-1;

        // compute increments;
        int inc = 1;
        for (int i=0; i<d; ++i)
        {
          _increment[i] = inc;
          inc *= r.size(i);
        }

        // initialize to given position in index set
        for (int i=0; i<d; ++i) _coord[i] = coord[i];
        _index = r.index(coord);
      }

      //! reinitialize iterator to given position
      void reinit (const YGrid<d,ct>& r, const iTupel& coord)
      {
        // copy data coming from grid to iterate over
        for (int i=0; i<d; ++i) _origin[i] = r.origin(i);
        for (int i=0; i<d; ++i) _end[i] = r.origin(i)+r.size(i)-1;

        // compute increments;
        int inc = 1;
        for (int i=0; i<d; ++i)
        {
          _increment[i] = inc;
          inc *= r.size(i);
        }

        // initialize to given position in index set
        for (int i=0; i<d; ++i) _coord[i] = coord[i];
        _index = r.index(coord);
      }

      //! Return true when two iterators over the same grid are equal (!).
      bool operator== (const Iterator& i) const
      {
        return _index == i._index;
      }

      //! Return true when two iterators over the same grid are not equal (!).
      bool operator!= (const Iterator& i) const
      {
        return _index != i._index;
      }

      //! Return index of the current cell in the consecutive numbering.
      int index () const
      {
        return _index;
      }

      //! Return coordinate of the cell in direction i.
      int coord (int i) const
      {
        return _coord[i];
      }

      //! Return coordinate of the cell as reference (do not modify).
      const iTupel& coord () const
      {
        return _coord;
      }

      //! Get index of cell which is dist cells away in direction i.
      int neighbor (int i, int dist) const
      {
        return _index+dist*_increment[i];
      }

      //! Get index of neighboring cell which is -1 away in direction i.
      int down (int i) const
      {
        return _index-_increment[i];
      }

      //! Get index of neighboring cell which is +1 away in direction i.
      int up (int i) const
      {
        return _index+_increment[i];
      }

      //! move this iterator dist cells in direction i
      void move (int i, int dist)
      {
        _coord[i] += dist;
        _index += dist*_increment[i];
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
      void print (std::ostream& s) const
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
    Iterator begin () const { return Iterator(*this); }

    //! return iterator to one past the last element of index set
    Iterator end () const {
      iTupel last;
      for (int i=0; i<d; i++) last[i] = max(i);
      last[0] += 1;
      return Iterator(*this,last);
    }

    /*! TransformingIterator is an Iterator providing in addition a linear transformation
       of the coordinates of the grid in the form \f$ y_i = x_i h_i + s_i \f$.
       This can be used to interpret the grid cells as vertices, edges, faces, etc.
     */
    class TransformingIterator : public Iterator {
    public:
      //! Make iterator pointing to first cell in a grid.
      TransformingIterator (const YGrid<d,ct>& r) : Iterator(r)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = _begin[i];
      }

      //! Make iterator pointing to given cell in a grid.
      TransformingIterator (const YGrid<d,ct>& r, iTupel& coord) : Iterator(r,coord)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = coord[i]*r.meshsize(i)+r.shift(i);
      }

      //! Make transforming iterator from iterator (used for automatic conversion of end)
      TransformingIterator (Iterator i) : Iterator(i)
      {       }

      //! Increment iterator to next cell with position.
      TransformingIterator& operator++ ()
      {
        ++(this->_index);
        for (int i=0; i<d; i++)
          if (++(this->_coord[i])<=this->_end[i])
          {
            _position[i] += _h[i];
            return *this;
          }
          else
          {
            this->_coord[i]=this->_origin[i];
            _position[i] = _begin[i];
          }
        return *this;
      }

      //! Return position of current cell in direction i.
      ct position (int i) const
      {
        return _position[i];
      }

      //! Return position of current cell as reference.
      const fTupel& position () const
      {
        return _position;
      }

      //! Return meshsize in direction i
      ct meshsize (int i) const
      {
        return _h[i];
      }

      //! Return meshsize of current cell as reference.
      const fTupel& meshsize () const
      {
        return _h;
      }

      //! Move cell position by dist cells in direction i.
      void move (int i, int dist)
      {
        Iterator::move(i,dist);
        _position[i] += dist*_h[i];
      }

      //! Print contents of iterator
      void print (std::ostream& s) const
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
    TransformingIterator tbegin () const
    {
      return TransformingIterator(*this);
    }

    //! return iterator to one past the last element of the grid
    TransformingIterator tend () const
    {
      iTupel last;
      for (int i=0; i<d; i++) last = max(i);
      last[0] += 1;
      return TransformingIterator(*this,last);
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
  inline std::ostream& operator<< (std::ostream& s, YGrid<d,ct> e)
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

  //! Output operator for Iterators
  template <int d, typename ct>
  inline std::ostream& operator<< (std::ostream& s, typename YGrid<d,ct>::Iterator& e)
  {
    e.print(s);
    return s;
  }


  /*! A SubYGrid is a grid that is embedded in a larger grid
     It is characterized by an offset and an enclosing grid as
     shown in the following picture:

        \image html  subgrid.png "The SubYGrid is shown in red, blue is the enclosing grid."
        \image latex subgrid.eps "The SubYGrid is shown in red, blue is the enclosing grid." width=\textwidth

     SubYGrid has additional iterators that provide a mapping to
     the consecutive index in the enclosing grid.
   */
  template<int d, typename ct>
  class SubYGrid : public YGrid<d,ct> {
  public:
    typedef typename YGrid<d,ct>::iTupel iTupel;
    typedef typename YGrid<d,ct>::fTupel fTupel;
    typedef typename YGrid<d,ct>::bTupel bTupel;

    //! Destructor
    virtual ~SubYGrid()
    {}

    //! make uninitialized subgrid
    SubYGrid () {}

    //! Make SubYGrid from origin, size, offset and supersize
    SubYGrid (iTupel origin, iTupel size, iTupel offset, iTupel supersize, fTupel h, fTupel r)
      : YGrid<d,ct>::YGrid(origin,size,h,r)
    {
      for (int i=0; i<d; ++i)
      {
        _offset[i] = offset[i];
        _supersize[i] = supersize[i];
        if (offset[i]<0)
          std::cout << "warning: offset["
          << i <<"] negative in SubYGrid"
          << std::endl;
        if (-offset[i]+supersize[i]<size[i])
          std::cout << "warning: subgrid larger than enclosing grid in direction "
          << i <<" in SubYGrid"
          << std::endl;
      }
    }

    //! Make SubYGrid from YGrid
    SubYGrid (YGrid<d,ct> base) : YGrid<d,ct>(base)
    {
      for (int i=0; i<d; ++i)
      {
        _offset[i] = 0;
        _supersize[i] = this->size(i);
      }
    }

    //! Return offset to origin of enclosing grid
    int offset (int i) const
    {
      return _offset[i];
    }

    //! Return offset to origin of enclosing grid
    const iTupel & offset () const
    {
      return _offset;
    }

    //! return size of enclosing grid
    int supersize (int i) const
    {
      return _supersize[i];
    }

    //! return size of enclosing grid
    const iTupel & supersize () const
    {
      return _supersize;
    }

    //! Return SubYGrid of supergrid of self which is the intersection of self and another YGrid
    virtual SubYGrid<d,ct> intersection (const YGrid<d,ct>& r) const
    {
      // check if the two grids can be intersected, must have same mesh size and shift
      for (int i=0; i<d; i++)
        if (fabs(this->meshsize(i)-r.meshsize(i))>Ytolerance) return SubYGrid<d,ct>();
      for (int i=0; i<d; i++)
        if (fabs(this->shift(i)-r.shift(i))>Ytolerance) return SubYGrid<d,ct>();

      iTupel neworigin;
      iTupel newsize;
      iTupel offset;

      for (int i=0; i<d; ++i)
      {
        // intersect
        neworigin[i] = std::max(this->min(i),r.min(i));
        newsize[i] = std::min(this->max(i),r.max(i))-neworigin[i]+1;
        if (newsize[i]<0) {
          newsize[i] = 0;
          neworigin[i] = this->min(i);
        }

        // offset to my supergrid
        offset[i] = _offset[i]+neworigin[i]-this->origin(i);
      }
      return SubYGrid<d,ct>(neworigin,newsize,offset,_supersize,this->meshsize(),this->shift());
    }

    /*! SubIterator is an Iterator that provides in addition the consecutive
       index in the enclosing grid.
     */
    class SubIterator : public YGrid<d,ct>::Iterator {
    public:
      //! Make iterator pointing to first cell in subgrid.
      SubIterator (const SubYGrid<d,ct>& r) : YGrid<d,ct>::Iterator::Iterator (r)
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

      //! Make iterator pointing to given cell in subgrid.
      SubIterator (const SubYGrid<d,ct>& r, const iTupel& coord) : YGrid<d,ct>::Iterator::Iterator (r,coord)
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
          _superindex += (r.offset(i)+coord[i]-r.origin(i))*_superincrement[i];
      }

      //! Make transforming iterator from iterator (used for automatic conversion of end)
      SubIterator (const typename YGrid<d,ct>::Iterator& i) : YGrid<d,ct>::Iterator::Iterator(i)
      {}

      //! Make iterator pointing to given cell in subgrid.
      void reinit (const SubYGrid<d,ct>& r, const iTupel& coord)
      {
        YGrid<d,ct>::Iterator::reinit(r,coord);

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
          _superindex += (r.offset(i)+coord[i]-r.origin(i))*_superincrement[i];
      }

      //! Return consecutive index in enclosing grid
      int superindex () const
      {
        return _superindex;
      }

      //! Get index of cell which is dist cells away in direction i in enclosing grid.
      int superneighbor (int i, int dist) const
      {
        return _superindex+dist*_superincrement[i];
      }

      //! Get index of neighboring cell which is -1 away in direction i in enclosing grid.
      int superdown (int i) const
      {
        return _superindex-_superincrement[i];
      }

      //! Get index of neighboring cell which is +1 away in direction i in enclosing grid.
      int superup (int i) const
      {
        return _superindex+_superincrement[i];
      }

      //! move this iterator dist cells in direction i
      void move (int i, int dist)
      {
        YGrid<d,ct>::Iterator::move(i,dist);            // move base iterator
        _superindex += dist*_superincrement[i];         // move superindex
      }

      //! Increment iterator to next cell in subgrid
      SubIterator& operator++ ()
      {
        ++(this->_index);                       // update consecutive index in grid
        for (int i=0; i<d; i++)                 // check for wrap around
        {
          _superindex += _superincrement[i];               // move on cell in direction i
          if (++(this->_coord[i])<=this->_end[i])
            return *this;
          else
          {
            this->_coord[i]=this->_origin[i];                         // move back to origin in direction i
            _superindex -= _size[i]*_superincrement[i];
          }
        }
        return *this;
      }

      //! Print position of iterator
      void print (std::ostream& s) const
      {
        YGrid<d,ct>::Iterator::print(s);
        s << " super=" << superindex();
      }

    protected:
      int _superindex;            //!< consecutive index in enclosing grid
      iTupel _superincrement;     //!< moves consecutive index by one in this direction in supergrid
      iTupel _size;               //!< size of subgrid
    };

    //! return subiterator to first element of index set
    SubIterator subbegin () const { return SubIterator(*this); }

    //! return subiterator to last element of index set
    SubIterator subend () const
    {
      iTupel last;
      for (int i=0; i<d; i++) last[i] = this->max(i);
      last[0] += 1;
      return SubIterator(*this,last);
    }

    /*! TransformingSubIterator is a SubIterator providing in addition a linear transformation
       of the coordinates of the grid in the form \f$ y_i = x_i h_i + s_i \f$.
       This can be used to interpret the grid cells as vertices, edges, faces, etc.
     */
    class TransformingSubIterator : public SubIterator {
    public:
      //! Make iterator pointing to first cell in a grid.
      TransformingSubIterator (const SubYGrid<d,ct>& r) : SubIterator(r)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = _begin[i];
      }

      //! Make iterator pointing to given cell in a grid.
      TransformingSubIterator (const SubYGrid<d,ct>& r, const iTupel& coord) : SubIterator(r,coord)
      {
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = coord[i]*r.meshsize(i)+r.shift(i);
      }

      //! Make transforming iterator from iterator (used for automatic conversion of end)
      TransformingSubIterator (const SubIterator& i) :
        SubIterator(i)
      {}

      TransformingSubIterator (const TransformingSubIterator & t) :
        SubIterator(t), _h(t._h), _begin(t._begin), _position(t._position)
      {}

      //! Make iterator pointing to given cell in a grid.
      void reinit (const SubYGrid<d,ct>& r, const iTupel& coord)
      {
        SubIterator::reinit(r,coord);
        for (int i=0; i<d; ++i) _h[i] = r.meshsize(i);
        for (int i=0; i<d; ++i) _begin[i] = r.origin(i)*r.meshsize(i)+r.shift(i);
        for (int i=0; i<d; ++i) _position[i] = coord[i]*r.meshsize(i)+r.shift(i);
      }

      //! Increment iterator to next cell with position.
      TransformingSubIterator& operator++ ()
      {
        ++(this->_index);                               // update consecutive index in subgrid
        for (int i=0; i<d; i++)                 // check for wrap around
        {
          this->_superindex += this->_superincrement[i];               // move on cell in direction i
          if (++(this->_coord[i])<=this->_end[i])
          {
            _position[i] += _h[i];
            return *this;
          }
          else
          {
            this->_coord[i]=this->_origin[i];                         // move back to origin in direction i
            this->_superindex -= this->_size[i]*this->_superincrement[i];
            _position[i] = _begin[i];
          }
        }
        return *this;
      }

      //! Return position of current cell in direction i.
      ct position (int i) const
      {
        return _position[i];
      }

      //! Return position of current cell as reference.
      const fTupel& position () const
      {
        return _position;
      }

      //! Return meshsize in direction i
      ct meshsize (int i) const
      {
        return _h[i];
      }

      //! Return meshsize of current cell as reference.
      const fTupel& meshsize () const
      {
        return _h;
      }

      //! Move cell position by dist cells in direction i.
      void move (int i, int dist)
      {
        SubIterator::move(i,dist);
        _position[i] += dist*_h[i];
      }

      //! Print contents of iterator
      void print (std::ostream& s) const
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
    TransformingSubIterator tsubbegin () const
    {
      return TransformingSubIterator(*this);
    }

    //! return iterator to given element of index set
    TransformingSubIterator tsubbegin (iTupel& co) const
    {
      return TransformingSubIterator(*this,co);
    }

    //! return subiterator to last element of index set
    TransformingSubIterator tsubend () const
    {
      SubIterator endit = subend();
      return TransformingSubIterator(endit);
    }

  private:
    iTupel _offset;        //!< offset to origin of the enclosing grid
    iTupel _supersize;     //!< size of the enclosing grid
  };


  //! Output operator for subgrids
  template <int d, typename ct>
  inline std::ostream& operator<< (std::ostream& s, SubYGrid<d,ct> e)
  {
    YGrid<d,ct> x = e;
    s << x << " ofs=" << e.offset() << " ss=" << e.supersize();
    return s;
  }

  //! Output operator for subgrids
  template <int d, typename ct>
  inline std::ostream& operator<< (std::ostream& s, typename SubYGrid<d,ct>::TransformingSubIterator& e)
  {
    e.print(s);
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
    typedef FieldVector<int, d> iTupel;
    typedef FieldVector<bool, d> bTupel;


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
#if HAVE_MPI
      MPI_Request request;     // used by MPI to handle request
#else
      int request;
#endif
      int flag;          // used by MPI
    };

  public:
    //! constructor making uninitialized object
    Torus ()
    {  }

    //! make partitioner from communicator and coarse mesh size
#if HAVE_MPI
    Torus (MPI_Comm comm, int tag, iTupel size)
#else
    Torus (int tag, iTupel size)
#endif
    {
      // MPI stuff
#if HAVE_MPI
      _comm = comm;
      MPI_Comm_size(comm,&_procs);
      MPI_Comm_rank(comm,&_rank);
#else
      _procs=1; _rank=0;
#endif
      _tag = tag;

      // determine dimensions
      iTupel dims;
      double opt=1E100;
      optimize_dims(d-1,size,_procs,dims,opt);
      //          if (_rank==0) std::cout << "Torus<" << d
      //                                                          << ">: mapping " << _procs << " processes onto "
      //                                                          << _dims << " torus." << std::endl;

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
    int rank () const
    {
      return _rank;
    }

    //! return own coordinates
    iTupel coord () const
    {
      return rank_to_coord(_rank);
    }

    //! return number of processes
    int procs () const
    {
      return _procs;
    }

    //! return dimensions of torus
    const iTupel & dims () const
    {
      return _dims;
    }

    //! return dimensions of torus in direction i
    int dims (int i) const
    {
      return _dims[i];
    }

    //! return MPI communicator
#if HAVE_MPI
    MPI_Comm comm () const
    {
      return _comm;
    }
#endif

    //! return tag used by torus
    int tag () const
    {
      return _tag;
    }

    //! return true if coordinate is inside torus
    bool inside (iTupel c) const
    {
      for (int i=d-1; i>=0; i--)
        if (c[i]<0 || c[i]>=_dims[i]) return false;
      return true;
    }

    //! map rank to coordinate in torus using lexicographic ordering
    iTupel rank_to_coord (int rank) const
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
    int coord_to_rank (iTupel coord) const
    {
      for (int i=0; i<d; i++) coord[i] = coord[i]%_dims[i];
      int rank = 0;
      for (int i=0; i<d; i++) rank += coord[i]*_increment[i];
      return rank;
    }

    //! return rank of process where its coordinate in direction dir has offset cnt (handles periodic case)
    int rank_relative (int rank, int dir, int cnt) const
    {
      iTupel coord = rank_to_coord(rank);
      coord[dir] = (coord[dir]+dims[dir]+cnt)%dims[dir];
      return coord_to_rank(coord);
    }

    //! assign color to given coordinate
    int color (const iTupel & coord) const
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
    int color (int rank) const
    {
      return color(rank_to_coord(rank));
    }

    //! return the number of neighbors, which is \f$3^d-1\f$
    int neighbors () const
    {
      int n=1;
      for (int i=0; i<d; ++i)
        n *= 3;
      return n-1;
    }

    //! return true if neighbor with given delta is a neighbor under the given periodicity
    bool is_neighbor (iTupel delta, bTupel periodic) const
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
    double partition (int rank, iTupel origin_in, iTupel size_in, iTupel& origin_out, iTupel& size_out) const
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
      ProcListIterator (typename std::deque<CommPartner>::const_iterator iter)
      {
        i = iter;
      }

      //! return rank of neighboring process
      int rank () const
      {
        return i->rank;
      }

      //! return distance vector
      iTupel delta () const
      {
        return i->delta;
      }

      //! return index in proclist
      int index () const
      {
        return i->index;
      }

      //! return 1-norm of distance vector
      int distance () const
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
      typename std::deque<CommPartner>::const_iterator i;
    };

    //! first process in send list
    ProcListIterator sendbegin () const
    {
      return ProcListIterator(_sendlist.begin());
    }

    //! end of send list
    ProcListIterator sendend () const
    {
      return ProcListIterator(_sendlist.end());
    }

    //! first process in receive list
    ProcListIterator recvbegin () const
    {
      return ProcListIterator(_recvlist.begin());
    }

    //! last process in receive list
    ProcListIterator recvend () const
    {
      return ProcListIterator(_recvlist.end());
    }

    //! store a send request; buffers are sent in order; handles also local requests with memcpy
    void send (int rank, void* buffer, int size) const
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
    void recv (int rank, void* buffer, int size) const
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
    void exchange () const
    {
      // handle local requests first
      if (_localsendrequests.size()!=_localrecvrequests.size())
      {
        std::cout << "[" << rank() << "]: ERROR: local sends/receives do not match in exchange!" << std::endl;
        return;
      }
      for (unsigned int i=0; i<_localsendrequests.size(); i++)
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

#if HAVE_MPI
      // handle foreign requests
      int sends=0;
      int recvs=0;

      // issue sends to foreign processes
      for (unsigned int i=0; i<_sendrequests.size(); i++)
        if (_sendrequests[i].rank!=rank())
        {
          MPI_Isend(_sendrequests[i].buffer, _sendrequests[i].size, MPI_BYTE,
                    _sendrequests[i].rank, _tag, _comm, &(_sendrequests[i].request));
          _sendrequests[i].flag = false;
          sends++;
        }

      // issue receives from foreign processes
      for (unsigned int i=0; i<_recvrequests.size(); i++)
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
        for (unsigned int i=0; i<_sendrequests.size(); i++)
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
        for (unsigned int i=0; i<_recvrequests.size(); i++)
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
#endif
    }

    //! global sum
    double global_sum (double x) const
    {
      double res;

      if (_procs==1) return x;
#if HAVE_MPI
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_SUM,_comm);
      return res;
#endif
    }

    //! global max
    double global_max (double x) const
    {
      double res;

      if (_procs==1) return x;
#if HAVE_MPI
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_MAX,_comm);
      return res;
#endif
    }

    //! global min
    double global_min (double x) const
    {
      double res;

      if (_procs==1) return x;
#if HAVE_MPI
      MPI_Allreduce(&x,&res,1,MPI_DOUBLE,MPI_MIN,_comm);
      return res;
#endif
    }


    //! print contents of torus object
    void print (std::ostream& s) const
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
        {
          double mm=((double)size[k])/((double)dims[k]);
          if (fmod((double)size[k],(double)dims[k])>0.0001) mm*=3;
          if ( mm > m ) m = mm;
        }
        if (_rank==0) std::cout << "optimize_dims: " << size << "|" << dims << " norm=" << m << std::endl;
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

#if HAVE_MPI
    MPI_Comm _comm;
#endif
    int _rank;
    int _procs;
    iTupel _dims;
    iTupel _increment;
    int _tag;
    std::deque<CommPartner> _sendlist;
    std::deque<CommPartner> _recvlist;

    mutable std::vector<CommTask> _sendrequests;
    mutable std::vector<CommTask> _recvrequests;
    mutable std::vector<CommTask> _localsendrequests;
    mutable std::vector<CommTask> _localrecvrequests;
  };

  //! Output operator for Torus
  template <int d>
  inline std::ostream& operator<< (std::ostream& s, const Torus<d> & t)
  {
    t.print(s);
    return s;
  }


  /*! MultiYGrid manages a d-dimensional grid mapped to a set of processes.
   */
  template<int d, typename ct>
  class MultiYGrid {
  public:
    // some data types
    struct Intersection {
      SubYGrid<d,ct> grid;     // the intersection as a subgrid of local grid
      int rank;                // rank of process where other grid is stored
      int distance;            // manhattan distance to other grid
    };

    struct YGridLevel {            // This stores all the information on one grid level
      // cell (codim 0) data
      YGrid<d,ct> cell_global;             // the whole cell grid on that level
      SubYGrid<d,ct> cell_overlap;         // we have no ghost cells, so our part is overlap completely
      SubYGrid<d,ct> cell_interior;        // interior cells are a subgrid of all cells

      std::deque<Intersection> send_cell_overlap_overlap;      // each intersection is a subgrid of overlap
      std::deque<Intersection> recv_cell_overlap_overlap;      // each intersection is a subgrid of overlap

      std::deque<Intersection> send_cell_interior_overlap; // each intersection is a subgrid of overlap
      std::deque<Intersection> recv_cell_overlap_interior; // each intersection is a subgrid of overlap

      // vertex (codim dim) data
      YGrid<d,ct> vertex_global;           // the whole vertex grid on that level
      SubYGrid<d,ct> vertex_overlapfront;  // all our vertices are overlap and front
      SubYGrid<d,ct> vertex_overlap;       // subgrid containing only overlap
      SubYGrid<d,ct> vertex_interiorborder; // subgrid containing only interior and border
      SubYGrid<d,ct> vertex_interior;      // subgrid containing only interior

      std::deque<Intersection> send_vertex_overlapfront_overlapfront; // each intersection is a subgrid of overlapfront
      std::deque<Intersection> recv_vertex_overlapfront_overlapfront; // each intersection is a subgrid of overlapfront

      std::deque<Intersection> send_vertex_overlap_overlapfront; // each intersection is a subgrid of overlapfront
      std::deque<Intersection> recv_vertex_overlapfront_overlap; // each intersection is a subgrid of overlapfront

      std::deque<Intersection> send_vertex_interiorborder_interiorborder;     // each intersection is a subgrid of overlapfront
      std::deque<Intersection> recv_vertex_interiorborder_interiorborder;     // each intersection is a subgrid of overlapfront

      std::deque<Intersection> send_vertex_interiorborder_overlapfront;     // each intersection is a subgrid of overlapfront
      std::deque<Intersection> recv_vertex_overlapfront_interiorborder;     // each intersection is a subgrid of overlapfront

      // general
      MultiYGrid<d,ct>* mg;  // each grid level knows its multigrid
      int overlap;           // in mesh cells on this level
    };

    //! define types used for arguments
    typedef FieldVector<int, d> iTupel;
    typedef FieldVector<ct, d> fTupel;
    typedef FieldVector<bool, d> bTupel;

    // communication tag used by multigrid
    enum { tag = 17 };

    //! constructor making a grid
#if HAVE_MPI
    MultiYGrid (MPI_Comm comm, fTupel L, iTupel s, bTupel periodic, int overlap)
      : _torus(comm,tag,s)     // torus gets s to compute procs/direction
    {
      // store parameters
      _LL = L;
      _s = s;
      _periodic = periodic;
      _overlap = overlap;

      // coarse cell interior  grid obtained through partitioning of global grid
      iTupel o_interior;
      iTupel s_interior;
      iTupel o = iTupel(0);
      double imbal = _torus.partition(_torus.rank(),o,s,o_interior,s_interior);
      imbal = _torus.global_max(imbal);

      // add level
      _maxlevel = 0;
      _levels[_maxlevel] = makelevel(L,s,periodic,o_interior,s_interior,overlap);

      // output
      //          if (_torus.rank()==0) std::cout << "MultiYGrid<" << d // changed dinfo to cout
      //                                                                          << ">: coarse grid with size " << s
      //                                                                          << " imbalance=" << (imbal-1)*100 << "%" << std::endl;
      //      print(std::cout);
    }
#else
    MultiYGrid (fTupel L, iTupel s, bTupel periodic, int overlap)
      : _torus(tag,s)     // torus gets s to compute procs/direction
    {
      // store parameters
      _LL = L;
      _s = s;
      _periodic = periodic;
      _overlap = overlap;

      // coarse cell interior  grid obtained through partitioning of global grid
      iTupel o = iTupel(0);
      iTupel o_interior(o);
      iTupel s_interior(s);

      // add level
      _maxlevel = 0;
      _levels[_maxlevel] = makelevel(L,s,periodic,o_interior,s_interior,overlap);
      // output
      //          if (_torus.rank()==0) std::cout << "MultiYGrid<" << d // changed dinfo to cout
      //                                                                          << ">: coarse grid with size " << s
      //                                                                          << " imbalance=" << (imbal-1)*100 << "%" << std::endl;
      //      print(std::cout);
    }
#endif

    //! do a global mesh refinement; true: keep overlap in absolute size; false: keep overlap in mesh cells
    void refine (bool keep_overlap)
    {
      // access to coarser grid level
      YGridLevel& cg = _levels[maxlevel()];

      // compute size of new global grid
      iTupel s;
      for (int i=0; i<d; i++) s[i] = 2*cg.cell_global.size(i);

      // compute overlap
      int overlap;
      if (keep_overlap) overlap = 2*cg.overlap;else overlap = cg.overlap;

      // output
      //          if (_torus.rank()==0) std::cout << "MultiYGrid<" // changed dinfo to cout
      //                                                                          << d << ">: refined to size "
      //                                                                          << s << std::endl;

      // the cell interior grid obtained from coarse cell interior grid
      iTupel o_interior;
      iTupel s_interior;
      for (int i=0; i<d; i++) o_interior[i] = 2*cg.cell_interior.origin(i);
      for (int i=0; i<d; i++) s_interior[i] = 2*cg.cell_interior.size(i);

      // add level
      _maxlevel++;
      _levels[_maxlevel] = makelevel(_LL,s,_periodic,o_interior,s_interior,overlap);
    }

    //! return reference to torus
    const Torus<d>& torus () const
    {
      return _torus;
    }

    //! return the maximum level index (number of levels is maxlevel()+1)
    int maxlevel () const
    {
      return _maxlevel;
    }

    //! return true if grid is periodic in given direction
    bool periodic (int i) const
    {
      return _periodic[i];
    }

    //! provides access to a given grid level
    class YGridLevelIterator {
    private:
      int l;
      const YGridLevel* i;
    public:
      //! empty constructor, use with care
      YGridLevelIterator ()
      {}

      //! make iterator pointing to level k (no check made)
      YGridLevelIterator (const YGridLevel* start, int level)
      {
        i=start; l=level;
      }

      //! make iterator pointing to level k (no check made)
      YGridLevelIterator (const YGridLevelIterator & it)
        : l(it.l), i(it.i)
      {}

      //! return number of this grid level
      int level () const
      {
        return l;
      }

      //! return size of overlap on this level
      int overlap () const
      {
        return i->overlap;
      }

      //! return pointer to multigrid object that contains this level
      const MultiYGrid<d,ct>* mg () const
      {
        return i->mg;
      }

      //! Return true when two iterators point to same member
      bool operator== (const YGridLevelIterator& iter) const
      {
        return i == iter.i;
      }

      //! Return true when two iterators do not point to same member
      bool operator!= (const YGridLevelIterator& iter) const
      {
        return i != iter.i;
      }

      //! Increment iterator to next finer grid level
      YGridLevelIterator& operator++ ()
      {
        ++i;         // assumes built-in array
        ++l;
        return *this;
      }

      //! Increment iterator to coarser grid level
      YGridLevelIterator& operator-- ()
      {
        --i;
        --l;
        return *this;
      }

      //! get iterator to next finer grid level
      YGridLevelIterator finer () const
      {
        return YGridLevelIterator(i+1,l+1);
      }

      //! get iterator to next coarser grid level
      YGridLevelIterator coarser () const
      {
        return YGridLevelIterator(i-1,l-1);
      }

      //! reference to global cell grid
      const YGrid<d,ct>& cell_global () const
      {
        return i->cell_global;
      }

      //! reference to local cell grid which is a subgrid of the global cell grid
      const SubYGrid<d,ct>& cell_overlap () const
      {
        return i->cell_overlap;
      }

      //! reference to cell master grid which is a subgrid of the local cell grid
      const SubYGrid<d,ct>& cell_interior () const
      {
        return i->cell_interior;
      }


      //! access to intersection lists
      const std::deque<Intersection>& send_cell_overlap_overlap () const
      {
        return i->send_cell_overlap_overlap;
      }
      const std::deque<Intersection>& recv_cell_overlap_overlap () const
      {
        return i->recv_cell_overlap_overlap;
      }
      const std::deque<Intersection>& send_cell_interior_overlap () const
      {
        return i->send_cell_interior_overlap;
      }
      const std::deque<Intersection>& recv_cell_overlap_interior () const
      {
        return i->recv_cell_overlap_interior;
      }


      //! reference to global vertex grid
      const YGrid<d,ct>& vertex_global () const
      {
        return i->vertex_global;
      }
      //! reference to vertex grid, up to front; there are no ghosts in this implementation
      const SubYGrid<d,ct>& vertex_overlapfront () const
      {
        return i->vertex_overlapfront;
      }
      //! reference to overlap vertex grid; is subgrid of overlapfront vertex grid
      const SubYGrid<d,ct>& vertex_overlap () const
      {
        return i->vertex_overlap;
      }
      //! reference to interiorborder vertex grid; is subgrid of overlapfront vertex grid
      const SubYGrid<d,ct>& vertex_interiorborder () const
      {
        return i->vertex_interiorborder;
      }
      //! reference to interior vertex grid; is subgrid of overlapfront vertex grid
      const SubYGrid<d,ct>& vertex_interior () const
      {
        return i->vertex_interior;
      }

      //! access to intersection lists
      const std::deque<Intersection>& send_vertex_overlapfront_overlapfront () const
      {
        return i->send_vertex_overlapfront_overlapfront;
      }
      const std::deque<Intersection>& recv_vertex_overlapfront_overlapfront () const
      {
        return i->recv_vertex_overlapfront_overlapfront;
      }
      const std::deque<Intersection>& send_vertex_overlap_overlapfront () const
      {
        return i->send_vertex_overlap_overlapfront;
      }
      const std::deque<Intersection>& recv_vertex_overlapfront_overlap () const
      {
        return i->recv_vertex_overlapfront_overlap;
      }
      const std::deque<Intersection>& send_vertex_interiorborder_interiorborder () const
      {
        return i->send_vertex_interiorborder_interiorborder;
      }
      const std::deque<Intersection>& recv_vertex_interiorborder_interiorborder () const
      {
        return i->recv_vertex_interiorborder_interiorborder;
      }
      const std::deque<Intersection>& send_vertex_interiorborder_overlapfront () const
      {
        return i->send_vertex_interiorborder_overlapfront;
      }
      const std::deque<Intersection>& recv_vertex_overlapfront_interiorborder () const
      {
        return i->recv_vertex_overlapfront_interiorborder;
      }
    };

    //! return iterator pointing to coarsest level
    YGridLevelIterator begin () const
    {
      return YGridLevelIterator(_levels,0);
    }

    //! return iterator pointing to given level
    YGridLevelIterator begin (int i) const
    {
      if (i<0 << i>maxlevel())
        DUNE_THROW(GridError, "level not existing");
      return YGridLevelIterator(_levels+i,i);
    }

    //! return iterator pointing to one past the finest level
    YGridLevelIterator end () const
    {
      return YGridLevelIterator(_levels+(_maxlevel+1),_maxlevel+1);
    }

    //! return iterator pointing to the finest level
    YGridLevelIterator rbegin () const
    {
      return YGridLevelIterator(_levels+_maxlevel,_maxlevel);
    }

    //! return iterator pointing to one before the coarsest level
    YGridLevelIterator rend () const
    {
      return YGridLevelIterator(_levels-1,-1);
    }

    //! print function for multigrids
    inline void print (std::ostream& s) const
    {
      int rank = torus().rank();

      s << "[" << rank << "]:" << " MultiYGrid maxlevel=" << maxlevel() << std::endl;

      for (YGridLevelIterator g=begin(); g!=end(); ++g)
      {
        s << "[" << rank << "]:   " << std::endl;
        s << "[" << rank << "]:   " << "==========================================" << std::endl;
        s << "[" << rank << "]:   " << "level=" << g.level() << std::endl;
        s << "[" << rank << "]:   " << "cell_global=" << g.cell_global() << std::endl;
        s << "[" << rank << "]:   " << "cell_overlap=" << g.cell_overlap() << std::endl;
        s << "[" << rank << "]:   " << "cell_interior=" << g.cell_interior() << std::endl;
        for (typename std::deque<Intersection>::const_iterator i=g.send_cell_overlap_overlap().begin();
             i!=g.send_cell_overlap_overlap().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_c_o_o "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_cell_overlap_overlap().begin();
             i!=g.recv_cell_overlap_overlap().end(); ++i)
        {
          s << "[" << rank << "]:    " << " r_c_o_o "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.send_cell_interior_overlap().begin();
             i!=g.send_cell_interior_overlap().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_c_i_o "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_cell_overlap_interior().begin();
             i!=g.recv_cell_overlap_interior().end(); ++i)
        {
          s << "[" << rank << "]:    " << " r_c_o_i "
          << i->rank << " " << i->grid << std::endl;
        }

        s << "[" << rank << "]:   " << "-----------------------------------------------"  << std::endl;
        s << "[" << rank << "]:   " << "vertex_global="         << g.vertex_global() << std::endl;
        s << "[" << rank << "]:   " << "vertex_overlapfront="   << g.vertex_overlapfront() << std::endl;
        s << "[" << rank << "]:   " << "vertex_overlap="        << g.vertex_overlap() << std::endl;
        s << "[" << rank << "]:   " << "vertex_interiorborder=" << g.vertex_interiorborder() << std::endl;
        s << "[" << rank << "]:   " << "vertex_interior="       << g.vertex_interior() << std::endl;
        for (typename std::deque<Intersection>::const_iterator i=g.send_vertex_overlapfront_overlapfront().begin();
             i!=g.send_vertex_overlapfront_overlapfront().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_v_of_of "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_vertex_overlapfront_overlapfront().begin();
             i!=g.recv_vertex_overlapfront_overlapfront().end(); ++i)
        {
          s << "[" << rank << "]:    " << " r_v_of_of "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.send_vertex_overlap_overlapfront().begin();
             i!=g.send_vertex_overlap_overlapfront().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_v_o_of "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_vertex_overlapfront_overlap().begin();
             i!=g.recv_vertex_overlapfront_overlap().end(); ++i)
        {
          s << "[" << rank << "]:    " << " r_v_of_o "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.send_vertex_interiorborder_interiorborder().begin();
             i!=g.send_vertex_interiorborder_interiorborder().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_v_ib_ib "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_vertex_interiorborder_interiorborder().begin();
             i!=g.recv_vertex_interiorborder_interiorborder().end(); ++i)
        {
          s << "[" << rank << "]:    " << " r_v_ib_ib "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.send_vertex_interiorborder_overlapfront().begin();
             i!=g.send_vertex_interiorborder_overlapfront().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_v_ib_of "
          << i->rank << " " << i->grid << std::endl;
        }
        for (typename std::deque<Intersection>::const_iterator i=g.recv_vertex_overlapfront_interiorborder().begin();
             i!=g.recv_vertex_overlapfront_interiorborder().end(); ++i)
        {
          s << "[" << rank << "]:    " << " s_v_of_ib "
          << i->rank << " " << i->grid << std::endl;
        }
      }

      s << std::endl;
    }

  private:
    // make a new YGridLevel structure. For that we need
    // L           size of the whole domain in each direction
    // s           number of cells in each direction
    // periodic    boolean indication periodicity in each direction
    // o_interior  origin of interior (non-overlapping) cell decomposition
    // s_interior  size of interior cell decomposition
    // overlap     to be used on this grid level
    YGridLevel makelevel (fTupel L, iTupel s, bTupel periodic, iTupel o_interior, iTupel s_interior, int overlap)
    {
      // first, lets allocate a new structure
      YGridLevel g;
      g.overlap = overlap;
      g.mg = this;

      // the global cell grid
      iTupel o = iTupel(0);     // logical origin is always 0, that is not a restriction
      fTupel h;
      fTupel r;
      for (int i=0; i<d; i++) h[i] = L[i]/s[i];     // the mesh size in each direction
      for (int i=0; i<d; i++) r[i] = 0.5*h[i];      // the shift for cell centers
      g.cell_global = YGrid<d,ct>(o,s,h,r);         // this is the global cell grid

      // extend the cell interior grid by overlap considering periodicity
      iTupel o_overlap;
      iTupel s_overlap;
      for (int i=0; i<d; i++)
      {
        if (periodic[i])
        {
          // easy case, extend by 2 overlaps in total
          o_overlap[i] = o_interior[i]-overlap;                  // Note: origin might be negative now
          s_overlap[i] = s_interior[i]+2*overlap;                // Note: might be larger than global size
        }
        else
        {
          // nonperiodic case, intersect with global size
          int min = std::max(0,o_interior[i]-overlap);
          int max = std::min(s[i]-1,o_interior[i]+s_interior[i]-1+overlap);
          o_overlap[i] = min;
          s_overlap[i] = max-min+1;
        }
      }
      g.cell_overlap = SubYGrid<d,ct>(YGrid<d,ct>(o_overlap,s_overlap,h,r));

      // now make the interior grid a subgrid of the overlapping grid
      iTupel offset;
      for (int i=0; i<d; i++) offset[i] = o_interior[i]-o_overlap[i];
      g.cell_interior = SubYGrid<d,ct>(o_interior,s_interior,offset,s_overlap,h,r);

      // compute cell intersections
      intersections(g.cell_overlap,g.cell_overlap,g.cell_global.size(),g.send_cell_overlap_overlap,g.recv_cell_overlap_overlap);
      intersections(g.cell_interior,g.cell_overlap,g.cell_global.size(),g.send_cell_interior_overlap,g.recv_cell_overlap_interior);

      // now we can do the vertex grids. They are derived completely from the cell grids
      iTupel o_vertex_global, s_vertex_global;
      for (int i=0; i<d; i++) r[i] = 0.0;      // the shift for vertices is zero, and the mesh size is same as for cells

      // first let's make the global grid
      for (int i=0; i<d; i++) o_vertex_global[i] = g.cell_global.origin(i);
      for (int i=0; i<d; i++) s_vertex_global[i] = g.cell_global.size(i)+1;     // one more vertices than cells ...
      g.vertex_global = YGrid<d,ct>(o_vertex_global,s_vertex_global,h,r);

      // now the local grid stored in this processor. All other grids are subgrids of this
      iTupel o_vertex_overlapfront;
      iTupel s_vertex_overlapfront;
      for (int i=0; i<d; i++) o_vertex_overlapfront[i] = g.cell_overlap.origin(i);
      for (int i=0; i<d; i++) s_vertex_overlapfront[i] = g.cell_overlap.size(i)+1;     // one more vertices than cells ...
      g.vertex_overlapfront = SubYGrid<d,ct>(YGrid<d,ct>(o_vertex_overlapfront,s_vertex_overlapfront,h,r));

      // now overlap only (i.e. without front), is subgrid of overlapfront
      iTupel o_vertex_overlap;
      iTupel s_vertex_overlap;
      for (int i=0; i<d; i++)
      {
        o_vertex_overlap[i] = g.cell_overlap.origin(i);
        s_vertex_overlap[i] = g.cell_overlap.size(i)+1;

        if (!periodic[i] && g.cell_overlap.origin(i)>g.cell_global.origin(i))
        {
          // not at the lower boundary
          o_vertex_overlap[i] += 1;
          s_vertex_overlap[i] -= 1;
        }

        if (!periodic[i] && g.cell_overlap.origin(i)+g.cell_overlap.size(i)<g.cell_global.origin(i)+g.cell_global.size(i))
        {
          // not at the upper boundary
          s_vertex_overlap[i] -= 1;
        }


        offset[i] = o_vertex_overlap[i]-o_vertex_overlapfront[i];
      }
      g.vertex_overlap = SubYGrid<d,ct>(o_vertex_overlap,s_vertex_overlap,offset,s_vertex_overlapfront,h,r);

      // now interior with border
      iTupel o_vertex_interiorborder;
      iTupel s_vertex_interiorborder;
      for (int i=0; i<d; i++) o_vertex_interiorborder[i] = g.cell_interior.origin(i);
      for (int i=0; i<d; i++) s_vertex_interiorborder[i] = g.cell_interior.size(i)+1;
      for (int i=0; i<d; i++) offset[i] = o_vertex_interiorborder[i]-o_vertex_overlapfront[i];
      g.vertex_interiorborder = SubYGrid<d,ct>(o_vertex_interiorborder,s_vertex_interiorborder,offset,s_vertex_overlapfront,h,r);

      // now only interior
      iTupel o_vertex_interior;
      iTupel s_vertex_interior;
      for (int i=0; i<d; i++)
      {
        o_vertex_interior[i] = g.cell_interior.origin(i);
        s_vertex_interior[i] = g.cell_interior.size(i)+1;

        if (!periodic[i] && g.cell_interior.origin(i)>g.cell_global.origin(i))
        {
          // not at the lower boundary
          o_vertex_interior[i] += 1;
          s_vertex_interior[i] -= 1;
        }

        if (!periodic[i] && g.cell_interior.origin(i)+g.cell_interior.size(i)<g.cell_global.origin(i)+g.cell_global.size(i))
        {
          // not at the upper boundary
          s_vertex_interior[i] -= 1;
        }

        offset[i] = o_vertex_interior[i]-o_vertex_overlapfront[i];
      }
      g.vertex_interior = SubYGrid<d,ct>(o_vertex_interior,s_vertex_interior,offset,s_vertex_overlapfront,h,r);

      // compute vertex intersections
      intersections(g.vertex_overlapfront,g.vertex_overlapfront,g.cell_global.size(),
                    g.send_vertex_overlapfront_overlapfront,g.recv_vertex_overlapfront_overlapfront);
      intersections(g.vertex_overlap,g.vertex_overlapfront,g.cell_global.size(),
                    g.send_vertex_overlap_overlapfront,g.recv_vertex_overlapfront_overlap);
      intersections(g.vertex_interiorborder,g.vertex_interiorborder,g.cell_global.size(),
                    g.send_vertex_interiorborder_interiorborder,g.recv_vertex_interiorborder_interiorborder);
      intersections(g.vertex_interiorborder,g.vertex_overlapfront,g.cell_global.size(),
                    g.send_vertex_interiorborder_overlapfront,g.recv_vertex_overlapfront_interiorborder);

      // return the whole thing
      return g;
    }


    // construct list of intersections with neighboring processors:
    //   recvgrid: the grid stored in this processor
    //   sendgrid:  the subgrid to be sent to neighboring processors
    //   size: needed to shift local grid in periodic case
    //   returns two lists: Intersections to be sent and Intersections to be received
    // Note: sendgrid/recvgrid may be SubYGrids. Since intersection method is virtual it should work properly
    void intersections (const SubYGrid<d,ct>& sendgrid, const SubYGrid<d,ct>& recvgrid, const iTupel& size,
                        std::deque<Intersection>& sendlist, std::deque<Intersection>& recvlist)
    {
      // the exchange buffers
      std::vector<YGrid<d,ct> > send_recvgrid(_torus.neighbors());
      std::vector<YGrid<d,ct> > recv_recvgrid(_torus.neighbors());
      std::vector<YGrid<d,ct> > send_sendgrid(_torus.neighbors());
      std::vector<YGrid<d,ct> > recv_sendgrid(_torus.neighbors());

      // fill send buffers; iterate over all neighboring processes
      // non-periodic case is handled automatically because intersection will be zero
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
      {
        // determine if we communicate with this neighbor (and what)
        bool skip = false;
        iTupel coord = _torus.coord();           // my coordinates
        iTupel delta = i.delta();                // delta to neighbor
        iTupel nb = coord;                       // the neighbor
        for (int k=0; k<d; k++) nb[k] += delta[k];
        iTupel v = iTupel(0);                            // grid movement

        for (int k=0; k<d; k++)
        {
          if (nb[k]<0)
          {
            if (_periodic[k])
              v[k] += size[k];
            else
              skip = true;
          }
          if (nb[k]>=_torus.dims(k))
          {
            if (_periodic[k])
              v[k] -= size[k];
            else
              skip = true;
          }
          // neither might be true, then v=0
        }

        // store moved grids in send buffers
        if (!skip)
        {
          send_sendgrid[i.index()] = sendgrid.move(v);
          send_recvgrid[i.index()] = recvgrid.move(v);
        }
        else
        {
          send_sendgrid[i.index()] = YGrid<d,ct>(iTupel(0),iTupel(0),fTupel(0.0),fTupel(0.0));
          send_recvgrid[i.index()] = YGrid<d,ct>(iTupel(0),iTupel(0),fTupel(0.0),fTupel(0.0));
        }
      }

      // issue send requests for sendgrid being sent to all neighbors
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
        _torus.send(i.rank(), &send_sendgrid[i.index()], sizeof(YGrid<d,ct>));

      // issue recv requests for sendgrids of neighbors
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
        _torus.recv(i.rank(), &recv_sendgrid[i.index()], sizeof(YGrid<d,ct>));

      // exchange the sendgrids
      _torus.exchange();

      // issue send requests for recvgrid being sent to all neighbors
      for (typename Torus<d>::ProcListIterator i=_torus.sendbegin(); i!=_torus.sendend(); ++i)
        _torus.send(i.rank(), &send_recvgrid[i.index()], sizeof(YGrid<d,ct>));

      // issue recv requests for recvgrid of neighbors
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
        _torus.recv(i.rank(), &recv_recvgrid[i.index()], sizeof(YGrid<d,ct>));

      // exchange the recvgrid
      _torus.exchange();

      // process receive buffers and compute intersections
      for (typename Torus<d>::ProcListIterator i=_torus.recvbegin(); i!=_torus.recvend(); ++i)
      {
        // what must be sent to this neighbor
        Intersection send_intersection;
        send_intersection.grid = sendgrid.intersection(recv_recvgrid[i.index()]);
        //std::cout << "[" << _torus.rank() << "]:   " << "sendgrid=" << sendgrid << std::endl;
        //std::cout << "[" << _torus.rank() << "]:   " << "recved recvgrid=" << recv_recvgrid[i.index()] << std::endl;
        //std::cout << "[" << _torus.rank() << "]:   " << "intersection=" << send_intersection.grid << std::endl;
        send_intersection.rank = i.rank();
        send_intersection.distance = i.distance();
        if (!send_intersection.grid.empty()) sendlist.push_front(send_intersection);

        Intersection recv_intersection;
        recv_intersection.grid = recvgrid.intersection(recv_sendgrid[i.index()]);
        //std::cout << "[" << _torus.rank() << "]:   " << "recvgrid=" << recvgrid << std::endl;
        //std::cout << "[" << _torus.rank() << "]:   " << "recved sendgrid=" << recv_sendgrid[i.index()] << std::endl;
        //std::cout << "[" << _torus.rank() << "]:   " << "intersection=" << recv_intersection.grid << std::endl;
        recv_intersection.rank = i.rank();
        recv_intersection.distance = i.distance();
        if(!recv_intersection.grid.empty()) recvlist.push_back(recv_intersection);
      }
    }

    // private data of multigrid
    fTupel _LL;
    iTupel _s;
    bTupel _periodic;
    int _maxlevel;
    YGridLevel _levels[32];
    int _overlap;
    Torus<d> _torus;
  };

  //! Output operator for multigrids
  template <int d, class ct>
  inline std::ostream& operator<< (std::ostream& s, MultiYGrid<d,ct>& mg)
  {
    mg.print(s);
    s << std::endl;
    return s;
  }

} // namespace Dune

#endif
