// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTION_HH__
#define __DUNE_DISCRETEFUNCTION_HH__

#include "../grid/common/grid.hh"
#include "../common/function.hh"
#include "../common/functionspace.hh"
#include "discretefunctionspace.hh"
#include "localfunctionarray.hh"
#include "dofiterator.hh"

//#include <fstream>

namespace Dune {


  /** @defgroup DiscreteFunction The Interface for DiscreteFunctions

      The DiscreteFunction is resposible for the dof storage. This can be
      done in various ways an is left to the user. The user has to derive his
      own implemenation from the DiscreteFunctionDefault class. If some of
      the implementations in the default class are for his dof storage
      unefficient, then one can overload this functions.

     @{
   */

  typedef std::basic_string <char> StringType;

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
  //! implemented. It contains an local function and a dof iterator which can
  //! iterator over all dofs of one level. Via the method access the local
  //! dofs and basfunction can be accessed for a given entity.
  //!
  //************************************************************************
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionInterface
    : public Function < DiscreteFunctionSpaceType,
          DiscreteFunctionInterface <DiscreteFunctionSpaceType,
              DofIteratorImp , DiscreteFunctionImp > >
  {
    // just for readability
    typedef Function < DiscreteFunctionSpaceType,
        DiscreteFunctionInterface <DiscreteFunctionSpaceType,
            DofIteratorImp , DiscreteFunctionImp > > FunctionType;
  public:
    //! remember the template types
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::GridType GridType;
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    };

    typedef typename DiscreteFunctionSpaceType::GridType GridType;
    typedef DofIteratorImp DofIteratorType;

    DiscreteFunctionInterface ( const DiscreteFunctionSpaceType &f )
      : FunctionType ( f ) {} ;


    //! access to the local function. Local functions can only be accessed
    //! for an existing entity.
    template <class EntityType>
    typename Traits<EntityType::codimension>::LocalFunctionIteratorType
    access (EntityType & en )
    {
      return asImp().access(en);
    }

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level.
    DofIteratorType dbegin ( int level )
    {
      return asImp().dbegin ( level );
    };

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level
    DofIteratorType dend ( int level )
    {
      return asImp().dend ( level );
    };

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
      class DofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionDefault
    : public DiscreteFunctionInterface
      <DiscreteFunctionSpaceType, DofIteratorImp, DiscreteFunctionImp >
  {

    typedef DiscreteFunctionInterface <DiscreteFunctionSpaceType,
        DofIteratorImp, DiscreteFunctionImp >  DiscreteFunctionInterfaceType;

    enum { myId_ = 0 };
  public:
    //! remember the used types
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
      typedef typename DiscreteFunctionSpaceType::DomainField DomainField;
    };

    typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    typedef typename DiscreteFunctionSpaceType::DomainField DomainField;
    typedef DofIteratorImp DofIteratorType;

    //! pass the function space to the interface class
    DiscreteFunctionDefault ( const DiscreteFunctionSpaceType & f ) :
      DiscreteFunctionInterfaceType ( f ) {};

    //! derived from Function
    //! search for element which contains point x an evaluate
    //! dof entity with en
    void evaluate ( const typename Traits<0>::Domain & ,
                    typename Traits<0>::Range &) const
    {
      // search element
    };


    //! evaluate an scalar product of the dofs of two DiscreteFunctions
    DiscreteFunctionSpaceType::RangeField scalarProductDofs( const DiscreteFunctionDefault &g );

    // assign
    Vector<DiscreteFunctionSpaceType::RangeField> &
    assign(const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // assign
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator = (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // add
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator += (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // substract
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator -= (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // multiply
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator *= (const DiscreteFunctionSpaceType::RangeField &scalar);

    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator /= (const DiscreteFunctionSpaceType::RangeField &scalar);

    // add
    Vector<DiscreteFunctionSpaceType::RangeField> &
    add(const Vector<DiscreteFunctionSpaceType::RangeField> &g ,
        DiscreteFunctionSpaceType::RangeField scalar );

    template <class EntityType>
    void assignLocal(EntityType &en, const DiscreteFunctionImp &g,
                     const DiscreteFunctionSpaceType::RangeField &scalar)
    {
      std::cout << "AssignLocal \n";
    }

    template <class EntityType>
    void addLocal(EntityType &en, const DiscreteFunctionImp &g,
                  const DiscreteFunctionSpaceType::RangeField &scalar)
    {
      std::cout << "AddLocal \n";
    }

    //! clear all dofs of a given level of the discrete function
    void clearLevel(int level );

    template <FileFormatType ftype>
    bool write(const char *filename, int timestep)
    {
      {
        enum { n = DiscreteFunctionSpaceType::DimDomain };
        enum { m = DiscreteFunctionSpaceType::DimRange };
        std::fstream file( filename , std::ios::out );
        StringType d = typeIdentifier<DomainField>();
        StringType r = typeIdentifier<RangeField>();

        file << d << " " << r << " ";
        file << n << " " << m << "\n";
        file << myId_ << " " << ftype << "\n";
      }

      if(ftype == xdr)
        return asImp().write_xdr(filename,timestep);
      if(ftype == ascii)
        return asImp().write_ascii(filename,timestep);
    }

    template <FileFormatType ftype>
    bool read(const char *filename, int timestep)
    {
      {
        enum { tn = DiscreteFunctionSpaceType::DimDomain };
        enum { tm = DiscreteFunctionSpaceType::DimRange };
        std::fstream file( filename , std::ios::in );
        int n,m;
        std::basic_string <char> r,d;
        std::basic_string <char> tr (typeIdentifier<RangeField>());
        std::basic_string <char> td (typeIdentifier<DomainField>());

        file >> d;
        file >> r;
        file >> n >> m;
        int id,type;
        file >> id >> type;
        FileFormatType ft = static_cast<FileFormatType> (type);
        if((d != td) || (r != tr) || (n != tn) || (m != tm) || (ft != ftype) )
        {
          std::cerr << d << " | " << td << " DomainField in read!\n";
          std::cerr << r << " | " << tr << " RangeField  in read!\n";
          std::cerr << n << " | " << tn << " in read!\n";
          std::cerr << m << " | " << tm << " in read!\n";
          std::cerr << ftype << " Wrong FileFormat! \n";
          std::cerr << "Can not initialize DiscreteFunction with wrong FunctionSpace!\n";
          abort();
        }
      }

      if(ftype == xdr)
        return asImp().read_xdr(filename,timestep);
      if(ftype == ascii)
        return asImp().read_ascii(filename,timestep);
    };


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
