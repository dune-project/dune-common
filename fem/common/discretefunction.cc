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
                         DofIteratorImp ,LocalFunctionIteratorImp, DiscreteFunctionImp > &g ) const
  {
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;

    RangeFieldType skp = 0.;

    const DofIteratorImp endit = this->dend ();
    const DofIteratorImp git =  g.dbegin ();
    const DofIteratorImp it = this->dbegin();

    // multiply
    for(; it != endit; ++it,++git)
    {
      skp += (*it) * (*git);
    }

    return skp;
  }


  // assign on maxlevel
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp >::
  assign( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp it = this->dbegin();
    DofIteratorImp endit = this->dend ();
    const DofIteratorImp git = gc.dbegin ();

    for(; it != endit; ++it, ++git)
      *it = *git;

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

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    const DofIteratorImp git = gc.dbegin ();

    for(DofIteratorImp it = this->dbegin(); it != endit; ++it,++git )
    {
      *it = *git;
    }
    return *this;
  }

  // operator +=
  /** \todo This operator can add a discretefunction defined on all levels to another
   * one defined only on one level.  We should somehow issue a warning in this case.
   */
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator += ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    const DofIteratorImp git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += *git;
    }
    return *this;
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

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    const DofIteratorImp git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it -= *git;
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
    DofIteratorImp endit = this->dend ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it)
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

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );
    // we would need const_iterators.....

    DofIteratorImp endit = this->dend ();
    DofIteratorImp git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += (*git) * scalar;
    }
    return *this;
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
      file << this->functionSpace_.type() << " " << ftype << "\n";
      file << this->functionSpace_.polynomOrder() << "\n";
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

    if(!file)
    {
      std::cerr << "Couldn't open file <"<<filename<<"> \n";
      abort();
    }

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

    std::cerr << ftype << " FileFormatType not supported at the moment! in file " << __FILE__ << " line " << __LINE__ << "\n";
    abort();

    return false;
  };

} // end namespace Dune

#endif
