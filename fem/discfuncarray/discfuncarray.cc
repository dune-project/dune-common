// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCFUNCARRAY_CC
#define DUNE_DISCFUNCARRAY_CC

namespace Dune
{

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const DiscreteFunctionSpaceType & f)
    : DiscreteFunctionDefaultType ( f )
      , name_ ( "no name" )
      , localFunc_ ( f , dofVec_ )
  {
    getMemory();
  }

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const char * name, const DiscreteFunctionSpaceType & f )
    : DiscreteFunctionDefaultType ( f )
      , name_ ( name )
      , localFunc_ ( f , dofVec_ )
  {
    getMemory();
  }

  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  DiscFuncArray(const DiscFuncArray <DiscreteFunctionSpaceType> & df ) :
    DiscreteFunctionDefaultType ( df.functionSpace_ )
    , localFunc_ ( df.localFunc_ )
  {
    name_ = df.name_;
    built_ = df.built_;

    dofVec_ = df.dofVec_;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::getMemory()
  {
    // the last level is done always
    int length = this->functionSpace_.size();
    dofVec_.resize( length );
    for( int j=0; j<length; j++) dofVec_[j] = 0.0;
    built_ = true;
  }

  // Desctructor
  template<class DiscreteFunctionSpaceType >
  inline DiscFuncArray< DiscreteFunctionSpaceType >::
  ~DiscFuncArray()
  {}


  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::set ( RangeFieldType x )
  {
    for(int i=0; i<dofVec_.size(); i++)
      dofVec_[i] = x;
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::print(std::ostream &s ) const
  {
    s << "DiscFuncArray '" << name_ << "'\n";
    ConstDofIteratorType enddof = this->dend ();
    for(ConstDofIteratorType itdof = this->dbegin (); itdof != enddof; ++itdof)
    {
      //s << (*itdof) << " \n";
      printf("%3.15e \n", *itdof);
    }
  }
  //*************************************************************************
  //  Interface Methods
  //*************************************************************************
  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void
  DiscFuncArray< DiscreteFunctionSpaceType >::
  localFunction ( const EntityType &en , LocalFunctionType &lf )
  {
    lf.init ( en );
  }

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline typename DiscFuncArray< DiscreteFunctionSpaceType >:: LocalFunctionType
  DiscFuncArray< DiscreteFunctionSpaceType >:: localFunction ( const EntityType &en ) const
  {
    return LocalFunctionType (en,*this);
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray< DiscreteFunctionSpaceType >:: LocalFunctionImp *
  DiscFuncArray< DiscreteFunctionSpaceType >::
  newLocalFunctionObject () const

  {
    return new LocalFunctionArray<DiscreteFunctionSpaceType> ( this->functionSpace_ , dofVec_ );
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray< DiscreteFunctionSpaceType >:: LocalFunctionType
  DiscFuncArray< DiscreteFunctionSpaceType >:: newLocalFunction ()
  {
    return LocalFunctionType (*this);
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dbegin ()
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    return tmp;
  }


  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::DofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dend ( )
  {
    DofIteratorType tmp ( dofVec_  , dofVec_.size() );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dbegin ( ) const
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    ConstDofIteratorType tmp2(tmp);
    return tmp2;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DiscFuncArray<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DiscFuncArray< DiscreteFunctionSpaceType >::dend ( ) const
  {
    DofIteratorType tmp ( dofVec_ , dofVec_.size() );
    ConstDofIteratorType tmp2(tmp);
    return tmp2;
  }
  //**************************************************************************
  //  Read and Write Methods
  //**************************************************************************
  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_xdr( const char *fn )
  {
    FILE  *file;
    XDR xdrs;

    file = fopen(fn, "wb");
    if (!file)
    {
      printf( "\aERROR in DiscFuncArray::write_xdr(..): couldnot open <%s>!\n", fn);
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
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_xdr( const char *fn )
  {
    FILE   *file;
    XDR xdrs;

    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn, "rb");
    if(!file)
    {
      printf( "\aERROR in DiscFuncArray::read_xdr(..): couldnot open <%s>!\n", fn);
      fflush(stderr);
      return(false);
    }

    // read xdr
    xdrstdio_create(&xdrs, file, XDR_DECODE);

    getMemory();

    dofVec_.processXdr(&xdrs);

    xdr_destroy(&xdrs);
    fclose(file);
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_ascii( const char *fn )
  {
    std::fstream outfile( fn , std::ios::out );
    if(outfile)
    {
      int length = this->functionSpace_.size();
      outfile << length << std::endl;
      DofIteratorType enddof = this->dend ();
      for(DofIteratorType itdof = this->dbegin (); itdof != enddof; ++itdof)
      {
        outfile << (*itdof) << std::endl;
      }
      outfile.close();
    }
    else
    {
      fprintf(stderr,"\aERROR in DiscFuncArray::read_xdr(..): couldnot open <%s>!\n", fn);
      fflush(stderr);
      return(false);
    }
    return true;
  }


  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_ascii( const char *fn )
  {
    FILE *infile=NULL;
    infile = fopen( fn, "r" );
    if(!infile)
    {
      std::cerr << "Couldnt open file! "<< fn << "\n";
      abort();
    }
    {
      getMemory();
      int length;
      fscanf(infile,"%d \n",&length);
      if(length != this->functionSpace_.size())
      {
        std::cerr << "ERROR: wrong number of dofs stored in file!\n";
        abort();
      }
      DofIteratorType enddof = this->dend ();
      for(DofIteratorType itdof = this->dbegin (); itdof != enddof; ++itdof)
      {
        fscanf(infile,"%le \n",& (*itdof));
      }
    }
    fclose(infile);
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  write_pgm( const char *fn )
  {
    std::ofstream out( fn );

    enum { dim = GridType::dimension };

    int danz = 129;
    /*
       int danz = this->functionSpace_.getGrid().size(level_, dim );
       danz = (int) pow (( double ) danz, (double) (1.0/dim) );
       std::cout << danz << " Danz!\n";
     */

    out << "P2\n " << danz << " " << danz <<"\n255\n";
    DofIteratorType enddof = this->dend ( );
    for(DofIteratorType itdof = this->dbegin ( ); itdof != enddof; ++itdof) {
      out << (int)((*itdof)*255.) << "\n";
    }
    out.close();
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DiscFuncArray< DiscreteFunctionSpaceType >::
  read_pgm( const char *fn )
  {
    FILE *in;
    int v;

    getMemory();
    in = fopen( fn, "r" );
    assert(in);
    fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
    DofIteratorType enddof = this->dend ( );
    for(DofIteratorType itdof = this->dbegin ( ); itdof != enddof; ++itdof) {
      fscanf( in, "%d", &v );
      (*itdof) = ((double)v)/255.;
    }
    fclose( in );
    return true;
  }


  template<class DiscreteFunctionSpaceType >
  inline void DiscFuncArray< DiscreteFunctionSpaceType >::
  addScaled( const DiscFuncArray<DiscreteFunctionSpaceType> &g,
             const RangeFieldType &scalar )
  {
    int length = dofVec_.size();
    const Array<RangeFieldType> &gvec = g.dofVec_;
    assert(length == gvec.size());

    for(int i=0; i<length; i++)
      dofVec_[i] += scalar*gvec[i];
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
                      Array < RangeFieldType > & dofVec )
    : fSpace_ ( f ), dofVec_ ( dofVec )
      , uniform_(true), init_(false) {}

  template<class DiscreteFunctionSpaceType >
  inline LocalFunctionArray < DiscreteFunctionSpaceType >::~LocalFunctionArray()
  {}

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

  template<class DiscreteFunctionSpaceType >
  inline int LocalFunctionArray < DiscreteFunctionSpaceType >::
  numDofs () const
  {
    return numOfDof_;
  }

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  evaluate (EntityType &en, const DomainType & x, RangeType & ret) const
  {
    //if(numOfDifferentDofs_ > 1) // i.e. polynom order > 0
    // {
    ret = 0.0;
    DomainType xtmp_ = en.geometry().local(x);
    for(int i=0; i<numOfDifferentDofs_; i++)
    {
      RangeType tmp_;
      fSpace_.evaluateLocal(i,en,xtmp_,tmp_);
      for(int l=0; l<dimrange; l++)
        ret[l] += (* (values_[i])) * tmp_[l];
    }
    //  }
    // else
    //  {
    //    for(int l=0; l<dimrange; l++)
    //      ret[l] = *(values_[l]) ;
    //  }
  }

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
        RangeType tmp_;
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

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline void LocalFunctionArray < DiscreteFunctionSpaceType >::
  init (const EntityType &en ) const
  {
    if(!uniform_ || !init_)
    {
      numOfDof_ =
        fSpace_.getBaseFunctionSet(en).numBaseFunctions();
      numOfDifferentDofs_ =
        fSpace_.getBaseFunctionSet(en).getNumberOfDiffBaseFuncs();

      if(numOfDof_ > values_.size())
        values_.resize( numOfDof_ );

      init_ = true;
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &(dofVec_[fSpace_.mapToGlobal ( en , i)]);
    return ;
  }

  //**********************************************************************
  //
  //  DofIteratorArray
  //
  //**********************************************************************
  template <class DofType>
  DofIteratorArray<DofType>::DofIteratorArray(const DofIteratorArray<DofType>& other) :
    dofArray_(other.dofArray_),
    count_(other.count_) {}

  template <class DofType>
  DofIteratorArray<DofType>&
  DofIteratorArray<DofType>::operator= (const DofIteratorArray<DofType>& other) {
    if (&other != this) {
      dofArray_ = other.dofArray_;
      count_ = other.count_;
    }
    return *this;
  }

  template <class DofType>
  inline DofType& DofIteratorArray<DofType>::operator *()
  {
    assert((count_ >=0) && (count_ < dofArray_->size()));
    return (*dofArray_) [ count_ ];
  }

  template <class DofType>
  inline const DofType& DofIteratorArray<DofType>::operator * () const
  {
    assert((count_ >=0) && (count_ < dofArray_->size()));
    return (*dofArray_) [ count_ ];
  }

  template <class DofType>
  inline DofIteratorArray<DofType>& DofIteratorArray<DofType>::operator ++()
  {
    ++count_;
    return (*this);
  }

  template <class DofType>
  inline DofType& DofIteratorArray<DofType>::operator [](int i)
  {
    assert((i >=0) && (i < dofArray_->size()));
    return (*dofArray_)[i];
  }

  template <class DofType>
  inline const DofType& DofIteratorArray<DofType>::operator [](int i) const
  {
    assert((i >=0) && (i < dofArray_->size()));
    return (*dofArray_)[i];
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
  inline void DofIteratorArray<DofType>::reset()
  {
    count_ = 0;
  }


} // end namespace

#endif
