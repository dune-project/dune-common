// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETE_FUNCTION_CC__
#define __DUNE_DISCRETE_FUNCTION_CC__

namespace Dune
{


  //************************************************************
  //  Default Implementations
  //************************************************************
  // scalarProductDofs
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline DiscreteFunctionSpaceType::RangeField
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  scalarProductDofs( const DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
                         DofIteratorImp , DiscreteFunctionImp > &g )
  {
    typedef DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType , DofIteratorImp , DiscreteFunctionImp >
    DiscreteFunctionDefaultType ;

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
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  assign( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

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
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator = ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

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
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator += ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

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

  // operator -=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator -= ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

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
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator *= ( const DiscreteFunctionSpaceType::RangeField & scalar )
  {
    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
      *it *= scalar;

    return *this;
  }

  // operator /=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator /= ( const DiscreteFunctionSpaceType::RangeField & scalar )
  {
    (*this) *= (1./scalar);
    return *this;
  }


  // add
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  add ( const Vector< DiscreteFunctionSpaceType::RangeField > & g ,
        DiscreteFunctionSpaceType::RangeField scalar )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

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
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline void DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >:: clearLevel ( int level  )
  {
    DofIteratorImp enddof = dend ( level_ );
    for(DofIteratorImp itdof = dbegin ( level_ );
        itdof != enddof; ++itdof)
    {
      *itdof = 0.;
    }
  }

  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  template <FileFormatType ftype>
  inline bool DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  write(const char *filename, int timestep)
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

    file.close();
    if(ftype == xdr)
      return asImp().write_xdr(filename,timestep);
    if(ftype == ascii)
      return asImp().write_ascii(filename,timestep);
  }

  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  template <FileFormatType ftype>
  inline bool DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  read(const char *filename, int timestep)
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
        std::cerr << "Can not initialize DiscreteFunction with wrong FunctionSpace! \n";
        abort();
      }
      file.close();
    }

    if(ftype == xdr)
      return asImp().read_xdr(filename,timestep);
    if(ftype == ascii)
      return asImp().read_ascii(filename,timestep);
  };


} // end namespace Dune

#endif
