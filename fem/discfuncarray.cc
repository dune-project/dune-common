// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCFUNCARRAY_CC__
#define __DUNE_DISCFUNCARRAY_CC__

namespace Dune
{

  // Constructor makeing empty discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const DiscreteFunctionSpaceType & f) :
    DiscreteFunctionDefaultType ( f ) , built_ ( false ) , freeLocalFunc_ (NULL) {}

  // Constructor makeing discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const DiscreteFunctionSpaceType & f,
                int level , int codim , bool allLevel )
    : DiscreteFunctionDefaultType ( f ) , level_ ( level ) ,
      allLevels_ ( allLevel ) , freeLocalFunc_ (NULL)
  {
    if(allLevels_)
      levOcu_ = level_+1;
    else
      levOcu_ = 1;

    getMemory();
  }

  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const DiscFuncArray <DiscreteFunctionSpaceType> & df ) :
    DiscreteFunctionDefaultType ( df.functionSpace_ )
  {
    built_ = df.built_;
    allLevels_ = df.allLevels_;
    levOcu_ = df.levOcu_;
    level_ = df.level_;

    dofVec_ = df.dofVec_;
    freeLocalFunc_ = NULL;
  }


  // Desctructor
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  ~DiscFuncArray()
  {
    if(freeLocalFunc_) delete freeLocalFunc_;
  }


  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::setLevel ( RangeField x, int level )
  {
    if(!allLevels_ && level != level_)
    {
      std::cout << "Level not set! \n";
      return;
    }
    int size = dofVec_[level].size();
    Array < RangeField > &vec = dofVec_[level];
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::set ( RangeField x )
  {
    if(allLevels_)
    {
      for(int l=0; l<level_; l++)
      {
        int size = dofVec_[l].size();
        Array < RangeField > &vec = dofVec_[l];
        for(int i=0; i<size; i++)
          vec[i] = x;
      }
    }

    int size = dofVec_[level_].size();
    Array < RangeField > &vec = dofVec_[level_];
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::clearLevel (int level )
  {
    set(0.0,level);
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::print(std::ostream &s, int level )
  {
    RangeField sum = 0.;
    int numLevel = const_cast<GridType &> (functionSpace_.getGrid()).maxlevel();
    DofIteratorType enddof = dend ( numLevel );
    for(DofIteratorType itdof = dbegin ( numLevel ); itdof != enddof; ++itdof)
    {
      s << (*itdof) << " DofValue \n";
      sum += *itdof;
    }
    s << "sum = " << sum << "\n";
  }
  //*************************************************************************
  //  Interface Methods
  //*************************************************************************
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::LocalFunctionType&
  DiscFuncArray< DiscreteFunctionSpaceType >::access (EntityType &en)
  {
    if(!freeLocalFunc_)
    {
      LocalFunctionType *lf = new LocalFunctionType (functionSpace_,dofVec_);
      lf->init(en);
      return (*lf);
    }
    else
    {
      LocalFunctionType *lf = freeLocalFunc_;
      freeLocalFunc_ = lf->getNext();
      lf->init(en);
      return (*lf);
    }
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  done ( typename DiscFuncArray<DiscreteFunctionSpaceType>::LocalFunctionType &lf)
  {
    lf.setNext(freeLocalFunc_);
    freeLocalFunc_ = &lf;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dbegin ( int level )
  {
    DofIteratorType tmp ( dofVec_ [level] , 0 );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dend ( int level )
  {
    DofIteratorType tmp ( dofVec_ [ level ] , dofVec_[ level ].size() );
    return tmp;
  }


  //**************************************************************************
  //  Read and Write Methods
  //**************************************************************************
  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_xdr( const char *filename , int timestep )
  {
    FILE  *file;
    XDR xdrs;
    const char *path = NULL;

    const char * fn  = genFilename(path,filename, timestep);
    file = fopen(fn, "wb");
    if (!file)
    {
      printf( "\aERROR in DiscFuncArray::write_xdr(..): couldnot open <%s>!\n", filename);
      fflush(stderr);
      return false;
    }

    xdrstdio_create(&xdrs, file, XDR_ENCODE);      /*  Hybrid-Mesh XDR schreiben  */

    // write allLevels to file
    xdr_bool(&xdrs, &allLevels_);

    if(allLevels_)
    {
      xdr_int(&xdrs, &level_);
      for(int lev=0; lev<level_; lev++)
      {
        int length = functionSpace_.size( lev );
        xdr_int(&xdrs, &length);
        dofVec_[lev].processXdr(&xdrs);
      }
    }

    int lev = level_;
    int length = functionSpace_.size( lev );
    xdr_int(&xdrs, &length);
    dofVec_[lev].processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);

    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_xdr( const char *filename , int timestep )
  {
    FILE   *file;
    XDR xdrs;
    const char *path = NULL;

    const char * fn  = genFilename(path,filename, timestep);
    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn, "rb");
    if(!file)
    {
      printf( "\aERROR in DiscFuncArray::read_xdr(..): couldnot open <%s>!\n", filename);
      fflush(stderr);
      return(false);
    }

    // read xdr
    xdrstdio_create(&xdrs, file, XDR_DECODE);

    // write allLevels to file
    xdr_bool(&xdrs, &allLevels_);

    if(allLevels_)
    {
      xdr_int(&xdrs, &level_);
      levOcu_ = level_+1;
      getMemory();

      for(int lev=0; lev<=level_; lev++)
      {
        int length = functionSpace_.size( lev );
        xdr_int(&xdrs, &length);
        dofVec_[lev].processXdr(&xdrs);
      }
    }
    else
    {
      levOcu_ = 1;
      getMemory();

      int lev = level_;
      int length = functionSpace_.size( lev );
      xdr_int(&xdrs, &length);
      dofVec_[lev].processXdr(&xdrs);
    }

    xdr_destroy(&xdrs);
    fclose(file);
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_ascii( const char *filename , int timestep )
  {
    const char * path=NULL;
    const char * fn = genFilename(path,filename,timestep);
    std::fstream outfile( fn , std::ios::out );
    outfile << allLevels_ <<  "\n";
    if(allLevels_)
    {
      outfile << level_ << "\n";
      for(int lev=0; lev<level_; lev++)
      {
        int length = functionSpace_.size( lev );
        outfile << length << "\n";
        DofIteratorType enddof = dend ( lev );
        for(DofIteratorType itdof = dbegin ( lev ); itdof != enddof; ++itdof)
        {
          outfile << (*itdof) << " ";
        }
        outfile << "\n";
      }
    }
    {
      int lev = level_;
      int length = functionSpace_.size( lev );
      outfile << length << "\n";
      DofIteratorType enddof = dend ( lev );
      for(DofIteratorType itdof = dbegin ( lev ); itdof != enddof; ++itdof)
      {
        outfile << (*itdof) << " ";
      }
      outfile << "\n";
    }

    outfile.close();
    return true;
  }


  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_ascii( const char *filename , int timestep )
  {
    const char * path=NULL;
    const char * fn = genFilename(path,filename,timestep);
    FILE *infile=NULL;
    infile = fopen( fn, "r" );
    if(!infile)
    {
      std::cerr << "Couldnt open file! "<< fn << "\n";
      abort();
    }
    fscanf(infile,"%d \n",&allLevels_);
    std::cout << "Got allLevels = " << allLevels_ << "\n";
    if(allLevels_)
    {
      fscanf(infile,"%d \n",&level_);
      levOcu_ = level_+1;
      getMemory();
      std::cout << "Got Levels = " << level_ << "\n";
      for(int lev=0; lev<=level_; lev++)
      {
        int length;
        fscanf(infile,"%d \n",&length);
        std::cout << "Got Size of Level = "<< length << " " << lev << "\n";
        std::cout << functionSpace_.size( lev ) << "\n";
        if(length != functionSpace_.size( lev ))
        {
          std::cerr << "ERROR: wrong number of dofs stored in file!\n";
          abort();
        }
        DofIteratorType enddof = dend ( lev );
        for(DofIteratorType itdof = dbegin ( lev ); itdof != enddof; ++itdof)
        {
          fscanf(infile,"%le \n",& (*itdof));
        }
      }
    }
    else
    {
      levOcu_ = 1;
      getMemory();
      int lev = level_;
      int length;
      fscanf(infile,"%d \n",&length);
      if(length != functionSpace_.size( lev ))
      {
        std::cerr << "ERROR: wrong number of dofs stored in file!\n";
        abort();
      }
      DofIteratorType enddof = dend ( lev );
      for(DofIteratorType itdof = dbegin ( lev ); itdof != enddof; ++itdof)
      {
        fscanf(infile,"%le \n",& (*itdof));
      }
    }
    fclose(infile);
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_pgm( const char *filename , int timestep )
  {
    std::ofstream out( filename );
    out << "P2\n " << DANZ+1 << " " <<DANZ+1 <<"\n255\n";
    DofIteratorType enddof = dend ( level_ );
    for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
      out << (int)((*itdof)*255.) << "\n";
    }
    out.close();
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_pgm( const char *filename , int timestep )
  {
    FILE *in;
    int v;
    in = fopen( filename, "r" );
    fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
    DofIteratorType enddof = dend ( level_ );
    for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
      fscanf( in, "%d", &v );
      (*itdof) = ((double)v)/255.;
    }
    fclose( in );
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_USPM( const char *filename , int timestep )
  {
    // USPM
    std::fstream out( filename , std::ios::out );
    //ElementType eltype = triangle;
    //out << eltype << " 1 1\n";
    int length = functionSpace_.size( level );
    out << length << " 1 1\n";

    DofIteratorType enddof = dend ( level );
    for(DofIteratorType itdof = dbegin ( level );
        itdof != enddof; ++itdof)
    {
      out << (*itdof)  << "\n";
    }

    out.close();
    std::cout << "Written Dof to file `" << filename << "' !\n";
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  addScaled( const DiscFuncArray<DiscreteFunctionSpaceType> &g,
             const RangeField &scalar )
  {
    int level = functionSpace_.getGrid().maxlevel();
    int length = dofVec_[level].size();

    Array<RangeField> &v = dofVec_[level];
    const Array<RangeField> &gvec = g.dofVec_[level];

    for(int i=0; i<length; i++)
      v[i] += scalar*gvec[i];
  }

  //**********************************************************************
  //  --LocalFunctionArray
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::
  LocalFunctionArray( const DiscreteFunctionSpaceType &f ,
                      std::vector < Array < RangeField > > & dofVec )
    : fSpace_ ( f ), dofVec_ ( dofVec )  , next_ (NULL)
      , baseFuncSet_ (NULL)
      , uniform_(true)
  {}

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::~LocalFunctionArray()
  {
    if(next_) delete next_;
  }

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::RangeField &
  LocalFunctionArray < DiscreteFunctionSpaceType >::operator [] (int num)
  {
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType >
  inline int LocalFunctionArray < DiscreteFunctionSpaceType >::
  numberOfDofs () const
  {
    return numOfDof_;
  }

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  evaluate (EntityType &en, const Domain & x, Range & ret)
  {
    ret = 0.0;
    for(int i=0; i<numOfDof_; i++)
    {
      baseFuncSet_->evaluate(i,diffVar,x,tmp);
      ret += this->operator [] (i) * tmp;
    }
  }

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType > *
  LocalFunctionArray < DiscreteFunctionSpaceType >::getNext () const
  {
    return next_;
  }

  template<class DiscreteFunctionSpaceType >
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  setNext (LocalFunctionArray < DiscreteFunctionSpaceType > *n)
  {
    next_ = n;
  }

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  init (EntityType &en )
  {
    if((!uniform_) || (!baseFuncSet_))
    {
      baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();

      if(numOfDof_ > values_.size())
        values_.resize( numOfDof_ );
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &((dofVec_[ en.level() ])[fSpace_.mapToGlobal ( en , i)]);
  }

} // end namespace

#endif
