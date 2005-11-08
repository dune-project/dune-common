// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LOCALFUNCTIONWRAPPER_HH
#define DUNE_LOCALFUNCTIONWRAPPER_HH

#include <vector>
#include <stack>

#include "../common/discretefunction.hh"

#include <dune/common/stack.hh>

namespace Dune {

  //! Stores pointers of local functions in a stack
  template <class DiscreteFunctionImp >
  class LocalFunctionStorage
  {
  private:
    typedef LocalFunctionStorage < DiscreteFunctionImp > MyType;
    typedef DiscreteFunctionImp DiscreteFunctionType;
    typedef typename DiscreteFunctionImp ::  LocalFunctionImp LocalFunctionImp;

  public:
    typedef typename std::pair < LocalFunctionImp * , int * > StackStorageType;
  private:
    std::stack < StackStorageType , std::vector<StackStorageType> > lfStack_;
    //FiniteStack< StackStorageType *, 10000 > lfStack_;
    const DiscreteFunctionType & df_;

    StackStorageType obj_;

  public:
    //! constructor
    LocalFunctionStorage (const DiscreteFunctionType & df)
      : df_(df) , obj_(0,0) {}

    //! delete all objects on stack
    ~LocalFunctionStorage ()
    {
      while ( !lfStack_.empty() )
      {
        obj_ = lfStack_.top();
        lfStack_.pop();
        if( obj_.first  ) delete obj_.first;
        obj_.first = 0;
        if( obj_.second ) delete obj_.second;
        obj_.second = 0;
      }
    }

    //! get local function object
    StackStorageType & getObject ()
    {
      if( lfStack_.empty() )
      {
        // first pointer is the local function pointer
        // ans second pointer is the reference counter initialized with 1
        obj_ = StackStorageType ( df_.newLocalFunctionObject(), new int (1) );
        return obj_;
      }
      else
      {
        obj_ = lfStack_.top();
        lfStack_.pop();
        return obj_;
      }
    }

    //! push local function to stack
    void freeObject ( StackStorageType & obj)
    {
      lfStack_.push(obj);
    }

  private:
    //! prohibited methods
    LocalFunctionStorage ( const MyType & c); // : df_(c.df_) {};
    MyType & operator = ( const MyType & c ); // { return *this; }
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
  class LocalFunctionWrapper
    : public LocalFunctionDefault <
          typename DiscreteFunctionImp :: DiscreteFunctionSpaceType,
          LocalFunctionWrapper < DiscreteFunctionImp > >
  {
  public:
    typedef typename DiscreteFunctionImp :: LocalFunctionImp LocalFunctionImp;
    typedef typename DiscreteFunctionImp :: DiscreteFunctionSpaceType
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

    typedef typename DiscreteFunctionDefaultType :: LocalFunctionStorageType LFStorage;

  private:

    // local function storage stack
    LFStorage* storage_;

    // type of stack entry
    typedef typename LFStorage :: StackStorageType StackStorageType;

    // pair storing pointer to local function and poiner to ref-counter
    StackStorageType obj_;

  public:
    //! empty Constructor
    LocalFunctionWrapper() : storage_(0) , obj_ (0,0) {}

    //! Constructor initializing the underlying local function
    template < class EntityType >
    LocalFunctionWrapper(const EntityType & en, const DiscreteFunctionImp & df)
      : storage_( df.localFunctionStorage() )
        , obj_ ( storage_->getObject() )
    {
      // init real local function with entity
      localFunc().init( en );
    }

    //! Constructor creating empty local function
    LocalFunctionWrapper (const DiscreteFunctionImp & df)
      : storage_( df.localFunctionStorage() )
        , obj_( storage_->getObject() )
    {}

    //! Copy constructor
    LocalFunctionWrapper(const LocalFunctionWrapper& org)
      : storage_(org.storage_)
        , obj_( org.obj_ )
    {
      ++(*(obj_.second));
    }

    //! Destructor , push local function to stack if there are no other
    //! to it references
    ~LocalFunctionWrapper ()
    {
      removeObj();
    }

    //! Assignment operator
    LocalFunctionWrapper& operator=(const LocalFunctionWrapper& org)
    {
      if (this != &org) {
        removeObj();

        storage_ = org.storage_;
        obj_ = org.obj_;
        ++(*(obj_.second));
      }
      return *this;
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
    void init ( const EntityType &en ) const
    {
      localFunc().init(en);
    }

  private:
    LocalFunctionImp & localFunc()
    {
      assert( obj_.first );
      return *(obj_.first);
    }
    const LocalFunctionImp & localFunc() const
    {
      assert( obj_.first );
      return *(obj_.first);
    }

    //! method remove the obj by using the storage
    void removeObj ()
    {
      if(obj_.first)
      {
        assert( *(obj_.second) > 0);
        // the second counter is left at a value of 1, so we dont have to
        // initialize when getting the object again
        if( (*(obj_.second)) == 1) {
          storage_->freeObject ( obj_ );
        }
        else
          --(*(obj_.second));
      }
    }
  }; // end LocalFunctionWrapper

} // end namespace Dune

#endif
