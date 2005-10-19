// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LOCALFUNCTIONWRAPPER_HH
#define DUNE_LOCALFUNCTIONWRAPPER_HH

#include <stack>

#include "../common/discretefunction.hh"

namespace Dune {

  //! Stores pointers of local functions in a stack
  template <class DiscreteFunctionImp >
  class LocalFunctionStorage
  {
  private:
    typedef LocalFunctionStorage < DiscreteFunctionImp > MyType;
    typedef DiscreteFunctionImp DiscreteFunctionType;
    typedef typename DiscreteFunctionImp ::  LocalFunctionImp LocalFunctionImp;
    std::stack < LocalFunctionImp * > lfStack_;
    DiscreteFunctionType & df_;

  public:
    //! constructor
    LocalFunctionStorage (DiscreteFunctionType & df) : df_(df) {}

    //! delete all objects on stack
    ~LocalFunctionStorage ()
    {
      while ( !lfStack_.empty() )
      {
        LocalFunctionImp* lf = lfStack_.top();
        lfStack_.pop();
        //std::cout << "Deleting " << lf << "\n";
        if( lf ) delete lf;
      }
    }

    //! get local function object
    LocalFunctionImp* getObject ()
    {
      if( lfStack_.empty() )
      {
        return df_.newLocalFunctionObject();
      }
      else
      {
        LocalFunctionImp* lf = lfStack_.top();
        lfStack_.pop();
        return lf;
      }
    }

    //! push local function to stack
    void freeObject ( LocalFunctionImp* lf )
    {
      lfStack_.push(lf);
    }

  private:
    //! prohibited methods
    LocalFunctionStorage ( const MyType & c) : df_(c.df_) {};
    MyType & operator = ( const MyType & c ) { return *this; }
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
          LocalFunctionWrapper < DiscreteFunctionImp  > >
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

    // real local function implementation
    LocalFunctionImp* lf_;

    // reference counter
    int* refCount_;

  public:
    //! Constructor initializing the underlying local function
    template < class EntityType >
    LocalFunctionWrapper (const EntityType & en , DiscreteFunctionImp & df) :
      storage_( df.localFunctionStorage() ),
      lf_( storage_->getObject() ),
      refCount_(new int (1))
    {
      // init real local function with entity
      lf_->init(en);
    }

    //! Constructor creating empty local function
    LocalFunctionWrapper ( DiscreteFunctionImp & df )
      : storage_( df.localFunctionStorage() ) ,
        lf_( storage_->getObject() ),
        refCount_(new int (1))
    {}

    //! Copy constructor
    LocalFunctionWrapper(const LocalFunctionWrapper& org) :
      storage_(org.storage_),
      lf_(org.lf_),
      refCount_(org.refCount_)
    {
      ++(*refCount_);
    }

    //! Destructor , push local function to stack if there are no other
    //! to it references
    ~LocalFunctionWrapper () {
      assert(*refCount_ > 0);
      --(*refCount_);
      if (*refCount_ == 0) {
        storage_->freeObject ( lf_ );
        delete refCount_;
      }
    }

    //! Assignment operator
    LocalFunctionWrapper& operator=(const LocalFunctionWrapper& org) {
      //assert(storage_ == org.storage_); // only assign local functions belonging to the same discrete function ! why not! perfectly safe to do it!
      if (this != &org) {
        --(*refCount_);
        if (*refCount_ == 0) {
          storage_->freeObject(lf_);
          delete refCount_;
        }

        storage_ = org.storage_;
        lf_ = org.lf_;
        refCount_ = org.refCount_;
        ++(*refCount_);
      }
      return *this;
    }

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num) { return (*lf_)[num]; }

    //! access to dof number num, all dofs of the dof entity
    const RangeFieldType & operator [] (int num) const { return (*lf_)[num]; }

    //! return number of degrees of freedom
    int numDofs () const { return lf_->numDofs(); }

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const
    {
      lf_->evaluate( en , x , ret );
    }

    //! sum over all local base functions but local
    template <class EntityType>
    void evaluateLocal(EntityType &en, const DomainType & x, RangeType & ret) const
    {
      lf_->evaluateLocal( en , x , ret );
    }

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const
    {
      lf_->evaluate( en , quad, quadPoint , ret );
    }

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void jacobian (EntityType &en, QuadratureType &quad, int quadPoint , JacobianRangeType & ret) const
    {
      lf_->jacobian( en, quad, quadPoint, ret );
    }

    //! sum over all local base functions evaluated on given quadrature
    //point, but local
    template <class EntityType>
    void jacobianLocal(EntityType& en, const DomainType& x, JacobianRangeType& ret) const
    {
      lf_->jacobianLocal( en, x , ret );
    }

    //! sum over all local base functions evaluated on given point x
    template <class EntityType>
    void jacobian(EntityType& en, const DomainType& x, JacobianRangeType& ret) const
    {
      lf_->jacobian( en, x, ret);
    }

    //! update local function for given Entity
    //! deprecated method
    template <class EntityType >
    void init ( const EntityType &en ) const
    {
      lf_->init(en);
    }

  }; // end LocalFunctionWrapper

} // end namespace Dune

#endif
