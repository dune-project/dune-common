// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DFADAPT_CC__
#define __DUNE_DFADAPT_CC__

namespace Dune
{

  // Constructor makeing empty discrete function
  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(const char * name, DiscreteFunctionSpaceType & f) :
    DiscreteFunctionDefaultType ( f )
    , name_ (name)
    , built_ ( false )
    , level_ (-1), levOcu_ (0)
    , memObj_ ( f.signIn( *this ) )
    , dofVec_ ( memObj_.getArray() )
    , localFunc_ ( f, dofVec_ )
  {}

  // Constructor makeing discrete function
  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(const char * name, DiscreteFunctionSpaceType & f,
          int level , int codim , bool allLevel )
    : DiscreteFunctionDefaultType ( f )
      , name_ (name)
      , level_ ( level )
      , memObj_ (f.signIn( const_cast <DFAdapt< DiscreteFunctionSpaceType > &> (*this) ))
      , dofVec_ ( memObj_.getArray() )
      , localFunc_ ( f , dofVec_ )
  {}

  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(const DFAdapt <DiscreteFunctionSpaceType> & df ) :
    DiscreteFunctionDefaultType ( df.functionSpace_ ) , name_ ("copy")
    , memObj_ ( df.functionSpace_.signIn( const_cast <DFAdapt< DiscreteFunctionSpaceType > &> (*this) ))
    , dofVec_ ( memObj_.getArray() )
    , localFunc_ ( df.localFunc_ )
  {
    // copy values of array
    dofVec_ = df.dofVec_;

    built_ = df.built_;
    levOcu_ = df.levOcu_;
    level_ = df.level_;
  }


  // Desctructor
  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  ~DFAdapt()
  {
    //std::cout << "Deleting DF " << this->name() << "\n";
    bool removed = this->functionSpace_.signOut(const_cast<DFAdapt< DiscreteFunctionSpaceType > &> (*this)  );
    if(!removed)
    {
      std::cerr << "ERROR: removal of DF '" << name_ << "' failed!\n";
      assert(removed);
    }
  }


  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::setLevel ( RangeFieldType x, int level )
  {
    int size = dofVec_.size ();
    DofArrayType &vec = dofVec_;
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::set ( RangeFieldType x )
  {
    int size = dofVec_.size();
    DofArrayType &vec = dofVec_;
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::clearLevel (int level )
  {
    setLevel(0.0,level);
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::print(std::ostream &s, int level )
  {
    RangeFieldType sum = 0.;
    DofIteratorType enddof = dend ( level );
    //std::cout << functionSpace_.size(level) << " fs | real " << (dofVec_).size() << "\n";
    for(DofIteratorType itdof = dbegin ( level ); itdof != enddof; ++itdof)
    {
      s << (*itdof) << " DofValue \n";
      sum += ABS(*itdof);
    }
    s << "sum = " << sum << "\n";
  }
  //*************************************************************************
  //  Interface Methods
  //*************************************************************************
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void
  DFAdapt< DiscreteFunctionSpaceType >::
  localFunction ( EntityType &en , LocalFunctionAdapt < DiscreteFunctionSpaceType > &lf )
  {
    lf.init ( en );
  }

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionAdapt<DiscreteFunctionSpaceType>
  DFAdapt< DiscreteFunctionSpaceType >::
  newLocalFunction ( )
  {
    LocalFunctionAdapt<DiscreteFunctionSpaceType> tmp ( this->functionSpace_ , dofVec_ );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dbegin ( int level )
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dend ( int level )
  {
    DofIteratorType tmp ( dofVec_ , dofVec_.size() );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline const typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dbegin ( int level ) const
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline const typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dend ( int level ) const
  {
    DofIteratorType tmp ( dofVec_ , dofVec_.size() );
    return tmp;
  }
  //**************************************************************************
  //  Read and Write Methods
  //**************************************************************************
  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_xdr( const char *filename , int timestep )
  {
    FILE  *file;
    XDR xdrs;
    const char *path = 0;

    const char * fn  = genFilename(path,filename, timestep);
    file = fopen(fn, "wb");
    if (!file)
    {
      printf( "\aERROR in DFAdapt::write_xdr(..): couldnot open <%s>!\n", filename);
      fflush(stderr);
      return false;
    }

    xdrstdio_create(&xdrs, file, XDR_ENCODE);
    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);

    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  read_xdr( const char *filename , int timestep )
  {
    FILE   *file;
    XDR xdrs;
    const char *path = 0;

    const char * fn  = genFilename(path,filename, timestep);
    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn, "rb");
    if(!file)
    {
      printf( "\aERROR in DFAdapt::read_xdr(..): couldnot open <%s>!\n", filename);
      fflush(stderr);
      return(false);
    }

    // read xdr
    xdrstdio_create(&xdrs, file, XDR_DECODE);
    levOcu_ = 1;
    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_ascii( const char *filename , int timestep )
  {
    const char * path=0;
    const char * fn = genFilename(path,filename,timestep);
    std::fstream outfile( fn , std::ios::out );
    {
      int lev = level_;
      int length = this->functionSpace_.size( lev );
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
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  read_ascii( const char *filename , int timestep )
  {
    const char * path=0;
    const char * fn = genFilename(path,filename,timestep);
    FILE *infile=0;
    infile = fopen( fn, "r" );
    assert(infile != 0);
    {
      levOcu_ = 1;
      int lev = level_;
      int length;
      fscanf(infile,"%d \n",&length);
      assert(length == this->functionSpace_.size( lev ));

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
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_pgm( const char *filename , int timestep )
  {
    const char * path=0;
    const char * fn = genFilename(path,filename,timestep);
    std::ofstream out( fn );

    enum { dim = GridType::dimension };

    int danz = 129;
    /*
       int danz = functionSpace_.getGrid().size(level_, dim );
       danz = (int) pow (( double ) danz, (double) (1.0/dim) );
       std::cout << danz << " Danz!\n";
     */

    out << "P2\n " << danz << " " << danz <<"\n255\n";
    DofIteratorType enddof = dend ( level_ );
    for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
      out << (int)((*itdof)*255.) << "\n";
    }
    out.close();
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  read_pgm( const char *filename , int timestep )
  {
    FILE *in;
    int v;

    // get the hole memory
    level_ = this->functionSpace_.getGrid().maxlevel();
    levOcu_ = level_+1;

    //getMemory();
    const char * path=0;
    const char * fn = genFilename(path,filename,timestep);
    in = fopen( fn, "r" );
    fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
    DofIteratorType enddof = dend ( level_ );
    for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
      fscanf( in, "%d", &v );
      (*itdof) = ((double)v)/255.;
    }
    fclose( in );
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_USPM( const char *filename , int timestep )
  {
    // USPM
    std::fstream out( filename , std::ios::out );
    //ElementType eltype = triangle;
    //out << eltype << " 1 1\n";
    int level = this->functionSpace_.getGrid().maxlevel();
    int length = this->functionSpace_.size( level );
    out << length << " 1 1\n";

    DofIteratorType enddof = dend ( level );
    for(DofIteratorType itdof = dbegin ( level );
        itdof != enddof; ++itdof)
    {
      out << (*itdof)  << "\n";
    }

    out.close();
    std::cout << "Written Dof to file `" << filename << "' !\n";
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  addScaled( int level,
             const DFAdapt<DiscreteFunctionSpaceType> &g,
             const RangeFieldType &scalar )
  {
    int length = dofVec_.size();

    DofArrayType &v = dofVec_;
    const DofArrayType &gvec = g.dofVec_;

    for(int i=0; i<length; i++)
      v[i] += scalar*gvec[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  addScaledLocal( EntityType &en ,
                  const DFAdapt<DiscreteFunctionSpaceType> &g, const RangeFieldType &scalar )
  {
    localFunction( en , localFunc_ );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(en,G.localFunc_);

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
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  addLocal( EntityType &en ,
            const DFAdapt<DiscreteFunctionSpaceType> &g)
  {
    localFunction( en , localFunc_ );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(en,G.localFunc_);

    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] += G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  substractLocal( EntityType &en ,
                  const DFAdapt<DiscreteFunctionSpaceType> &g)
  {
    localFunction( en , localFunc_ );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunction(en,G.localFunc_);

    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] -= G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType >
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  setLocal( EntityType &en , const RangeFieldType & scalar )
  {
    localFunction( en , localFunc_ );
    int length = localFunc_.numberOfDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] = scalar;
  }

  //**********************************************************************
  //  --LocalFunctionAdapt
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionAdapt < DiscreteFunctionSpaceType >::
  LocalFunctionAdapt( const DiscreteFunctionSpaceType &f ,
                      DofArrayType & dofVec )
    : fSpace_ ( f ), dofVec_ ( dofVec )
      , baseFuncSet_ (0)
      , uniform_(true) {}

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionAdapt < DiscreteFunctionSpaceType >::~LocalFunctionAdapt()
  {}

  template<class DiscreteFunctionSpaceType >
  inline typename LocalFunctionAdapt < DiscreteFunctionSpaceType >::RangeFieldType &
  LocalFunctionAdapt < DiscreteFunctionSpaceType >::operator [] (int num)
  {
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType >
  inline const typename LocalFunctionAdapt < DiscreteFunctionSpaceType >::RangeFieldType &
  LocalFunctionAdapt < DiscreteFunctionSpaceType >::read (int num) const
  {
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType >
  inline int LocalFunctionAdapt < DiscreteFunctionSpaceType >::
  numberOfDofs () const
  {
    return numOfDof_;
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType >::
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
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType >::
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

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline bool LocalFunctionAdapt < DiscreteFunctionSpaceType >::
  init (EntityType &en ) const
  {
    if((!uniform_) || (!baseFuncSet_))
    {
      baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();
      numOfDifferentDofs_ = baseFuncSet_->getNumberOfDiffBaseFuncs();

      if(numOfDof_ > this->values_.size())
        this->values_.resize( numOfDof_ );
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &(this->dofVec_[fSpace_.mapToGlobal ( en , i)]);
    return true;
  }

  //**********************************************************************
  //
  //  DofIteratorAdapt
  //
  //**********************************************************************
  template <class DofType,class DofArrayType>
  inline DofType& DofIteratorAdapt<DofType,DofArrayType>::operator *()
  {
    return dofArray_ [ count_ ];
  }

  template <class DofType,class DofArrayType>
  inline const DofType& DofIteratorAdapt<DofType,DofArrayType>::read () const
  {
    return constArray_ [ count_ ];
  }

  template <class DofType,class DofArrayType>
  inline DofIteratorAdapt<DofType,DofArrayType>& DofIteratorAdapt<DofType,DofArrayType>::operator ++()
  {
    count_++;
    return (*this);
  }

  template <class DofType,class DofArrayType>
  inline const DofIteratorAdapt<DofType,DofArrayType>& DofIteratorAdapt<DofType,DofArrayType>::operator ++() const
  {
    const_cast<DofIteratorAdapt<DofType,DofArrayType>&> (*this).operator ++ ();
    return (*this);
  }

  template <class DofType,class DofArrayType>
  inline DofIteratorAdapt<DofType,DofArrayType>& DofIteratorAdapt<DofType,DofArrayType>::operator ++(int i)
  {
    count_ += i;
    return (*this);
  }

  template <class DofType,class DofArrayType>
  inline const DofIteratorAdapt<DofType,DofArrayType>& DofIteratorAdapt<DofType,DofArrayType>::operator ++(int i) const
  {
    count_ += i;
    return (*this);
  }

  template <class DofType,class DofArrayType>
  inline DofType& DofIteratorAdapt<DofType,DofArrayType>::operator [](int i)
  {
    return dofArray_[i];
  }

  template <class DofType,class DofArrayType>
  inline const DofType& DofIteratorAdapt<DofType,DofArrayType>::read(int i) const
  {
    return constArray_[i];
  }

  template <class DofType,class DofArrayType>
  inline bool DofIteratorAdapt<DofType,DofArrayType>::
  operator ==(const DofIteratorAdapt<DofType,DofArrayType> & I) const
  {
    return count_ == I.count_;
  }

  template <class DofType,class DofArrayType>
  inline bool DofIteratorAdapt<DofType,DofArrayType>::
  operator !=(const DofIteratorAdapt<DofType,DofArrayType> & I) const
  {
    return count_ != I.count_;
  }

  template <class DofType,class DofArrayType>
  inline int DofIteratorAdapt<DofType,DofArrayType>::index() const
  {
    return count_;
  }

  template <class DofType,class DofArrayType>
  inline void DofIteratorAdapt<DofType,DofArrayType>::reset() const
  {
    count_ = 0;
  }


} // end namespace

#endif
