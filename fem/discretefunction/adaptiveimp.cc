// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


namespace Dune {
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  AdaptiveFunctionImplementation(std::string name,
                                 const DiscreteFunctionSpaceType& spc) :
    spc_(spc),
    name_(name),
    dm_(DofManagerFactory<DofManagerType>::getDofManager(spc.grid())),
    memObj_(dm_.addDofSet(&dofVec_, spc.mapper(), name_)),
    dofVec_(memObj_.getArray())
  {}

  /*
     template <class DiscreteFunctionSpaceImp, class DofManagerImp>
     AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
     AdaptiveFunctionImplementation(std::string name,
                                 const DiscreteFunctionSpaceType& spc,
                                 MemObjectType& memObject) :
     spc_(spc),
     name_(name),
     dm_(DofManagerFactory<DofManagerType>::getDofManager(spc.grid())),
     memObj_(memObject),
     dofVec_(memObject.getArray())
     {}
   */
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  AdaptiveFunctionImplementation(const ThisType& other) :
    spc_(other.spc_),
    name_(std::string("copy of")+other.name_),
    dm_(other.dm_),
    memObj_(dm_.addDofSet(&dofVec_, other.spc_.mapper(), name_)),
    dofVec_(memObj_.getArray())
  {
    // copy values
    dofVec_ = other.dofVec_;
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  ~AdaptiveFunctionImplementation()
  {
    bool removed = dm_.removeDofSet(memObj_);
    assert(removed);
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  std::string
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  name() const
  {
    return name_;
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  DofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  dbegin()
  {
    return dofVec_.begin();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  DofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  dend()
  {
    return dofVec_.end();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  dbegin() const
  {
    return dofVec_.begin();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  ConstDofIteratorType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  dend() const
  {
    return dofVec_.end();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  LocalFunctionType
  AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp, DofManagerImp>::
  newLocalFunction()
  {
    return LocalFunctionType(spc_, dofVec_);
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
  localFunction(const EntityType& en, LocalFunctionType& lf)
  {
    lf.init(en);
  }

  //- Read/write methods
  template<class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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


  template<class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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

  template<class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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

  template<class DiscreteFunctionSpaceImp, class DofManagerImp>
  bool AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp,DofManagerImp>::
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
