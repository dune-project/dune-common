// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTION_HH__
#define __DUNE_DISCRETEFUNCTION_HH__

#include <dune/grid/common/grid.hh>
#include <dune/common/function.hh>
#include <dune/common/functionspace.hh>
#include "discretefunctionspace.hh"

namespace Dune {

  /** @defgroup DiscreteFunction DiscreteFunction
      @ingroup FunctionCommon
      The DiscreteFunction is responsible for the dof storage. This can be
      done in various ways an is left to the user. The user has to derive his
      own implementation from the DiscreteFunctionDefault class. If some of
      the implementations in the default class are for ineffecient for the
      dof storage in the derived class these functions can be overloaded.

     @{
   */

  typedef std::basic_string <char> StringType;

  /** \brief ???
   * \todo Please doc me! */
  template <typename T>
  StringType typeIdentifier ()
  {
    StringType tmp = "unknown";
    return tmp;
  };

  template <>
  StringType typeIdentifier<float> ()
  {
    StringType tmp = "float";
    return tmp;
  };

  template <>
  StringType typeIdentifier<int> ()
  {
    StringType tmp = "int";
    return tmp;
  };

  template <>
  StringType typeIdentifier<double> ()
  {
    StringType tmp = "double";
    return tmp;
  };


  //************************************************************************
  //
  //  --DiscreteFunctionInterface
  //
  //! This is the minimal interface of a discrete function which has to be
  //! implemented. It contains a local function and a dof iterator which can
  //! iterate over all dofs of one level. Via the method access the local
  //! dofs and basis functions can be accessed for a given entity.
  //!
  //************************************************************************
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp,
      template <class , class> class LocalFunctionIteratorImp ,
      class DiscreteFunctionImp >
  class DiscreteFunctionInterface
    : public Function < DiscreteFunctionSpaceType,
          DiscreteFunctionInterface <DiscreteFunctionSpaceType,
              DofIteratorImp , LocalFunctionIteratorImp , DiscreteFunctionImp > >
  {
  public:
    //! types that we sometimes need outside
    typedef Function < DiscreteFunctionSpaceType,
        DiscreteFunctionInterface <DiscreteFunctionSpaceType,
            DofIteratorImp , LocalFunctionIteratorImp,  DiscreteFunctionImp > > FunctionType;

    //! ???
    typedef typename DiscreteFunctionSpaceType::Domain DomainType;
    //! ???
    typedef typename DiscreteFunctionSpaceType::Range RangeType;

    //! ???
    typedef typename DiscreteFunctionSpaceType::DomainField DomainFieldType;
    //! ???
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;

    //! remember the template types
    template <class GridIteratorType>
    struct Traits
    {
      //! ???
      typedef LocalFunctionIteratorImp<DiscreteFunctionImp,GridIteratorType> LocalFunctionIteratorType;
    };

    //! Type of the underlying grid
    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    //! Type of the Dof iterator
    typedef DofIteratorImp DofIteratorType;

    //* end of type declarations

    //! ???
    DiscreteFunctionInterface ( DiscreteFunctionSpaceType &f )
      : FunctionType ( f ) {} ;

    //! ???
    template <class GridIteratorType>
    LocalFunctionIteratorImp<DiscreteFunctionImp,GridIteratorType>
    localFunction ( GridIteratorType & it)
    {
      return asImp().localFunction( it );
    }

    //! the implementation of an iterator to iterate efficient
    //! over all dofs of a discrete function
    DofIteratorType dbegin ()
    {
      return asImp().dbegin ();
    };

    //! the implementation of an iterator to iterate efficient
    //! over all dofs of a discrete function
    DofIteratorType dend ()
    {
      return asImp().dend ();
    };

    //! const version of dbegin
    const DofIteratorType dbegin () const
    {
      return asImp().dbegin ();
    };

    //! const version of dend
    const DofIteratorType dend () const
    {
      return asImp().dend ();
    };

  private:
    // Barton-Nackman trick
    DiscreteFunctionImp &asImp()
    {
      return static_cast<DiscreteFunctionImp&>(*this);
    }

    //! const version of asImp
    const DiscreteFunctionImp &asImp() const
    {
      return static_cast<const DiscreteFunctionImp&>(*this);
    }
  };

  //*************************************************************************
  //
  //  --DiscreteFunctionDefault
  //
  //! Default implementation of the discrete function. This class provides
  //! is responsible for the dof storage. Different implementations of the
  //! discrete function use different dof storage.
  //! The default implementation provides +=, -= ans so on operators and
  //! a DofIterator access, which can run over all dofs in an efficient way.
  //! Furthermore with an entity you can access an local function to evaluate
  //! the discrete function by multiplying the dofs and the basefunctions.
  //!
  //*************************************************************************
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp,
      template <class , class> class LocalFunctionIteratorImp,
      class DiscreteFunctionImp >
  class DiscreteFunctionDefault
    : public DiscreteFunctionInterface
      <DiscreteFunctionSpaceType, DofIteratorImp,
          LocalFunctionIteratorImp, DiscreteFunctionImp >
  {

    typedef DiscreteFunctionInterface <DiscreteFunctionSpaceType,
        DofIteratorImp, LocalFunctionIteratorImp, DiscreteFunctionImp >  DiscreteFunctionInterfaceType;

    enum { myId_ = 0 };

  public:
    //! ???
    typedef typename DiscreteFunctionSpaceType::Domain DomainType;
    //! ???
    typedef typename DiscreteFunctionSpaceType::Range RangeType;

    //! ???
    typedef typename DiscreteFunctionSpaceType::DomainField DomainFieldType;
    //! ???
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;

    //! pass the function space to the interface class
    DiscreteFunctionDefault ( DiscreteFunctionSpaceType & f ) :
      DiscreteFunctionInterfaceType ( f ) {};

    //! Evaluate a scalar product of the dofs of two DiscreteFunctions
    //! on the top level of the underlying grid
    RangeFieldType scalarProductDofs( const DiscreteFunctionDefault &g ) const;

    //! Assignment on same as operator =
    Vector<RangeFieldType> &
    assign(const Vector<RangeFieldType> &g);

    //! Assignment operator, this = g
    Vector<RangeFieldType> & operator = (const Vector<RangeFieldType> &g);

    //! Addition of g to discrete function
    Vector<RangeFieldType> & operator += (const Vector<RangeFieldType> &g);

    //! substract g from discrete function
    Vector<RangeFieldType> &
    operator -= (const Vector<RangeFieldType> &g);

    //! multiply with scalar
    Vector<RangeFieldType> &
    operator *= (const RangeFieldType &scalar);

    //! Division by a scalar
    Vector<RangeFieldType> &
    operator /= (const RangeFieldType &scalar);

    //! add scalar * g to discrete function
    Vector<RangeFieldType> &
    add(const Vector<RangeFieldType> &g , RangeFieldType scalar );

    //! write disc func information file and write dofs to file+timestep
    //! this method use the write method of the implementation of the
    //! discrete function
    bool write(const FileFormatType ftype, const char *filename, int timestep, int precision = 6);

    //! same as write only read
    bool read(const char *filename, int timestep);

  private:
    // Barton-Nackman trick
    DiscreteFunctionImp &asImp()
    {
      return static_cast<DiscreteFunctionImp&>(*this);
    }
    const DiscreteFunctionImp &asImp() const
    {
      return static_cast<const DiscreteFunctionImp&>(*this);
    }

  }; // end class DiscreteFunctionDefault

  /** @} end documentation group */

} // end namespace Dune

#include "discretefunction.cc"

#endif
