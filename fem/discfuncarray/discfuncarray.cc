// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCFUNCARRAY_CC__
#define __DUNE_DISCFUNCARRAY_CC__

namespace Dune
{

  // Constructor makeing empty discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const char * name, DiscreteFunctionSpaceType & f) :
    DiscreteFunctionDefaultType ( f )
    , name_ ( name )
    , built_ ( false )
    , levOcu_ (0)
    , level_ (-1)
    , allLevels_ (false)
    , freeLocalFunc_ (NULL)
    , localFunc_ ( f, dofVec_ ) {}

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(DiscreteFunctionSpaceType & f,
                int level , int codim , bool allLevel )
    : DiscreteFunctionDefaultType ( f )
      , name_ ( "no name" )
      , level_ ( level ) ,
      allLevels_ ( allLevel ) , freeLocalFunc_ (NULL)
      , localFunc_ ( f , dofVec_ )
  {
    if(allLevels_)
      levOcu_ = level_+1;
    else
      levOcu_ = 1;

    getMemory();
  }

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const char * name, DiscreteFunctionSpaceType & f,
                int level , int codim , bool allLevel )
    : DiscreteFunctionDefaultType ( f )
      , name_ ( name )
      , level_ ( level ) ,
      allLevels_ ( allLevel ) , freeLocalFunc_ (NULL)
      , localFunc_ ( f , dofVec_ )
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
    DiscreteFunctionDefaultType ( df.functionSpace_ ) , localFunc_ ( df.localFunc_ )
  {
    name_ = df.name_;
    built_ = df.built_;
    levOcu_ = df.levOcu_;
    level_ = df.level_;
    allLevels_ = df.allLevels_;

    freeLocalFunc_ = NULL;
    dofVec_ = df.dofVec_;
  }


  // Desctructor
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  ~DiscFuncArray()
  {
    if(freeLocalFunc_) delete freeLocalFunc_;
  }


  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::setLevel ( RangeFieldType x, int level )
  {
    if(!allLevels_ && level != level_)
    {
      std::cout << "Level not set! \n";
      return;
    }
    int size = dofVec_[level].size();
    Array < RangeFieldType > &vec = dofVec_[level];
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::set ( RangeFieldType x )
  {
    if(allLevels_)
    {
      for(int l=0; l<level_; l++)
      {
        int size = dofVec_[l].size();
        Array < RangeFieldType > &vec = dofVec_[l];
        for(int i=0; i<size; i++)
          vec[i] = x;
      }
    }

    int size = dofVec_[level_].size();
    Array < RangeFieldType > &vec = dofVec_[level_];
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::clearLevel (int level )
  {
    setLevel(0.0,level);
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::print(std::ostream &s, int level ) const
  {
    s << "DiscFuncArray '" << name_ << "', level " << level << "\n";
    DofIteratorType enddof = this->dend ( level );
    for(DofIteratorType itdof = this->dbegin ( level ); itdof != enddof; ++itdof)
    {
      s << (*itdof) << " \n";
    }
  }
  //*************************************************************************
  //  Interface Methods
  //*************************************************************************
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void
  DiscFuncArray< DiscreteFunctionSpaceType >::
  localFunction ( EntityType &en , LocalFunctionArray < DiscreteFunctionSpaceType > &lf )
  {
    lf.init ( en );
  }

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray<DiscreteFunctionSpaceType>
  DiscFuncArray< DiscreteFunctionSpaceType >::
  newLocalFunction ( )
  {
    LocalFunctionArray<DiscreteFunctionSpaceType> tmp ( this->functionSpace_ , dofVec_ );
    return tmp;
  }

#if 0
  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::LocalFunctionType *
  DiscFuncArray< DiscreteFunctionSpaceType >::getLocalFunction ()
  {
    if(!freeLocalFunc_)
    {
      LocalFunctionType *lf = new LocalFunctionType (functionSpace_,dofVec_);
      return lf;
    }
    else
    {
      LocalFunctionType *lf = freeLocalFunc_;
      freeLocalFunc_ = lf->getNext();
      return lf;
    }
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  freeLocalFunction ( typename DiscFuncArray<DiscreteFunctionSpaceType>::LocalFunctionType *lf)
  {
    lf->setNext(freeLocalFunc_);
    freeLocalFunc_ = lf;
  }
#endif

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

  template<class DiscreteFunctionSpaceType >
  inline const typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dbegin ( int level ) const
  {
    DofIteratorType tmp ( dofVec_ [level] , 0 );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline const typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dend ( int level ) const
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

    xdrstdio_create(&xdrs, file, XDR_ENCODE);

    int allHelp = (allLevels_ == true) ? 1 : 0;
    // write allLevels to file , xdr_bool didnt work
    xdr_int (&xdrs, &allHelp);
    xdr_int (&xdrs, &level_);

    if(allLevels_)
    {
      for(int lev=0; lev<level_; lev++)
        dofVec_[lev].processXdr(&xdrs);
    }

    int lev = level_;
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

    int allHelp;
    // write allLevels to file
    xdr_int (&xdrs, &allHelp);
    allLevels_ = (allHelp == 0) ? false : true;

    // read max level on which function lives
    xdr_int (&xdrs, &level_);

    if(allLevels_)
    {
      levOcu_ = level_+1;
      getMemory();

      for(int lev=0; lev<=level_; lev++)
      {
        dofVec_[lev].processXdr(&xdrs);
      }
    }
    else
    {
      levOcu_ = 1;
      getMemory();

      int lev = level_;
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
        int length = this->functionSpace_.size( lev );
        outfile << length << "\n";
        DofIteratorType enddof = this->dend ( lev );
        for(DofIteratorType itdof = this->dbegin ( lev ); itdof != enddof; ++itdof)
        {
          outfile << (*itdof) << " ";
        }
        outfile << "\n";
      }
    }
    {
      int lev = level_;
      int length = this->functionSpace_.size( lev );
      outfile << length << "\n";
      DofIteratorType enddof = this->dend ( lev );
      for(DofIteratorType itdof = this->dbegin ( lev ); itdof != enddof; ++itdof)
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
        std::cout << this->functionSpace_.size( lev ) << "\n";
        if(length != this->functionSpace_.size( lev ))
        {
          std::cerr << "ERROR: wrong number of dofs stored in file!\n";
          abort();
        }
        DofIteratorType enddof = this->dend ( lev );
        for(DofIteratorType itdof = this->dbegin ( lev ); itdof != enddof; ++itdof)
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
      if(length != this->functionSpace_.size( lev ))
      {
        std::cerr << "ERROR: wrong number of dofs stored in file!\n";
        abort();
      }
      DofIteratorType enddof = this->dend ( lev );
      for(DofIteratorType itdof = this->dbegin ( lev ); itdof != enddof; ++itdof)
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
    const char * path=NULL;
    const char * fn = genFilename(path,filename,timestep);
    std::ofstream out( fn );

    enum { dim = GridType::dimension };

    int danz = 129;
    /*
       int danz = this->functionSpace_.getGrid().size(level_, dim );
       danz = (int) pow (( double ) danz, (double) (1.0/dim) );
       std::cout << danz << " Danz!\n";
     */

    out << "P2\n " << danz << " " << danz <<"\n255\n";
    DofIteratorType enddof = this->dend ( level_ );
    for(DofIteratorType itdof = this->dbegin ( level_ ); itdof != enddof; ++itdof) {
      out << (int)((*itdof)*255.) << "\n";
    }
    out.close();
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_pgm( const char *filename , int timestep )
  {
    FILE *in;
    int v;

    // get the hole memory
    level_ = this->functionSpace_.getGrid().maxlevel();
    allLevels_ = true;
    levOcu_ = level_+1;

    getMemory();
    const char * path=NULL;
    const char * fn = genFilename(path,filename,timestep);
    in = fopen( fn, "r" );
    fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
    DofIteratorType enddof = this->dend ( level_ );
    for(DofIteratorType itdof = this->dbegin ( level_ ); itdof != enddof; ++itdof) {
      fscanf( in, "%d", &v );
      (*itdof) = ((double)v)/255.;
    }
    fclose( in );
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_USPM( const char *filename , int timestep )
  {
    // USPM
    std::fstream out( filename , std::ios::out );
    //ElementType eltype = triangle;
    //out << eltype << " 1 1\n";
    int level = this->functionSpace_.getGrid().maxlevel();
    int length = this->functionSpace_.size( level );
    out << length << " 1 1\n";

    DofIteratorType enddof = this->dend ( level );
    for(DofIteratorType itdof = this->dbegin ( level );
        itdof != enddof; ++itdof)
    {
      out << (*itdof)  << "\n";
    }

    out.close();
    std::cout << "Written Dof to file `" << filename << "' !\n";
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  addScaled( int level,
             const DiscFuncArray<DiscreteFunctionSpaceType> &g,
             const RangeFieldType &scalar )
  {
    int length = dofVec_[level].size();

    Array<RangeFieldType> &v = dofVec_[level];
    const Array<RangeFieldType> &gvec = g.dofVec_[level];

    for(int i=0; i<length; i++)
      v[i] += scalar*gvec[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class GridIteratorType>
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  addScaledLocal( GridIteratorType &it ,
                  const DiscFuncArray<DiscreteFunctionSpaceType> &g, const RangeFieldType &scalar )
  {
    localFunction( *it , localFunc_ );

    DiscFuncArray<DiscreteFunctionSpaceType> &G =
      const_cast<DiscFuncArray<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(*it,G.localFunc_);

    int length = localFunc_.numberOfDofs();
    if(scalar == 1.)
    {
      for(int i=0; i<length; i++)
        localFunc_[i] += G.localFunc_[i];
    }
    else if ( scalar == -1. )
    {
      for(int i=0; i<length; i++)
        localFunc_[i] -= G.localFunc_[i];
    }
    else
    {
      for(int i=0; i<length; i++)
        localFunc_[i] += scalar * G.localFunc_[i];
    }
  }

  template<class DiscreteFunctionSpaceType >
  template<class GridIteratorType>
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  addLocal( GridIteratorType &it ,
            const DiscFuncArray<DiscreteFunctionSpaceType> &g)
  {
    localFunction( *it , localFunc_ );

    DiscFuncArray<DiscreteFunctionSpaceType> &G =
      const_cast<DiscFuncArray<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(*it,G.localFunc_);

    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] += G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class GridIteratorType>
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  substractLocal( GridIteratorType &it ,
                  const DiscFuncArray<DiscreteFunctionSpaceType> &g)
  {
    localFunction( *it , localFunc_ );

    DiscFuncArray<DiscreteFunctionSpaceType> &G =
      const_cast<DiscFuncArray<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(*it,G.localFunc_);

    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] -= G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class GridIteratorType>
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  setLocal( GridIteratorType &it , const RangeFieldType & scalar )
  {
    localFunction( *it , localFunc_ );
    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] = scalar;
  }
  //**********************************************************************
  //  --LocalFunctionArray
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::
  LocalFunctionArray( const DiscreteFunctionSpaceType &f ,
                      std::vector < Array < RangeFieldType > > & dofVec )
    : fSpace_ ( f ), dofVec_ ( dofVec )  , next_ (NULL)
      , baseFuncSet_ (NULL)
      , uniform_(true) {}

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::~LocalFunctionArray()
  {
    if(next_) delete next_;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename LocalFunctionArray < DiscreteFunctionSpaceType >::RangeFieldType &
  LocalFunctionArray < DiscreteFunctionSpaceType >::operator [] (int num)
  {
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType >
  inline const typename LocalFunctionArray < DiscreteFunctionSpaceType >::RangeFieldType &
  LocalFunctionArray < DiscreteFunctionSpaceType >::operator [] (int num) const
  {
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType >
  inline int LocalFunctionArray < DiscreteFunctionSpaceType >::
  numberOfDofs () const
  {
    return numOfDof_;
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  evaluate (EntityType &en, const DomainType & x, RangeType & ret) const
  {
    if(numOfDifferentDofs_ > 1) // i.e. polynom order > 0
    {
      ret = 0.0;
      xtmp_ = en.geometry().local(x);
      for(int i=0; i<numOfDifferentDofs_; i++)
      {
        bool eval = fSpace_.evaluateLocal(i,en,xtmp_,tmp_);
        if(eval)
        {
          for(int l=0; l<dimrange; l++)
            ret(l) += (* (values_[i])) * tmp_(l);
        }
      }
    }
    else
    {
      for(int l=0; l<dimrange; l++)
        ret(l) = (* (values_[ l ]));
    }
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType >
  template <class EntityType, class QuadratureType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  evaluate (EntityType &en, QuadratureType &quad, int quadPoint, RangeType & ret) const
  {
    if(numOfDifferentDofs_ > 1) // i.e. polynom order > 0
    {
      ret = 0.0;
      for(int i=0; i<numOfDifferentDofs_; i++)
      {
        bool eval = fSpace_.evaluateLocal(i,en,quad,quadPoint,tmp_);
        if(eval)
        {
          for(int l=0; l<dimrange; l++)
            ret[l] += (* (values_[i])) * tmp_[l];
        }
      }
    }
    else
    {
      for(int l=0; l<dimrange; l++)
        ret[l] = (* (values_[ l ]));
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
  inline bool LocalFunctionArray < DiscreteFunctionSpaceType >::
  init (EntityType &en ) const
  {
    if((!uniform_) || (!baseFuncSet_))
    {
      baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();
      numOfDifferentDofs_ = baseFuncSet_->getNumberOfDiffBaseFuncs();

      if(numOfDof_ > values_.size())
        values_.resize( numOfDof_ );
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &((dofVec_[ en.level() ])[fSpace_.mapToGlobal ( en , i)]);
    return true;
  }

  //**********************************************************************
  //
  //  DofIteratorArray
  //
  //**********************************************************************
  template <class DofType>
  inline DofType& DofIteratorArray<DofType>::operator *()
  {
    assert((count_ >=0) && (count_ < dofArray_.size()));
    return dofArray_ [ count_ ];
  }

  template <class DofType>
  inline const DofType& DofIteratorArray<DofType>::operator * () const
  {
    assert((count_ >=0) && (count_ < constArray_.size()));
    return constArray_ [ count_ ];
  }

  template <class DofType>
  inline DofIteratorArray<DofType>& DofIteratorArray<DofType>::operator ++()
  {
    count_++;
    return (*this);
  }

  template <class DofType>
  inline const DofIteratorArray<DofType>& DofIteratorArray<DofType>::operator ++() const
  {
    count_++;
    return (*this);
  }

  template <class DofType>
  inline DofType& DofIteratorArray<DofType>::operator [](int i)
  {
    assert((i >=0) && (i < dofArray_.size()));
    return dofArray_[i];
  }

  template <class DofType>
  inline const DofType& DofIteratorArray<DofType>::operator [](int i) const
  {
    assert((i >=0) && (i < constArray_.size()));
    return constArray_[i];
  }

  template <class DofType>
  inline bool DofIteratorArray<DofType>::
  operator ==(const DofIteratorArray<DofType> & I) const
  {
    return count_ == I.count_;
  }

  template <class DofType>
  inline bool DofIteratorArray<DofType>::
  operator !=(const DofIteratorArray<DofType> & I) const
  {
    return count_ != I.count_;
  }

  template <class DofType>
  inline int DofIteratorArray<DofType>::index() const
  {
    return count_;
  }

  template <class DofType>
  inline void DofIteratorArray<DofType>::reset() const
  {
    count_ = 0;
  }


} // end namespace

#endif
