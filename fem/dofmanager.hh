// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFMANAGER_HH__
#define __DUNE_DOFMANAGER_HH__

#include <assert.h>

#include <dune/common/dlist.hh>
#include <dune/common/stdstreams.hh>

#include <dune/fem/common/dofmapperinterface.hh>

// here are the default grid index set defined
#include <dune/grid/common/defaultindexsets.hh>

namespace Dune {

  // forward declaration
  template <class GridType,
      class IndexSetImp>
  class DofManager;

  // forward declaration
  class MemObject;

  // type of pointer to memory, for easy replacements
  typedef char MemPointerType;

  //********************************************************

  /*!
     DofArrayMemory holds the memory for one discrete function.
     It consits of a pointer to memory and size. For each DofArrayMemory the
     DofManager hold a MemObject. If the grid to which the DofManager belongs
     is adapted, then the DofManager reaaranges the memory if necessary.
     The DofManager also organizes to copy old mem to new mem.
   */
  class DofArrayMemory
  {
  private:
    // pointer to memory
    MemPointerType * vec_;

    // size of array
    int size_;

    // sizeof one array entry
    size_t objSize_;

    // name of this array
    const char * name_;

    // only this class is allowed to generate instances of this class
    friend class MemObject;

    // Constructor can only be called from MemObject
    DofArrayMemory(const char * name, size_t objSize) : vec_ (0), size_(0)
                                                        , objSize_(objSize) , name_(name) {}

  public:
    //! size of vec
    int size () const { return size_; }

    //! size of one entry
    size_t objSize() const { return objSize_; }

    //! copy array
    void assign ( const DofArrayMemory &copy )
    {
      assert(size_    == copy.size_);
      assert(objSize_ == copy.objSize_);

      std::memcpy(vec_,copy.vec_, size_ * objSize_);
    }

    //! cast this vector to right type and return entry i
    template <class T>
    T& get ( int i ) { return static_cast<T *> ((void *)vec_)[i]; }

    //! cast this vector to right type and return entry i
    template <class T>
    const T& get ( int i ) const { return static_cast<T *> ((void *)vec_)[i]; }

    //! return vector for cg scheme
    template <class T>
    T* vector () { return static_cast<T *> ((void *)vec_); }

    template <class T>
    const T* vector () const { return static_cast<T *> ((void *)vec_); }

    //! write data to xdr stream
    template <class T>
    bool processXdr(XDR *xdrs)
    {
      std::cerr << "WARNING: DofArrayMemory::processXdr: No appropriate xdr type!\n";
      if(xdrs != 0)
      {
        int len = size_;
        xdr_int( xdrs, &len );
        assert(size_ <= len);
        xdr_vector(xdrs,(char *) vec_,size_, sizeof(T) ,(xdrproc_t)xdr_double);
        return true;
      }
      else
        return false;
    }

  private:
    // set new memory, to be called only from MemObject
    void resize (MemPointerType * mem, int newSize )
    {
      size_ = newSize;
      vec_ = mem;
    }
  };

  //! specialisation for int
  template <>
  inline
  bool DofArrayMemory::processXdr<int>(XDR *xdrs)
  {
    typedef int T;
    if(xdrs != 0)
    {
      int len = size_;
      xdr_int( xdrs, &len );
      assert(size_ <= len);
      xdr_vector(xdrs,(char *) vec_,size_, sizeof(T) ,(xdrproc_t)xdr_int);
      return true;
    }
    else
      return false;
  }

  //! specialisation for double
  template <>
  inline
  bool DofArrayMemory::processXdr<double>(XDR *xdrs)
  {
    typedef double T;

    if(xdrs != 0)
    {
      int len = size_;
      xdr_int( xdrs, &len );
      assert(size_ <= len);

      xdr_vector(xdrs,(char *) vec_,size_, sizeof(T) ,(xdrproc_t)xdr_double);
      return true;
    }
    else
      return false;
  }

  /*!
     DofArray is the array that a discrete functions sees. If a discrete
     function is created, then it is signed in by the function space and the
     return value is a MemObject. This MemObject contains a DofArrayMemory
     which is then as reference given to the DofArray of the DiscreteFunction.
     The DofArray is only a wrapper class for DofArrayMemory where we dont know
     the type of the dofs only the size of one dof.
     Therefore we have this wrapper class for cast to the right type.
   */
  template <class T>
  class DofArray
  {
    // the real memory , we only do the casts here
    DofArrayMemory & array_;
  public:
    //! store reference to real array
    DofArray(DofArrayMemory & array) : array_ (array)
    {
      assert(sizeof(T) == array_.objSize());
    }

    //! return number of enties of array
    int size () const { return array_.size(); }

    //! return reference to entry i
    T&       operator [] ( int i )       { return array_.template get<T>(i); }

    //! return reference to const entry i
    const T& operator [] ( int i ) const { return array_.template get<T>(i); }

    //! assign arrays
    DofArray<T>& operator= (const DofArray<T> &copy)
    {
      array_.assign(copy.array_);
      return *this;
    }

    //! operator = assign all entrys with value t
    DofArray<T>& operator= (const T t)
    {
      for(int i=0; i<size(); i ++) this->operator [] (i) = t;
      return *this;
    }

    T* vector() { return array_.vector<T> (); }
    const T* vector() const { return array_.vector<T> (); }

    //! read and write xdr
    bool processXdr(XDR *xdrs)
    {
      return array_.template processXdr<T> (xdrs);
    }
  };

  class DefaultGHMM
  {
  public:
    MemPointerType *Malloc (size_t n)
    {
      MemPointerType *p;
      p = new MemPointerType[n];
      return p;
    }

    void  Free (MemPointerType *p)
    {
      delete[] p;
    }
  };



  /*!
     A MemObject holds the memory for one DiscreteFunction and the
     corrsponding DofArrayMemory. If a DiscreteFunction is signed in by a
     function space, then such a MemObject is created by the DofManager.
     The MemObject also know the DofMapper from the function space which the
     discrete function belongs to. Here we dont know the exact type of the dof
     mapper therefore the methods newSize and calcInsertPoints of the mappers
     have to be virtual. This isnt a problem because this methods should only
     be called during memory reorganizing which is only once per timestep.
   */
  class MemObject
  {
  private:
    // size of mem entities
    int memSize_;

    // actual size of vector
    int vecSize_;

    // name of discrete function we belong to
    const char * name_;

    // Memory Manager
    DefaultGHMM & ghmm_;

    // sizeof datatype
    size_t sizeOfObj_;

    // pointer to memory
    MemPointerType * myMem_;

    // the dof set stores number of dofs on entity for each codim
    DofMapperInterface * dofmap_;

    DofArrayMemory array_;

  public:
    // Constructor of MemObject, only to call from DofManager
    template <class GridType, class MapperType>
    MemObject ( GridType & grid, MapperType & mapper,
                const char * name , DefaultGHMM & ghmm , size_t objSize )
      : memSize_(0), vecSize_(0), name_ (name) , ghmm_( ghmm ) , sizeOfObj_ (objSize)
        , myMem_(0) , dofmap_ (0)
        , array_( name_, sizeOfObj_ )
    {
      vecSize_ = mapper.size( grid.maxlevel() );
      memSize_ = vecSize_;
      dofmap_ = &mapper;

      myMem_   = ghmm_.Malloc( memSize_ * sizeOfObj_ );
    }

    ~MemObject ()
    {
      if(myMem_) ghmm_.Free(myMem_);
    }

    // defines the corresponding array type
    typedef DofArrayMemory DefaultArrayType;

    //! returns name of this vector
    const char * name () const { return name_; }

    //! if grid changed, then calulate new size of dofset
    int newSize () const { return dofmap_->newSize(); }

    //! return number of entities
    int size () const { return vecSize_; }

    //! return size of allocated memory
    int memSize () const { return memSize_; }

    //! return size on one entity
    size_t objSize () const { return sizeOfObj_; }

    //! return pointer to memory
    MemPointerType * myMem() const { return myMem_; }

    //! return reference for Constructor of DofArray
    DefaultArrayType & getArray()
    {
      array_.resize( myMem_ , vecSize_ );
      return array_;
    }

    //! get new mem from dof manager
    void resize ( MemPointerType * mem, int newMemSize, int newVecSize )
    {
      memSize_  = newMemSize;
      vecSize_  = newVecSize;
      myMem_ = mem;
      array_.resize ( mem , vecSize_ );
    }
  };

  /*!
     The DofManager is responsable for managing memory allocation and freeing
     for all discrete functions living on the grid the manager belongs to.
     The rule is: For a certain grid only one DofManager, but a least one.
     Each function space knows the dofmanager and can sign in the discrete
     functions that belong to that space. If the grid is adapted, then the
     dofmanager reorganizes the memory if necessary. The DofManager holds a
     list or vector of MemObject which know the memory and the corresponding
     mapper so they can determine the size of new memory.
     Furthermore the DofManager holds an IndexSet which the DofMapper need for
     calculating the indices in the dof vector for a given entity and local dof
     number. This IndexSet is deliverd to the mapper, if a function space is
     created. The default value for the IndexSet is the DefaultIndexSet class
     which is mostly a wrapper for the grid indices.
   */
  template <class GridType, class IndexSetImp = DefaultGridIndexSet<GridType> >
  class DofManager
  {
  public:
    // all things for one discrete function are put together in a MemObject
    typedef MemObject MemObjectType;
    typedef IndexSetImp IndexSetType;

  private:
    typedef DoubleLinkedList < MemObjectType * > ListType;
    typedef typename ListType::Iterator ListIteratorType;

    // list with MemObjects, for each DiscreteFunction we have one MemObject
    ListType memList_;

    // the memory managers
    DefaultGHMM ghmm_;

    // the dofmanager belong to one grid only
    GridType & grid_;

    // index set for mapping
    mutable IndexSetType indexSet_;

  public:
    //! Constructor, creates and index set
    DofManager (GridType & grid)
      : grid_(grid),  indexSet_ ( grid )
    {}

    //! Constructor, creates and index set
    DofManager (GridType & grid, bool verbose)
      : grid_(grid), indexSet_ ( grid )
    {}

    //! Desctructor, removes all MemObjects
    ~DofManager ()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      int count = 0;
      for( ; it != endit ; ++it)
      {
        assert(count < memList_.size());
        count++;
        // alloc new mem an copy old mem
        //MemPointerType * mem = (*it)->myMem();
        //ghmm_.Free(mem);
        MemObjectType * mobj = (*it);
        dverb << "Removing " << count << " '" << mobj->name() << "' from DofManager!\n";
        memList_.erase( it );
        if(mobj) delete mobj;
      }
    }

    //! add dofset to dof manager
    //! this method should be called at signIn of DiscreteFucntion, and there
    //! we know our DofType T
    template <class T, class MapperType>
    MemObjectType & addDofSet(T * t, GridType &grid, MapperType & mapper, const char * name )
    {
      assert(&grid_ == &grid);
      dverb << "Adding '" << name << "' to DofManager! \n";
      MemObjectType * obj =
        new MemObjectType( grid , mapper, name, ghmm_ , sizeof(T) );

      memList_.insert_after ( memList_.rbegin() , obj );
      return *obj;
    }

    //! remove MemObject, is called from DiscreteFucntion
    bool removeDofSet (MemObjectType & obj)
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      bool removed = false;
      for( ; it != endit ; ++it)
      {
        if((*it) == &obj)
        {
          // alloc new mem an copy old mem
          //MemPointerType * mem  = (*it)->myMem();
          //ghmm_.Free(mem);
          MemObjectType * mobj = (*it);
          memList_.erase( it );
          dverb << "Removing '" << obj.name() << "' from DofManager!\n";
          if(mobj) delete mobj;
          removed = true;
          break;
        }
      }
      return removed;
    }

    //! generate index for father
    template <class EntityType>
    void createFatherIndex (EntityType &en)
    {
      indexSet_.createFatherIndex(en);
    }

    void resizeTmp ()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        int memSize  = (*it)->memSize();

        // create new memory, which smaller than the mem we have
        int newSize  = indexSet_.tmpSize();

        // if we have enough, do notin'
        if(newSize <= memSize) continue;

        // alloc new mem an copy old mem
        MemPointerType * mem    = (*it)->myMem();
        MemPointerType * newMem = (MemPointerType *) ghmm_.Malloc((*it)->objSize()*newSize);
        std::memcpy(newMem,mem, memSize * (*it)->objSize());
        (*it)->resize(newMem,newSize,newSize);

        // free old mem
        //std::cout << mem << " free Mem\n";
        ghmm_.Free(mem);
      }
    }

    void resize()
    {
      indexSet_.resize();
      resizeMem();
    }

    //! resize the MemObject if necessary
    void resizeMem()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {

        int newSize = (*it)->newSize();
        int memSize  = (*it)->memSize();
        MemPointerType * mem  = (*it)->myMem();

        if(newSize <= memSize)
        {
          (*it)->resize(mem,memSize,newSize);
          continue;
        }

        // alloc new mem an copy old mem
        assert(mem != 0);
        MemPointerType * newMem = (MemPointerType *) ghmm_.Malloc((*it)->objSize()*newSize);
        std::memcpy(newMem,mem, memSize * (*it)->objSize());
        (*it)->resize(newMem,newSize,newSize);

        // free old mem
        ghmm_.Free(mem);
      }
    }

    void dofCompress()
    {
      // keeps the old indices for a while
      bool haveToCompress = indexSet_.compress();

      if( haveToCompress )
      {
        ListIteratorType it    = memList_.begin();
        ListIteratorType endit = memList_.end();


        for( ; it != endit ; ++it)
        {
          // gem mem pointer and object size
          MemPointerType * mem = (*it)->myMem();
          size_t objSize = (*it)->objSize();

          for(int i=0; i<indexSet_.oldSize(0,0); i++)
          {
            if(indexSet_.indexNew(i))
            {
              int oldInd = indexSet_.oldIndex(i);
              int newInd = indexSet_.newIndex(i);

              // copy value form old to new place
              MemPointerType * t1 = (mem + (newInd*objSize));
              MemPointerType * t2 = (mem + (oldInd*objSize));
              std::memcpy(t1,t2, objSize);
            }
          }

          // stroe new size, which is smaller then size
          int newSize = (*it)->newSize();
          int memSize = (*it)->memSize();
          (*it)->resize(mem,memSize,newSize);
        }
      }

      //std::cout << "Dof Compress Done! \n";
    }

    //! return indexSet for dofmappers
    IndexSetType & indexSet () const
    {
      return indexSet_;
    }

    //********************************************************
    // read-write Interface for index set
    //********************************************************
    bool write(const FileFormatType ftype, const char *filename, int timestep);
    bool read(const char *filename, int timestep);
    bool write_xdr( const char * filename, int timestep);
    bool read_xdr( const char * filename, int timestep);
  };

  template <class GridType,class IndexSetType>
  inline bool DofManager<GridType,IndexSetType>::
  write(const FileFormatType ftype, const char *filename, int timestep)
  {
    assert(ftype == xdr);
    return write_xdr(filename,timestep);
  }
  template <class GridType,class IndexSetType>
  inline bool DofManager<GridType,IndexSetType>::
  read(const char * filename , int timestep)
  {
    return read_xdr(filename,timestep);
  }

  template <class GridType,class IndexSetType>
  inline bool DofManager<GridType,IndexSetType>::
  write_xdr(const char * filename , int timestep)
  {
    //std::cout << indexSet_.size(grid_.maxlevel(),0) << " Size\n";
    return indexSet_.write_xdr(filename,timestep);
  }

  template <class GridType,class IndexSetType>
  inline bool DofManager<GridType,IndexSetType>::
  read_xdr(const char * filename , int timestep)
  {
    return indexSet_.read_xdr(filename,timestep);
  }

} // end namespace Dune

#endif
