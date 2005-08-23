// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


namespace Dune {

  template <class DiscreteFunctionSpaceImp>
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::
  AdaptiveDiscreteFunction(const DiscreteFunctionSpaceType& f) :
    BaseType(f)
  {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>&
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::
  operator+=(const MappingType& g) {
    static_cast<BaseType&>(*this).operator+=(g);
    return *this;
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>&
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::
  operator-=(const MappingType& g) {
    static_cast<BaseType&>(*this).operator-=(g);
    return *this;
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>&
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::
  operator*=(const MappingType& g) {
    static_cast<BaseType&>(*this).operator*=(g);
    return *this;
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>&
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::
  operator/=(const MappingType& g) {
    static_cast<BaseType&>(*this).operator+=(g);
    return *this;
  }

  //- DofIterator methods
  template <class DiscreteFunctionSpaceImp>
  DofIteratorType
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::dbegin() {
    return dofVec_.dbegin();
  }

  template <class DisceteFunctionSpaceImp>
  DofIteratorType
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::dend() {
    return dofVec_.dend();
  }

  template <class DiscreteFunctionSpaceImp>
  ConstDofIteratorType
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::dbegin() const {
    return dofVec_.dbegin();
  }

  template <class DiscreteFunctionSpaceImp>
  ConstDofIteratorType
  AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>::dend() const {
    return dofVec_.dend();
  }

  //- Read/write methods
  template<class DiscreteFunctionSpaceImp >
  inline bool DFAdapt< DiscreteFunctionSpaceImp >::
  write_xdr( const std::basic_string<char> fn )
  {
    FILE  *file;
    XDR xdrs;
    file = fopen(fn.c_str(), "wb");
    if (!file)
    {
      printf( "\aERROR in DFAdapt::write_xdr(..): couldnot open <%s>!\n", fn.c_str());
      fflush(stderr);
      return false;
    }

    xdrstdio_create(&xdrs, file, XDR_ENCODE);
    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);

    return true;
  }

  template<class DiscreteFunctionSpaceImp >
  inline bool AdaptiveDiscreteFunction< DiscreteFunctionSpaceImp >::
  read_xdr( const std::basic_string<char> fn )
  {
    FILE   *file;
    XDR xdrs;
    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn.c_str() , "rb");
    if(!file)
    {
      printf( "\aERROR in AdaptiveDiscreteFunction::read_xdr(..): could not open <%s>!\n", fn.c_str());
      fflush(stderr);
      return(false);
    }

    // read xdr
    xdrstdio_create(&xdrs, file, XDR_DECODE);
    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);
    return true;
  }

  template<class DiscreteFunctionSpaceImp >
  inline bool AdaptiveDiscreteFunction< DiscreteFunctionSpaceImp >::
  write_ascii( const std::basic_string<char> fn )
  {
    std::fstream outfile( fn.c_str() , std::ios::out );
    if (!outfile)
    {
      printf( "\aERROR in AdaptiveDiscreteFunction::write_ascii(..): could not open <%s>!\n", fn.c_str());
      fflush(stderr);
      return false;
    }

    {
      int length = this->functionSpace_.size();
      outfile << length << "\n";
      DofIteratorType enddof = dend ( );
      for(DofIteratorType itdof = dbegin ( ); itdof != enddof; ++itdof)
      {
        outfile << (*itdof) << " ";
      }
      outfile << "\n";
    }

    outfile.close();
    return true;
  }


  template<class DiscreteFunctionSpaceImp >
  inline bool AdaptiveDiscreteFunction< DiscreteFunctionSpaceImp >::
  read_ascii( const std::basic_string<char> fn )
  {
    FILE *infile=0;
    infile = fopen( fn.c_str(), "r" );
    assert(infile != 0);
    {
      int length;
      fscanf(infile,"%d \n",&length);
      assert(length == this->functionSpace_.size( ));

      DofIteratorType enddof = dend ( );
      for(DofIteratorType itdof = dbegin ( ); itdof != enddof; ++itdof)
      {
        fscanf(infile,"%le \n",& (*itdof));
      }
    }
    fclose(infile);
    return true;
  }

  //- class AdaptiveLocalFunction
  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::AdaptiveLocalFunction() {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::~AdaptiveLocalFunction() {}

  template <class DiscreteFunctionSpaceImp>
  typename AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::operator[] (int num) {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  typename AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::ConstDofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::operator[] (int num) const {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  AdaptiveLocalFunction(const ThisType& other) {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  evaluateLocal(EntityType& en, const DomainType& x, RangeType& ret) {
    // * more to come
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  jacobianLocal(EntityType& en, const DomainType& x, JacobianRangeType& ret) {
    // * more to come
  }

} // end namespace Dune
