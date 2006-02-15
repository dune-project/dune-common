// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LOCALFUNCTIONWRAPPER_HH
#define DUNE_LOCALFUNCTIONWRAPPER_HH

#include <vector>
#include <stack>

namespace Dune {

  //! Stores pointers of local functions in a stack
  template <class DiscreteFunctionImp>
  class LocalFunctionStorage
  {
  private:
    typedef LocalFunctionStorage<DiscreteFunctionImp> MyType;
    typedef DiscreteFunctionImp DiscreteFunctionType;
    typedef typename DiscreteFunctionImp::LocalFunctionImp LocalFunctionImp;

  public:
    typedef typename std::pair<LocalFunctionImp * , int* > StackStorageType;
  private:
    std::stack < StackStorageType , std::vector<StackStorageType> > lfStack_;
    const DiscreteFunctionType & df_;

    int numIssuedFunctions_;

  public:
    //! constructor
    LocalFunctionStorage (const DiscreteFunctionType & df)
      : df_(df) , numIssuedFunctions_(0) {}

    //! delete all objects on stack
    ~LocalFunctionStorage ()
    {
      assert(numIssuedFunctions_ == 0);

      while ( !lfStack_.empty() )
      {
        StackStorageType obj = lfStack_.top();
        lfStack_.pop();
        delete obj.first;
        obj.first = 0;
        delete obj.second;
        obj.second = 0;
      }
    }

    //! get local function object
    StackStorageType getObject ()
    {
#ifndef NDEBUG
      ++numIssuedFunctions_;
#endif

      if( lfStack_.empty() )
      {
        // first pointer is the local function pointer
        // ans second pointer is the reference counter initialized with 1
        return StackStorageType ( df_.newLocalFunctionObject() , new int (1) );
      }
      else
      {
        StackStorageType obj = lfStack_.top();
        lfStack_.pop();
        return obj;
      }
    }

    //! push local function to stack
    void freeObject (StackStorageType & obj)
    {
#ifndef NDEBUG
      --numIssuedFunctions_;
#endif
      lfStack_.push(obj);
    }

  private:
    //! prohibited methods
    LocalFunctionStorage ( const MyType & c);
    MyType & operator = ( const MyType & c );
  };

  template < class DFTraits > class DiscreteFunctionDefault;
  template < class DiscreteFunctionSpaceType, class LocalFunctionImp > class LocalFunctionDefault;
  //**************************************************************************
  //
  //  --LocalFunctionWrapper
  //
  //**************************************************************************
  //! Manages the getting and deleting of local function pointers and
  //! acts like a local functions
  template < class DiscreteFunctionImp >
  class LocalFunctionWrapper :
    public LocalFunctionDefault <
        typename DiscreteFunctionImp::DiscreteFunctionSpaceType,
        LocalFunctionWrapper < DiscreteFunctionImp > >
  {
  public:
    typedef typename DiscreteFunctionImp::LocalFunctionImp LocalFunctionImp;
    typedef typename DiscreteFunctionImp::DiscreteFunctionSpaceType
    DiscreteFunctionSpaceType;

    typedef DiscreteFunctionImp DiscreteFunctionType;
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef LocalFunctionWrapper < LocalFunctionImp > MyType;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };

    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;

    typedef DiscreteFunctionDefault< typename DiscreteFunctionImp::Traits >
    DiscreteFunctionDefaultType;

    typedef typename DiscreteFunctionDefaultType::LocalFunctionStorageType LFStorage;

  private:
    // local function storage stack
    LFStorage & storage_;

    // type of stack entry
    typedef typename LFStorage :: StackStorageType StackStorageType;

    // pair storing pointer to local function and poiner to ref-counter
    StackStorageType obj_;

    // reference to local function
    LocalFunctionImp & lf_;

  public:

    //! Constructor initializing the underlying local function
    template < class EntityType >
    LocalFunctionWrapper(const EntityType & en, const DiscreteFunctionImp & df)
      : storage_( df.localFunctionStorage() )
        , obj_ ( storage_.getObject() )
        , lf_ ( *obj_.first )
    {
      // init real local function with entity
      localFunc().init( en );
    }

    //! Constructor creating empty local function
    LocalFunctionWrapper (const DiscreteFunctionImp & df)
      : storage_( df.localFunctionStorage() )
        , obj_( storage_.getObject() )
        , lf_ ( *obj_.first )
    {}

    //! Copy constructor
    LocalFunctionWrapper(const LocalFunctionWrapper& org)
      : storage_(org.storage_)
        , obj_( org.obj_ )
        , lf_ ( *obj_.first )
    {
      assert(*obj_.second == 1);
      ++(*(obj_.second));
    }

    //! Destructor , push local function to stack if there are no other
    //! to it references
    ~LocalFunctionWrapper ()
    {
      removeObj();
    }

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num) { return localFunc()[num]; }

    //! access to dof number num, all dofs of the dof entity
    const RangeFieldType & operator [] (int num) const { return localFunc()[num]; }

    //! return number of degrees of freedom
    int numDofs () const { return localFunc().numDofs(); }

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const
    {
      localFunc().evaluate( en , x , ret );
    }

    //! sum over all local base functions but local
    template <class EntityType>
    void evaluateLocal(EntityType &en, const DomainType & x, RangeType & ret) const
    {
      localFunc().evaluateLocal( en , x , ret );
    }

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const
    {
      localFunc().evaluate( en , quad, quadPoint , ret );
    }

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void jacobian (EntityType &en, QuadratureType &quad, int quadPoint , JacobianRangeType & ret) const
    {
      localFunc().jacobian( en, quad, quadPoint, ret );
    }

    //! sum over all local base functions evaluated on given quadrature
    //point, but local
    template <class EntityType>
    void jacobianLocal(EntityType& en, const DomainType& x, JacobianRangeType& ret) const
    {
      localFunc().jacobianLocal( en, x , ret );
    }

    //! sum over all local base functions evaluated on given point x
    template <class EntityType>
    void jacobian(EntityType& en, const DomainType& x, JacobianRangeType& ret) const
    {
      localFunc().jacobian( en, x, ret);
    }

    //! update local function for given Entity
    //! deprecated method
    template <class EntityType >
    void init ( const EntityType &en )
    {
      localFunc().init(en);
    }

    const BaseFunctionSetType& getBaseFunctionSet() const
    {
      return localFunc().getBaseFunctionSet();
    }

  private:
    LocalFunctionWrapper& operator=(const LocalFunctionWrapper);

  private:
    LocalFunctionImp & localFunc()
    {
      assert( obj_.first );
      assert( &lf_ == obj_.first );
      return lf_;
    }
    const LocalFunctionImp & localFunc() const
    {
      assert( obj_.first );
      assert( &lf_ == obj_.first );
      return lf_;
    }

    //! method remove the obj by using the storage
    void removeObj ()
    {
      assert( obj_.first );
      assert( *(obj_.second) > 0);
      // the second counter is left at a value of 1, so we dont have to
      // initialize when getting the object again
      if( (*(obj_.second)) == 1) {
        storage_.freeObject ( obj_ );
      }
      else
        --(*(obj_.second));
    }
  }; // end LocalFunctionWrapper

} // end namespace Dune

#endif
