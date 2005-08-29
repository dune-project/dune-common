// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


namespace Dune {
  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  AdaptiveFunctionImplementation(std::string name,
                                 DiscreteFunctionSpaceType& spc) :
    spc_(spc),
    memObj_(spc.signIn(*this))
    dofVec_(memObj_.getArray()),
    name_(name)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  ~AdaptiveFunctionImplementation() {
    bool removed = spc_.signOut(*this);

    assert(removed);
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::DofIterator
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::dbegin() {
    return dofVec_.dbegin();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::DofIterator
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::dend() {
    return dofVec_.dend();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::dbegin() const {
    return dofVec_.dbegin();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::dend() const {
    return dofVec_.dend();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::LocalFunctionType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  newLocalFunction() {
    return LocalFunctionType(spc_, dofVec_);
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  localFunction(const EntityType& en, LocalFunctionType& lf) {
    lf.init(en);
  }

  //- Read/write methods
  template<class DiscreteFunctionSpaceImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  write_xdr(std::string fn)
  {
    FILE  *file;
    XDR xdrs;
    file = fopen(fn.c_str(), "wb");
    if (!file)
    {
      printf( "\aERROR in AdaptiveDiscreteFunction::write_xdr(..): could not open <%s>!\n", fn.c_str());
      fflush(stderr);
      return false;
    }

    xdrstdio_create(&xdrs, file, XDR_ENCODE);
    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);

    return true;
  }

  template <class DiscreteFunctionSpaceImp>
  inline bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  read_xdr(std::string fn)
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

  template <class DiscreteFunctionSpaceImp>
  inline bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  write_ascii(std::string fn)
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
  inline bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  read_ascii(std::string fn)
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

} // end namespace Dune
