// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFMANAGER_HH__
#define __DUNE_DOFMANAGER_HH__

#include <assert.h>

#include <dune/common/dlist.hh>
#include <dune/common/stdstreams.hh>
#include <dune/common/exceptions.hh>

#include <dune/fem/common/dofmapperinterface.hh>

// here are the default grid index set defined
#include <dune/grid/common/defaultindexsets.hh>
#include <dune/fem/transfer/datacollector.hh>

namespace Dune {

  // forward declaration
  template <class GridType,
      class DataCollectorType = DataCollectorInterface<GridType> >
  class DofManager;

  // forward declaration

  // type of pointer to memory, for easy replacements
  typedef char MemPointerType;

  //! oriented to the STL Allocator funtionality
  template <class T>
  class DefaultDofAllocator {
  public:
    //! allocate array of nmemb objects of type T
    static T* malloc (size_t nmemb)
    {
      T* p = new T[nmemb];
      return p;
    }

    //! release memory previously allocated with malloc member
    static void free (T* p)
    {
      delete [] p;
    }

    //! allocate array of nmemb objects of type T
    static T* realloc (T* oldMem, size_t oldSize , size_t nmemb)
    {
      T* p = new T[nmemb];
      std::memcpy(p,oldMem,oldSize * sizeof(T));
      DefaultDofAllocator :: free (oldMem);
      return p;
    }
  };

  //! allocator for simple structures like int, double and float
  //! using the C malloc,free, and realloc
  struct SimpleDofAllocator
  {
    //! allocate array of nmemb objects of type T
    template <typename T>
    static T* malloc (size_t nmemb)
    {
      T* p = (T *) std::malloc(nmemb * sizeof(T));
      assert(p);
      return p;
    }

    //! release memory previously allocated with malloc member
    template <typename T>
    static void free (T* p)
    {
      assert(p);
      std::free(p);
    }

    //! allocate array of nmemb objects of type T
    template <typename T>
    static T* realloc (T* oldMem, size_t oldSize , size_t nmemb)
    {
      assert(oldMem);
      T * p = (T *) std::realloc(oldMem,nmemb*sizeof(T));
      assert(p);
      return p;
    }
  };

  template <>
  class DefaultDofAllocator<double>
  {
    typedef double T;
  public:
    //! allocate array of nmemb objects of type T
    static T* malloc (size_t nmemb)
    {
      return SimpleDofAllocator::malloc<T> (nmemb);
    }

    //! release memory previously allocated with malloc member
    static void free (T* p)
    {
      SimpleDofAllocator::free<T> (p);
      return ;
    }

    //! allocate array of nmemb objects of type T
    static T* realloc (T* oldMem, size_t oldSize , size_t nmemb)
    {
      return SimpleDofAllocator::realloc<T> (oldMem,oldSize,nmemb);
    }
  };

  template <>
  class DefaultDofAllocator<int>
  {
    typedef int T;
  public:
    //! allocate array of nmemb objects of type T
    static T* malloc (size_t nmemb)
    {
      return SimpleDofAllocator::malloc<T> (nmemb);
    }

    //! release memory previously allocated with malloc member
    static void free (T* p)
    {
      SimpleDofAllocator::free<T> (p);
      return ;
    }

    //! allocate array of nmemb objects of type T
    static T* realloc (T* oldMem, size_t oldSize , size_t nmemb)
    {
      return SimpleDofAllocator::realloc<T> (oldMem,oldSize,nmemb);
    }
  };


  /*!
     DofArray is the array that a discrete functions sees. If a discrete
     function is created, then it is signed in by the function space and the
     return value is a MemObject. This MemObject contains a DofArrayMemory
     which is then as reference given to the DofArray of the DiscreteFunction.
     The DofArray is only a wrapper class for DofArrayMemory where we dont know
     the type of the dofs only the size of one dof.
     Therefore we have this wrapper class for cast to the right type.
   */
  template <class T, class AllocatorType = DefaultDofAllocator<T> >
  class DofArray
  {
    // size of array
    int size_;

    int memSize_;

    // pointer to mem
    T * vec_;

  public:
    //! create array of length size
    DofArray(int size)
      : size_(size) , memSize_(size) , vec_(0)
    {
      vec_ = AllocatorType :: malloc (size_);
    }

    //! return number of enties of array
    int size () const { return size_; }

    //! return reference to entry i
    T&       operator [] ( int i )
    {
      assert( ((i<0) || (i>=size()) ? (std::cout << std::endl << i << " i|size " << size() << std::endl, 0) : 1));
      return vec_[i];
    }

    //! return reference to const entry i
    const T& operator [] ( int i ) const
    {
      assert( ((i<0) || (i>=size()) ? (std::cout << std::endl << i << " i|size " << size() << std::endl, 0) : 1));
      return vec_[i];
    }

    //! assign arrays
    DofArray<T>& operator= (const DofArray<T> &copy)
    {
      assert(copy.size_ >= size_);
      std::memcpy(vec_,copy.vec_, size_ * sizeof(T));
      return *this;
    }

    //! operator = assign all entrys with value t
    DofArray<T>& operator= (const T t)
    {
      for(int i=0; i<size(); i ++) this->operator [] (i) = t;
      return *this;
    }

    T* vector() { return vec_; }
    const T* vector() const { return vec_; }

    //! read and write xdr
    bool processXdr(XDR *xdrs)
    {
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

    void realloc ( int nsize )
    {
      assert(nsize >= 0);
      if(nsize <= memSize_)
      {
        size_ = nsize;
        return ;
      }

      int nMemSize = (int) (nsize * 0.02);
      nMemSize += nsize;
      vec_ = AllocatorType :: realloc (vec_,size_,nMemSize);

      size_ = nsize;
      memSize_ = nMemSize;
    }
  };

  //! specialisation for int
  template <>
  inline bool DofArray<int>::processXdr(XDR *xdrs)
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
  inline bool DofArray<double>::processXdr(XDR *xdrs)
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
  //******************************************************************
  //
  //  IndexSetObject
  /*! The idea of the IndexSetObject is that, every MemObject has an
   *  IndexSetObject, but if two different MemObjects belong to the same
   *  funtion space, then they have the same IndexSetObject.
   *  Furthermore the IndexSetObject is more or less a wrapper for the
   *  IndexSetInterface, but here we can store aditional infomation, for
   *  example if set has been compressed.
   */
  //******************************************************************

  class IndexSetObjectInterface
  {
  public:
    virtual ~IndexSetObjectInterface () {}
    virtual void resize () = 0;
    virtual bool compress () = 0;
    virtual void unsetCompressed() = 0;
    virtual bool operator == (const IndexSetInterface & iset) = 0;

    virtual void read_xdr(const char * filename, int timestep) = 0;
    virtual void write_xdr(const char * filename, int timestep) const = 0;
  };

  template <class IndexSetType, class EntityType>
  class IndexSetObject : public IndexSetObjectInterface ,
                         public LocalInlinePlus < IndexSetObject<IndexSetType,EntityType> , EntityType >
  {
  private:
    // the dof set stores number of dofs on entity for each codim
    IndexSetType & indexSet_;

    //! true if compress has been called
    bool compressed_;

  public:
    // Constructor of MemObject, only to call from DofManager
    IndexSetObject ( IndexSetType & iset ) : indexSet_ (iset) , compressed_(false) {}

    void resize ()
    {
      indexSet_.resize();
      compressed_ = false;
    }

    bool compress ()
    {
      if(!compressed_)
      {
        indexSet_.compress();
        compressed_ = true;
      }
      return compressed_;
    }

    void unsetCompressed() { compressed_ = false; }

    bool operator == ( const IndexSetInterface & iset )
    {
      return &indexSet_ == &iset;
    }

    void apply ( EntityType & en )
    {
      //std::cout << "Create Index for father " << en.globalIndex() << "\n";
      indexSet_.createFatherIndex ( en );
    }

    virtual void read_xdr(const char * filename, int timestep)
    {
      indexSet_.read_xdr(filename,timestep);
    }
    virtual void write_xdr(const char * filename, int timestep) const
    {
      indexSet_.write_xdr(filename,timestep);
    }

  };

  //****************************************************************
  //
  // MemObject
  //
  //****************************************************************
  class MemObjectInterface
  {
  public:
    virtual ~MemObjectInterface() {};
    virtual void realloc (int newSize) = 0;
    virtual int size () const = 0;
    virtual int additionalSizeEstimate () const = 0;
    virtual int newSize () const = 0;
    virtual const char * name () const  = 0;
    virtual void dofCompress () = 0;
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
  template <class MapperType , class DofArrayType>
  class MemObject : public MemObjectInterface
  {
  private:
    // the dof set stores number of dofs on entity for each codim
    MapperType & mapper_;

    // index set object, holding the index set
    IndexSetObjectInterface & indexObject_;

    // Array which belongs to discrete function
    DofArrayType & array_;

    // name of mem object, i.e. name of discrete function
    const char * name_;

  public:
    // Constructor of MemObject, only to call from DofManager
    MemObject ( MapperType & mapper, IndexSetObjectInterface & iobj,
                DofArrayType & array, const char * name )
      : mapper_ (mapper) , indexObject_(iobj) , array_( array ), name_ (name) {}

    //! returns name of this vector
    const char * name () const { return name_; }

    //! if grid changed, then calulate new size of dofset
    int newSize () const { return mapper_.newSize(); }

    int size () const { return array_.size(); }

    //! return number of entities
    int additionalSizeEstimate () const
    {
      return mapper_.additionalSizeEstimate();
    }

    //! reallocate the memory with the new size
    void realloc ( int nSize )
    {
      array_.realloc( nSize );
    }

    //! copy the dof from the rear section of the vector to the holes
    void dofCompress ()
    {
      indexObject_.compress();

      for(int i=0; i<mapper_.oldSize(); i++)
      {
        if(mapper_.indexNew(i))
        {
          // copy value
          array_[ mapper_.newIndex(i) ] = array_[ mapper_.oldIndex(i) ];
        }
      }

      // store new size, which is should be smaller then actual size
      array_.realloc ( newSize() );
    }

    //! return IndexSetObject for comparison with newly added functions
    IndexSetObjectInterface & getIndexSetObject()
    {
      return indexObject_;
    }
  };

  class DofManError : public Exception {};


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
  template <class GridType , class DataCollectorType >
  class DofManager
  {
  public:
    // all things for one discrete function are put together in a MemObject
    typedef MemPointerType MemoryPointerType;

  private:
    typedef DoubleLinkedList < MemObjectInterface * > ListType;
    typedef typename ListType::Iterator ListIteratorType;

    typedef DoubleLinkedList < IndexSetObjectInterface * > IndexListType;
    typedef typename IndexListType::Iterator IndexListIteratorType;

    // list with MemObjects, for each DiscreteFunction we have one MemObject
    ListType memList_;

    // list of all different indexsets
    IndexListType indexList_;

    // the dofmanager belong to one grid only
    const GridType & grid_;

    // index set for mapping
    mutable DataCollectorType dataInliner_;
    mutable DataCollectorType dataXtractor_;

    typedef typename DataCollectorType::LocalInterfaceType LocalDataCollectorType;
    mutable LocalDataCollectorType dataWriter_;
    mutable LocalDataCollectorType dataReader_;

    typedef LocalInterface<typename GridType::template codim<0>::Entity> LocalIndexSetObjectsType;
    mutable LocalIndexSetObjectsType indexSets_;

  public:
    template <class MapperType , class DofStorageType >
    struct Traits
    {
      typedef MemObject< MapperType, DofStorageType > MemObjectType;
    };

    //! Constructor
    DofManager (const GridType & grid) : grid_(grid) {}

    //! Desctructor, removes all MemObjects and IndexSetObjects
    ~DofManager ()
    {
      if(memList_.size() > 0)
      {
        while( indexList_.rbegin() != indexList_.rend())
        {
          MemObjectInterface * mobj = (* memList_.rbegin() );
          indexList_.erase( indexList_.rbegin() );

          // alloc new mem an copy old mem
          dverb << "Removing '" << mobj->name() << "' from DofManager!\n";
          if(mobj) delete mobj;
        }
      }

      if(indexList_.size() > 0)
      {
        while ( indexList_.rbegin() != indexList_.rend())
        {
          IndexSetObjectInterface * iobj = (* indexList_.rbegin() );
          indexList_.erase( indexList_.rbegin() );
          if(iobj) delete iobj;
        }
      }
    }

    template <class IndexSetType>
    void addIndexSet (const GridType &grid, IndexSetType &iset)
    {
      if(&grid_ != &grid)
        DUNE_THROW(DofManError,"DofManager can only be used for one grid! \n");

      IndexSetInterface & set = iset;
      typedef IndexSetObject< IndexSetType,
          typename GridType::template codim<0>::Entity > IndexSetObjectType;

      IndexSetObjectType * indexSet = 0;

      IndexListIteratorType endit = indexList_.end();
      for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
      {
        if( (* (*it)) == set )
        {
          indexSet = static_cast<IndexSetObjectType *> ((*it));
          break;
        }
      }

      if(!indexSet)
      {
        indexSet = new IndexSetObjectType ( iset );
        IndexSetObjectInterface * iobj = indexSet;
        indexList_.insert_after ( indexList_.rbegin() , iobj );
        indexSets_ += *indexSet;
      }
      return ;
    }

    //! add dofset to dof manager
    //! this method should be called at signIn of DiscreteFucntion, and there
    //! we know our DofStorage which is the actual DofArray
    template <class DofStorageType, class IndexSetType, class MapperType >
    MemObject<MapperType,DofStorageType> &
    addDofSet(DofStorageType & ds, const GridType &grid, IndexSetType &iset,
              MapperType & mapper, const char * name )
    {
      if(&grid_ != &grid)
        DUNE_THROW(DofManError,"DofManager can only be used for one grid! \n");
      dverb << "Adding '" << name << "' to DofManager! \n";

      IndexSetInterface & set = iset;
      typedef IndexSetObject< IndexSetType,
          typename GridType::template codim<0>::Entity > IndexSetObjectType;

      IndexSetObjectType * indexSet = 0;

      IndexListIteratorType endit = indexList_.end();
      for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
      {
        if( (* (*it)) == set )
        {
          indexSet = static_cast<IndexSetObjectType *> ((*it));
          break;
        }
      }

      // index was added when functions space is created, should be here
      assert( indexSet );
      if( !indexSet )
        DUNE_THROW(DofManError,"No IndexSet for DofSet! \n");

      MemObject<MapperType,DofStorageType> * obj =
        new MemObject<MapperType,DofStorageType> ( mapper, *indexSet , ds , name );

      MemObjectInterface * saveObj = obj;
      memList_.insert_after ( memList_.rbegin() , saveObj );

      return *obj;
    }

    //! remove MemObject, is called from DiscreteFucntionSpace at sign out of
    //! DiscreteFunction
    bool removeDofSet (MemObjectInterface & obj)
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      bool removed = false;
      for( ; it != endit ; ++it)
      {
        if((*it) == &obj)
        {
          // alloc new mem an copy old mem
          MemObjectInterface * mobj = (*it);
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
    void createFatherIndex (typename GridType::template codim<0>::Entity & en )
    {
      // calls createFatherIndex for all IndexSets
      indexSets_.apply( en );
    }

    void resizeForRestrict ()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        (*it)->realloc ( (*it)->size() + (*it)->additionalSizeEstimate() );
      }
    }

    void resizeMem (int nsize)
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        (*it)->realloc ( (*it)->size() + nsize );
      }
    }

    void resize()
    {
      IndexListIteratorType endit = indexList_.end();
      for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
      {
        (*it)->resize();
      }

      resizeDofMem();
    }

  private:
    //! resize the MemObject if necessary
    void resizeDofMem()
    {
      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        int nSize = (*it)->newSize();
        (*it)->realloc ( nSize );
      }
    }

    void unsetIndexSets ()
    {
      IndexListIteratorType endit = indexList_.end();
      for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
      {
        // reset compressed so the next time compress of index set is called
        (*it)->unsetCompressed();
      }
    }

  public:
    void dofCompress()
    {
      unsetIndexSets ();

      ListIteratorType it    = memList_.begin();
      ListIteratorType endit = memList_.end();

      for( ; it != endit ; ++it)
      {
        // if correponding index was not compressed yet, yhis is called in
        // the MemObject dofCompress, if index has not changes, nothing happens
        (*it)->dofCompress () ;
      }
    }

    template <class DataCollType>
    void addDataInliner ( DataCollType & d)
    {
      dataInliner_ += d;
    }

    template <class DataCollType>
    void addDataXtractor ( DataCollType & d)
    {
      dataXtractor_ += d;
    }

    template <class DataCollType>
    void addDataWriter ( DataCollType & d)
    {
      dataWriter_ += d;
    }

    template <class DataCollType>
    void addDataReader ( DataCollType & d)
    {
      dataReader_ += d;
    }

    template <class ObjectStreamType, class EntityType>
    void inlineData ( ObjectStreamType & str, EntityType & en )
    {
      dataInliner_.apply(str,en);
    }

    template <class ObjectStreamType, class EntityType>
    void scatter ( ObjectStreamType & str, EntityType & en )
    {
      //resize();
      //std::cout << "Scatter of el = " << en.global_index () << "\n";
      std::pair < ObjectStreamType * , EntityType * > p (&str,&en);
      dataWriter_.apply( p );
    }

    template <class ObjectStreamType, class EntityType>
    void gather ( ObjectStreamType & str, EntityType & en )
    {
      //std::cout << "Gather of el = " << en.global_index () << "\n";
      resize();
      std::pair < ObjectStreamType * , EntityType * > p (&str,&en);
      dataReader_.apply( p );
    }

    template <class ObjectStreamType, class EntityType>
    void xtractData ( ObjectStreamType & str, EntityType & en )
    {
      resize();
      dataXtractor_.apply(str,en);
    }

    //********************************************************
    // read-write Interface for index set
    //********************************************************
    bool write(const FileFormatType ftype, const char *filename, int timestep);
    bool read(const char *filename, int timestep);
    bool write_xdr( const char * filename, int timestep);
    bool read_xdr( const char * filename, int timestep);
  };

  template <class GridType, class DataCollectorType>
  inline bool DofManager<GridType,DataCollectorType>::
  write(const FileFormatType ftype, const char *filename, int timestep)
  {
    assert(ftype == xdr);
    return write_xdr(filename,timestep);
  }
  template <class GridType, class DataCollectorType>
  inline bool DofManager<GridType,DataCollectorType>::
  read(const char * filename , int timestep)
  {
    return read_xdr(filename,timestep);
  }

  template <class GridType, class DataCollectorType>
  inline bool DofManager<GridType,DataCollectorType>::
  write_xdr(const char * filename , int timestep)
  {
    assert( filename );

    int count = 0;
    IndexListIteratorType endit = indexList_.end();
    for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
    {
      char * newFilename = new char [strlen(filename) + 10];
      sprintf(newFilename,"%s_%d_",filename,count);
      (*it)->write_xdr(newFilename,timestep);
      count ++;
      if(newFilename) delete [] newFilename;
    }
    return true;
  }

  template <class GridType, class DataCollectorType>
  inline bool DofManager<GridType,DataCollectorType>::
  read_xdr(const char * filename , int timestep)
  {
    assert( filename );

    int count = 0;
    IndexListIteratorType endit = indexList_.end();
    for(IndexListIteratorType it = indexList_.begin(); it != endit; ++it)
    {
      char * newFilename = new char [strlen(filename) + 10];
      sprintf(newFilename,"%s_%d_",filename,count);
      (*it)->read_xdr(newFilename,timestep);
      count ++;
      if(newFilename) delete [] newFilename;
    }
    return true;
  }

} // end namespace Dune

#endif
