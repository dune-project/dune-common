// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


namespace Dune {
  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  AdaptiveFunctionImplementation(std::string name,
                                 const DiscreteFunctionSpaceType& spc) :
    spc_(spc),
    name_(name),
    dm_(DofManagerFactory<DofManagerType>::getDofManager(spc.grid())),
    memPair_(dm_.addDofSet(&dofVec_, spc.mapper(), name_)),
    dofVec_(*memPair_.second)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  AdaptiveFunctionImplementation(std::string name,
                                 const DiscreteFunctionSpaceType& spc,
                                 DofStorageType& dofVec) :
    spc_(spc),
    name_(name),
    dm_(DofManagerFactory<DofManagerType>::getDofManager(spc.grid())),
    memObj_(std::pair<MemObjectInterface*, DofStorageType*>(0, 0)),
    dofVec_(dofVec)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  AdaptiveFunctionImplementation(const ThisType& other) :
    spc_(other.spc_),
    name_(std::string("copy of")+other.name_),
    dm_(other.dm_),
    memPair_(dm_.addDofSet(&dofVec_, other.spc_.mapper(), name_)),
    dofVec_(*memPair_.second)
  {
    // copy values
    dofVec_ = other.dofVec_;
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  ~AdaptiveFunctionImplementation()
  {
    if (memPair_.first) {
      bool removed = dm_.removeDofSet(*memPair_.first);
      assert(removed);
    }
  }

  template <class DiscreteFunctionSpaceImp>
  std::string
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  name() const
  {
    return name_;
  }

  template <class DiscreteFunctionSpaceImp>
  int AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  size() const
  {
    return dofVec_.size();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  DofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  dbegin()
  {
    return dofVec_.begin();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  DofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  dend()
  {
    return dofVec_.end();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  dbegin() const
  {
    return dofVec_.begin();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  dend() const
  {
    return dofVec_.end();
  }

  template <class DiscreteFunctionSpaceImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  LocalFunctionType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  newLocalFunction()
  {
    return LocalFunctionType(spc_, dofVec_);
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  localFunction(const EntityType& en, LocalFunctionType& lf)
  {
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
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
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
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
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
      int length = spc_.size();
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


  template<class DiscreteFunctionSpaceImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  read_ascii(std::string fn)
  {
    FILE *infile=0;
    infile = fopen( fn.c_str(), "r" );
    assert(infile != 0);
    {
      int length;
      fscanf(infile,"%d \n",&length);
      assert(length == spc_.size( ));

      DofIteratorType enddof = dend ( );
      for(DofIteratorType itdof = dbegin ( ); itdof != enddof; ++itdof)
      {
        fscanf(infile,"%le \n",& (*itdof));
      }
    }
    fclose(infile);
    return true;
  }

  template<class DiscreteFunctionSpaceImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  write_pgm(std::string fn)
  {
    std::ofstream out( fn.c_str() );

    enum { dim = GridType::dimension };

    if (out) {
      int danz = 129;

      out << "P2\n " << danz << " " << danz <<"\n255\n";
      DofIteratorType enddof = dend ();
      for(DofIteratorType itdof = dbegin (); itdof != enddof; ++itdof) {
        out << (int)((*itdof)*255.) << "\n";
      }
      out.close();
    }
    else {
      std::cerr << "Couldn't open file '"<<fn<<"' \n";
    }
    return true;
  }

  template<class DiscreteFunctionSpaceImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>::
  read_pgm(std::string fn)
  {
    FILE *in;
    int v;

    in = fopen( fn.c_str(), "r" );
    assert(in);

    fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
    DofIteratorType enddof = dend ();
    for(DofIteratorType itdof = dbegin (); itdof != enddof; ++itdof) {
      fscanf( in, "%d", &v );
      (*itdof) = ((double)v)/255.;
    }
    fclose( in );
    return true;
  }

} // end namespace Dune
