// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFMANAGER_HH__
#define __DUNE_DOFMANAGER_HH__

#include <dune/common/dlist.hh>
#include <dune/fem/common/dofmapperinterface.hh>

namespace Dune {

  /*!
     The DefaultGridIndexSet is a wrapper for the grid index which can be
     index of entity or global_index. The DofMapper uses an IndexSet for
     mapping the dofs, so we can hide the real grid index behijnd the index
     set. Furthermore if an grid doesn't provide the consecutive index set
     then this can be calculated in the IndexSet. These two following index
     sets are just the identiy to the grid indices.
   */
  template <class GridType>
  class DefaultGridIndexSetBase
  {
  public:
    enum { ncodim = GridType::dimension + 1 };

    DefaultGridIndexSetBase ( GridType & grid ) : grid_ (grid) {}

    void insertNew( GridType & grid )
    {}

    bool write_xdr(const char * filename , int timestep)
    {
      return true;
    }

    bool read_xdr(const char * filename , int timestep)
    {
      return true;
    }

  protected:
    GridType & grid_;
  };

  //! Default is the Identity
  template <class GridType, GridIndexType GridIndex = GlobalIndex>
  class DefaultGridIndexSet : public DefaultGridIndexSetBase <GridType>
  {
    template <class EntityType,int enCodim, int codim>
    struct IndexWrapper
    {
      static int index (EntityType & en , int num )
      {
        return en.global_index();
      }
    };

    template <class EntityType, int codim>
    struct IndexWrapper<EntityType,codim,codim>
    {
      static int index (EntityType & en , int num )
      {
        return en.global_index();
      }
    };

    //! if codim > codim of entity use subIndex
    template <class EntityType>
    struct IndexWrapper<EntityType,0,2>
    {
      static int index (EntityType & en , int num )
      {
        return en.template subIndex<2> (num);
      }
    };

    template <class EntityType>
    struct IndexWrapper<EntityType,0,3>
    {
      static int index (EntityType & en , int num )
      {
        return en.template subIndex<3> (num);
      }
    };

  public:
    enum { ncodim = GridType::dimension + 1 };
    DefaultGridIndexSet ( GridType & grid ) : DefaultGridIndexSetBase <GridType> (grid) {}

    int size ( int level , int codim ) const
    {
      return this->grid_.global_size(codim);
    }

    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return IndexWrapper<EntityType,EntityType::codimension,codim>::index(en,num);
    }
  };

  template <class GridType>
  class DefaultGridIndexSet<GridType,LevelIndex>
    : public DefaultGridIndexSetBase <GridType>
  {
    template <class EntityType,int enCodim, int codim>
    struct IndexWrapper
    {
      static int index (EntityType & en , int num )
      {
        return en.index();
      }
    };

    template <class EntityType, int codim>
    struct IndexWrapper<EntityType,codim,codim>
    {
      static int index (EntityType & en , int num )
      {
        return en.index();
      }
    };

    template <class EntityType>
    struct IndexWrapper<EntityType,0,2>
    {
      static int index (EntityType & en , int num )
      {
        return en.template subIndex<2> (num);
      }
    };

  public:
    enum { ncodim = GridType::dimension + 1 };
    DefaultGridIndexSet ( GridType & grid ) : DefaultGridIndexSetBase <GridType> (grid) {}

    //! return size of grid entities per level and codim
    int size ( int level , int codim ) const
    {
      return this->grid_.size(level,codim);
    }

    //! return index of entity with codim codim belonging to entity en which
    //! could have a bigger codim (for example return num of vertex num of an
    //! element en
    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return IndexWrapper<EntityType,EntityType::codimension,codim>::index(en,num);
    }
  };

  // forward declaration
  template <class GridType,
      class IndexSetImp = DefaultGridIndexSet<GridType> >
  class DofManager;

  // forward declaration
  class MemObject;

  // type of pointer to memory, for easy replacements
  typedef void MemPointerType;

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
    T& get ( int i ) { return static_cast<T *> (vec_)[i]; }

    //! cast this vector to right type and return entry i
    template <class T>
    const T& get ( int i ) const { return static_cast<T *> (vec_)[i]; }

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

    //! read and write xdr
    bool processXdr(XDR *xdrs)
    {
      return array_.template processXdr<T> (xdrs);
    }
  };


  class DefaultGHMM
  {
  public:
    void *Malloc (unsigned long n)
    {
      void *p;
      p =  std::malloc((size_t) n);
      if (p==0) std::cerr << "Malloc: could not allocate " << n << " bytes\n";
      assert(p != 0);
      return p;
    }

    void  Free (void *p)
    {
      assert(p != 0);
      std::free(p);
      return;
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
    // number of entities
    int size_;

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
      : size_(0), name_ (name) , ghmm_( ghmm ) , sizeOfObj_ (objSize)
        , myMem_(0) , dofmap_ (0)
        , array_( name_, sizeOfObj_ )
    {
      size_ = mapper.size( grid.maxlevel() );
      dofmap_ = &mapper;

      myMem_   = ghmm_.Malloc( size_ * sizeOfObj_ );
    }

    // defines the corresponding array type
    typedef DofArrayMemory DefaultArrayType;

    //! returns name of this vector
    const char * name () const { return name_; }

    //! if grid changed, then calulate new size of dofset
    int newSize (int level) const { return dofmap_->newSize(level); }

    //! return number of entities
    int size () const { return size_; }

    //! return size on one entity
    size_t objSize () const { return sizeOfObj_; }

    //! return pointer to memory
    MemPointerType * myMem() const { return myMem_; }

    //! return reference for Constructor of DofArray
    DefaultArrayType & getArray()
    {
      array_.resize( myMem_ , size_ );
      return array_;
    }

    //! get new mem from dof manager
    void resize ( MemPointerType * mem, int newSize )
    {
      size_  = newSize;
      myMem_ = mem;
      array_.resize ( mem , size_ );
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
  template <class GridType, class IndexSetImp >
  class DofManager
  {
  public:
    // all things for one discrete function are put together in a MemObject
    typedef MemObject MemObjectType;
    typedef IndexSetImp IndexSetType;

  private:
    typedef DoubleLinkedList < MemObjectType * > ListType;
    typedef ListType::Iterator ListIteratorType;

    // list with MemObjects, for each DiscreteFunction we have one MemObject
    ListType memList_;

    // the memory managers
    DefaultGHMM ghmm_;

    // the dofmanager belong to one grid only
    GridType & grid_;

    // verbose, true if output of messages
    bool verbose_;

    // index set for mapping
    mutable IndexSetType indexSet_;

  public:
    //! Constructor, creates and index set
    DofManager (GridType & grid, bool verbose = false)
      : grid_(grid), verbose_ (verbose) , indexSet_ ( grid )
    {
      indexSet_.insertNew( grid );
    }

    //! Desctructor, removes all MemObjects
    ~DofManager ()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        // alloc new mem an copy old mem
        MemPointerType * mem = (*it)->myMem();
        ghmm_.Free(mem);
        MemObjectType * mobj = (*it);
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
      if(verbose_)
        std::cout << "Adding '" << name << "' to DofManager! \n";
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
          MemPointerType * mem  = (*it)->myMem();
          ghmm_.Free(mem);
          MemObjectType * mobj = (*it);
          memList_.erase( it );
          if(verbose_)
            std::cout << "Removing '" << obj.name() << "' from DofManager!\n";
          if(mobj) delete mobj;
          removed = true;
          break;
        }
      }
      return removed;
    }

    //! resize the MemObject if necessary
    void resize()
    {
      indexSet_.insertNew( grid_ );

      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        int newSize = (*it)->newSize(grid_.maxlevel());
        int mySize  = (*it)->size();
        if(newSize <= mySize) continue;

        // alloc new mem an copy old mem
        MemPointerType * mem    = (*it)->myMem();
        MemPointerType * newMem = (MemPointerType *) ghmm_.Malloc((*it)->objSize()*newSize);
        std::memcpy(newMem,mem, mySize * (*it)->objSize());
        (*it)->resize(newMem,newSize);

        // free old mem
        ghmm_.Free(mem);
      }
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
