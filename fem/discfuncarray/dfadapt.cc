// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DFADAPT_CC
#define DUNE_DFADAPT_CC

#include <algorithm>

namespace Dune
{

  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(DiscreteFunctionSpaceType& f) :
    DiscreteFunctionDefaultType ( f )
    , name_ ("no name")
    , memObj_ (f.signIn(*this))
    , dofVec_ ( memObj_.getArray() )
    , localFunc_ ( f , dofVec_ )
  {}

  // Constructor makeing discrete function
  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(const char * name, DiscreteFunctionSpaceType & f)
    : DiscreteFunctionDefaultType ( f )
      , name_ (name)
      , memObj_ (f.signIn(*this))
      , dofVec_ ( memObj_.getArray() )
      , localFunc_ ( f , dofVec_ )
  {}

  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  DFAdapt(const DFAdapt <DiscreteFunctionSpaceType>& df ) :
    DiscreteFunctionDefaultType ( df.functionSpace_ ),
    name_ ("copy"),
    memObj_ ( df.functionSpace_.signIn(*this) ),
    dofVec_ ( memObj_.getArray() ),
    localFunc_ ( df.localFunc_ )
  {
    // copy values of array
    dofVec_ = df.dofVec_;
  }


  // Desctructor
  template<class DiscreteFunctionSpaceType >
  inline DFAdapt< DiscreteFunctionSpaceType >::
  ~DFAdapt()
  {
    //std::cout << "Deleting DF " << this->name() << "\n";
    // * Temporary hack (burriad)
    const FunctionSpaceType& spc = this->functionSpace_;
    bool removed = spc.signOut(*this);
    if(!removed)
    {
      std::cerr << "ERROR: removal of DF '" << name_ << "' failed!\n";
      assert(removed);
    }
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
  inline void DFAdapt< DiscreteFunctionSpaceType >::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::print(std::ostream &s )
  {
    RangeFieldType sum = 0.;
    DofIteratorType enddof = dend ( );
    for(DofIteratorType itdof = dbegin ( ); itdof != enddof; ++itdof)
    {
      s << (*itdof) << " DofValue \n";
      sum += std::abs(*itdof);
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
  DFAdapt< DiscreteFunctionSpaceType >::dbegin ( )
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dend ()
  {
    DofIteratorType tmp ( dofVec_ , dofVec_.size() );
    return tmp;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DFAdapt<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dbegin ( ) const
  {
    DofIteratorType tmp ( dofVec_ , 0 );
    ConstDofIteratorType tmp2(tmp);
    return tmp2;
  }

  template<class DiscreteFunctionSpaceType >
  inline typename DFAdapt<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DFAdapt< DiscreteFunctionSpaceType >::dend () const
  {
    DofIteratorType tmp ( dofVec_ , dofVec_.size() );
    ConstDofIteratorType tmp2(tmp);
    return tmp2;
  }
  //**************************************************************************
  //  Read and Write Methods
  //**************************************************************************
  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_xdr( const char *fn )
  {
    FILE  *file;
    XDR xdrs;
    file = fopen(fn, "wb");
    if (!file)
    {
      printf( "\aERROR in DFAdapt::write_xdr(..): couldnot open <%s>!\n", fn);
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
  read_xdr( const char *fn )
  {
    FILE   *file;
    XDR xdrs;
    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn, "rb");
    if(!file)
    {
      printf( "\aERROR in DFAdapt::read_xdr(..): couldnot open <%s>!\n", fn);
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

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_ascii( const char *fn )
  {
    std::fstream outfile( fn , std::ios::out );
    if (!outfile)
    {
      printf( "\aERROR in DFAdapt::write_ascii(..): couldnot open <%s>!\n", fn);
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


  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  read_ascii( const char *fn )
  {
    FILE *infile=0;
    infile = fopen( fn, "r" );
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

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  write_pgm( const char *fn )
  {
    std::ofstream out( fn );

    enum { dim = GridType::dimension };

    if(out)
    {
      int danz = 129;

      out << "P2\n " << danz << " " << danz <<"\n255\n";
      DofIteratorType enddof = dend ();
      for(DofIteratorType itdof = dbegin (); itdof != enddof; ++itdof) {
        out << (int)((*itdof)*255.) << "\n";
      }
      out.close();
    }
    else
    {
      std::cerr << "Couldn't open file '"<<fn<<"' \n";
    }
    return true;
  }

  template<class DiscreteFunctionSpaceType >
  inline bool DFAdapt< DiscreteFunctionSpaceType >::
  read_pgm( const char *fn )
  {
    FILE *in;
    int v;

    in = fopen( fn, "r" );
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

  template<class DiscreteFunctionSpaceType >
  inline void DFAdapt< DiscreteFunctionSpaceType >::
  addScaled( const DFAdapt<DiscreteFunctionSpaceType> &g,
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
  LocalFunctionAdapt < DiscreteFunctionSpaceType >::operator [] (int num) const
  {
    std::cout << "LFAdadpt::operator[]: " << num << " of " << numberOfDofs() << std::endl;
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

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType >
  template <class EntityType, class QuadratureType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType >::
  jacobian (EntityType &en, QuadratureType &quad, int quadPoint, JacobianRangeType & ret) const
  {
    enum { dim = EntityType::dimension };
    Mat<dim,dim,RangeFieldType> & inv = en.geometry().Jacobian_inverse(quad.point(quadPoint));

    if(numOfDifferentDofs_ > 1) // i.e. polynom order > 0
    {
      ret = 0.0;
      for(int i=0; i<numOfDifferentDofs_; i++)
      {
        baseFuncSet_->jacobian(i,quad,quadPoint,tmpGrad_);
        tmpGrad_(0) = inv.mult_t(tmpGrad_(0));

        tmpGrad_(0) *= (* (values_[i]));

        ret[0] += tmpGrad_(0);
      }
    }
    else
    {
      ret = 0.0;
    }
  }

  template<class DiscreteFunctionSpaceType > template <class EntityType>
  inline bool LocalFunctionAdapt < DiscreteFunctionSpaceType >::
  init (EntityType &en ) const
  {
    if((!uniform_) || (!baseFuncSet_))
    {
      // * this is a hack and needs to go away
      baseFuncSet_ = const_cast<BaseFunctionSetType*>(& ( fSpace_.getBaseFunctionSet(en) ));
      //baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();
      numOfDifferentDofs_ = baseFuncSet_->getNumberOfDiffBaseFuncs();

      if(numOfDof_ > this->values_.size())
        this->values_.resize( numOfDof_ );
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &(this->dofVec_[ fSpace_.mapToGlobal ( en , i) ]);
    return true;
  }

  //**********************************************************************
  //
  //  DofIteratorAdapt
  //
  //**********************************************************************
  template <class DofType, class DofArrayType>
  DofIteratorAdapt<DofType, DofArrayType>&
  DofIteratorAdapt<DofType, DofArrayType>::
  operator= (const DofIteratorAdapt<DofType,
                 DofArrayType>& other) {
    if (*this != other) {
      dofArray_ = other.dofArray_;
      count_ = other.count_;
    }
    return *this;
  }

  template <class DofType,class DofArrayType>
  inline DofType& DofIteratorAdapt<DofType,DofArrayType>::operator *()
  {
    assert((count_ >=0) && (count_ < dofArray_->size()));
    return (*dofArray_)[ count_ ];
  }

  template <class DofType,class DofArrayType>
  inline const DofType& DofIteratorAdapt<DofType,DofArrayType>::operator* () const
  {
    assert((count_ >=0) && (count_ < dofArray_->size()));
    return (*dofArray_) [ count_ ];
  }

  template <class DofType,class DofArrayType>
  inline DofIteratorAdapt<DofType,DofArrayType>& DofIteratorAdapt<DofType,DofArrayType>::operator ++()
  {
    ++count_;
    return (*this);
  }

  template <class DofType,class DofArrayType>
  inline DofType& DofIteratorAdapt<DofType,DofArrayType>::operator [](int i)
  {
    assert((i >=0) && (i < dofArray_->size()));
    return (*dofArray_)[i];
  }

  template <class DofType,class DofArrayType>
  inline const DofType& DofIteratorAdapt<DofType,DofArrayType>::operator [](int i) const
  {
    assert((i >=0) && (i < dofArray_->size()));
    return (*dofArray_)[i];
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
  inline void DofIteratorAdapt<DofType,DofArrayType>::reset()
  {
    count_ = 0;
  }


} // end namespace

#endif
