// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETE_FUNCTION_CC__
#define __DUNE_DISCRETE_FUNCTION_CC__

#include <fstream>

namespace Dune
{


  //************************************************************
  //  Default Implementations
  //************************************************************
  // scalarProductDofs
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline typename DiscreteFunctionSpaceType::RangeField
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  scalarProductDofs( const DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
                         DofIteratorImp ,LocalFunctionIteratorImp, DiscreteFunctionImp > &g )
  {
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    RangeFieldType skp = 0.;

    int level = getFunctionSpace().getGrid().maxlevel();

    // get DofIterator from this
    DofIteratorImp endit = dend ( level );

    // hack
    DofIteratorImp git   = const_cast<DiscreteFunctionDefault &>( g ).dbegin ( level );
    // multiply
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      skp += (*it) * (*git);
      ++git;
    }
    return skp;
  }

  // assign
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp >::
  assign( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it = *git;
      ++git;
    }

    return *this;
  }

  // operator =
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator = ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it = *git;
      ++git;
    }

    return *this;
  }

  // operator +=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator += ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it += *git;
      ++git;
    }
    return *this;
  }

  // operator +=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline void DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  addScaled( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g ,
             const typename DiscreteFunctionSpaceType::RangeField &scalar)
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it += (scalar* (*git));
      ++git;
    }
  }

  // operator -=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator -= ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it -= *git;
      ++git;
    }
    return *this;
  }

  // operator *=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator *= ( const typename DiscreteFunctionSpaceType::RangeField & scalar )
  {
    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
      *it *= scalar;

    return *this;
  }

  // operator /=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator /= ( const typename DiscreteFunctionSpaceType::RangeField & scalar )
  {
    (*this) *= (1./scalar);
    return *this;
  }


  // add
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  add ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g ,
        typename DiscreteFunctionSpaceType::RangeField scalar )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it += (*git) * scalar;
      ++git;
    }
    return *this;
  }

  // clear
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline void DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >:: clearLevel ( int level  )
  {
    DofIteratorImp enddof = dend ( level_ );
    for(DofIteratorImp itdof = dbegin ( level_ );
        itdof != enddof; ++itdof)
    {
      *itdof = 0.;
    }
  }

  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline bool DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  write(const FileFormatType ftype, const char *filename, int timestep)
  {
    {
      enum { n = DiscreteFunctionSpaceType::DimDomain };
      enum { m = DiscreteFunctionSpaceType::DimRange };
      std::fstream file( filename , std::ios::out );
      StringType d = typeIdentifier<DomainFieldType>();
      StringType r = typeIdentifier<RangeFieldType>();

      file << d << " " << r << " ";
      file << n << " " << m << "\n";
      file << myId_ << " " << ftype << "\n";
      file.close();
    }

    if(ftype == xdr)
      return asImp().write_xdr(filename,timestep);
    if(ftype == ascii)
      return asImp().write_ascii(filename,timestep);
    if(ftype == pgm)
      return asImp().write_pgm(filename,timestep);

    return false;
  }

  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline bool DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  read(const char *filename, int timestep)
  {
    enum { tn = DiscreteFunctionSpaceType::DimDomain };
    enum { tm = DiscreteFunctionSpaceType::DimRange };
    std::fstream file ( filename , std::ios::in );
    int n,m;
    std::basic_string <char> r,d;
    std::basic_string <char> tr (typeIdentifier<RangeFieldType>());
    std::basic_string <char> td (typeIdentifier<DomainFieldType>());

    file >> d;
    file >> r;
    file >> n >> m;
    int id,type;
    file >> id >> type;
    FileFormatType ftype = static_cast<FileFormatType> (type);
    if((d != td) || (r != tr) || (n != tn) || (m != tm) )
    {
      std::cerr << d << " | " << td << " DomainField in read!\n";
      std::cerr << r << " | " << tr << " RangeField  in read!\n";
      std::cerr << n << " | " << tn << " in read!\n";
      std::cerr << m << " | " << tm << " in read!\n";
      std::cerr << "Can not initialize DiscreteFunction with wrong FunctionSpace! \n";
      abort();
    }
    file.close();

    if(ftype == xdr)
      return asImp().read_xdr(filename,timestep);
    if(ftype == ascii)
      return asImp().read_ascii(filename,timestep);
    if(ftype == pgm)
      return asImp().read_pgm(filename,timestep);

    std::cerr << ftype << " FileFormatType not supported at the moment! \n";
    abort();

    return false;
  };


} // end namespace Dune

#endif
