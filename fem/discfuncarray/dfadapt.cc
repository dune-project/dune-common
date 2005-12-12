// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DFADAPT_CC
#define DUNE_DFADAPT_CC

#include <algorithm>

namespace Dune {

  template<class DiscreteFunctionSpaceType>
  inline DFAdapt< DiscreteFunctionSpaceType>::
  DFAdapt(DiscreteFunctionSpaceType& f) :
    DiscreteFunctionDefaultType ( f ),
    name_ ("no name"),
    dm_(DofManagerFactoryType::getDofManager(f.grid())),
    memPair_(dm_.addDofSet(&dofVec_, f.mapper(), name_)),
    dofVec_ ( *memPair_.second ),
    localFunc_(*this)
    //localFunc_ ( f , dofVec_ )
  {}

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType>
  inline DFAdapt< DiscreteFunctionSpaceType>::
  DFAdapt(std::string name, DiscreteFunctionSpaceType & f) :
    DiscreteFunctionDefaultType ( f ),
    name_ ((name.length() > 0) ? name : "no name"),
    dm_(DofManagerFactoryType::getDofManager(f.grid())),
    memPair_(dm_.addDofSet(&dofVec_, f.mapper(), name_)),
    dofVec_ ( *memPair_.second ),
    localFunc_(*this)
  {}

  // Constructor making discrete function
  template<class DiscreteFunctionSpaceType>
  template <class VectorPointerType>
  inline DFAdapt< DiscreteFunctionSpaceType>::
  DFAdapt(std::string name, DiscreteFunctionSpaceType & f, VectorPointerType * vector ) :
    DiscreteFunctionDefaultType ( f ),
    name_ ((name.length() > 0) ? name : "no name"),
    dm_(DofManagerFactoryType::getDofManager(f.grid())),
    memPair_(dm_.addDummyDofSet(&dofVec_, f.mapper(), name_, vector )),
    dofVec_ ( *memPair_.second ),
    localFunc_(*this)
  {}

  template<class DiscreteFunctionSpaceType>
  inline DFAdapt< DiscreteFunctionSpaceType>::
  DFAdapt(const DFAdapt <DiscreteFunctionSpaceType> & df ) :
    DiscreteFunctionDefaultType ( df.functionSpace_ ),
    name_ (std::string("copy of ") + df.name()),
    dm_(df.dm_),
    memPair_(dm_.addDofSet(&dofVec_, df.functionSpace_.mapper(), name_)),
    dofVec_ ( *memPair_.second ),
    localFunc_(*this)
  {
    // copy values of array
    dofVec_ = df.dofVec_;
  }


  // Desctructor
  template<class DiscreteFunctionSpaceType>
  inline DFAdapt< DiscreteFunctionSpaceType>::
  ~DFAdapt()
  {
    dm_.removeDofSet(*memPair_.first);
  }


  template<class DiscreteFunctionSpaceType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::set ( RangeFieldType x )
  {
    int size = dofVec_.size();
    DofArrayType &vec = dofVec_;
    for(int i=0; i<size; i++)
      vec[i] = x;
  }

  template<class DiscreteFunctionSpaceType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::clear ()
  {
    set ( 0.0 );
  }

  template<class DiscreteFunctionSpaceType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  print(std::ostream &s )
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
  template<class DiscreteFunctionSpaceType>
  template <class EntityType>
  inline typename DFAdapt< DiscreteFunctionSpaceType>:: LocalFunctionType
  DFAdapt< DiscreteFunctionSpaceType>::localFunction(const EntityType& en) const
  {
    return LocalFunctionType (en,*this);
  }

  template<class DiscreteFunctionSpaceType>
  template <class EntityType>
  inline void
  DFAdapt< DiscreteFunctionSpaceType>::
  localFunction ( const EntityType &en , LocalFunctionType &lf )
  {
    lf.init ( en );
  }

  template<class DiscreteFunctionSpaceType>
  inline LocalFunctionAdapt<DiscreteFunctionSpaceType> *
  DFAdapt< DiscreteFunctionSpaceType>::
  newLocalFunctionObject ( ) const
  {
    return new LocalFunctionAdapt<DiscreteFunctionSpaceType> ( this->functionSpace_ , dofVec_ );
  }

  template<class DiscreteFunctionSpaceType>
  inline typename DFAdapt< DiscreteFunctionSpaceType>:: LocalFunctionType
  DFAdapt< DiscreteFunctionSpaceType>::
  newLocalFunction ()
  {
    return LocalFunctionType (*this);
  }

  template<class DiscreteFunctionSpaceType>
  inline typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType>::dbegin ( )
  {
    return dofVec_.begin();
  }

  template<class DiscreteFunctionSpaceType>
  inline typename DFAdapt<DiscreteFunctionSpaceType>::DofIteratorType
  DFAdapt< DiscreteFunctionSpaceType>::dend ()
  {
    return dofVec_.end();
  }

  template<class DiscreteFunctionSpaceType>
  inline typename DFAdapt<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DFAdapt< DiscreteFunctionSpaceType>::dbegin ( ) const
  {
    return dofVec_.begin();
  }

  template<class DiscreteFunctionSpaceType>
  inline typename DFAdapt<DiscreteFunctionSpaceType>::ConstDofIteratorType
  DFAdapt< DiscreteFunctionSpaceType>::dend () const
  {
    return dofVec_.end();
  }
  //**************************************************************************
  //  Read and Write Methods
  //**************************************************************************
  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
  write_xdr(std::string fn)
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

  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
  read_xdr(std::string fn)
  {
    FILE   *file;
    XDR xdrs;
    std::cout << "Reading <" << fn << "> \n";
    file = fopen(fn.c_str() , "rb");
    if(!file)
    {
      printf( "\aERROR in DFAdapt::read_xdr(..): couldnot open <%s>!\n", fn.c_str());
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

  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
  write_ascii(std::string fn)
  {
    std::fstream outfile( fn.c_str() , std::ios::out );
    if (!outfile)
    {
      printf( "\aERROR in DFAdapt::write_ascii(..): couldnot open <%s>!\n", fn.c_str());
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


  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
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

  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
  write_pgm(std::string fn)
  {
    std::ofstream out( fn.c_str() );

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

  template<class DiscreteFunctionSpaceType>
  inline bool DFAdapt< DiscreteFunctionSpaceType>::
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

  template<class DiscreteFunctionSpaceType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  addScaled( const DFAdapt<DiscreteFunctionSpaceType> &g,
             const RangeFieldType &scalar )
  {
    int length = dofVec_.size();

    DofArrayType &v = dofVec_;
    const DofArrayType &gvec = g.dofVec_;

    for(int i=0; i<length; i++)
      v[i] += scalar*gvec[i];
  }

  template<class DiscreteFunctionSpaceType>
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  addScaledLocal( EntityType &en ,
                  const DFAdapt<DiscreteFunctionSpaceType> &g, const RangeFieldType &scalar )
  {
    localFunc_ = localFunction( en );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunc_ = G.localFunction(en);

    int length = localFunc_.numDofs();
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

  template<class DiscreteFunctionSpaceType>
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  addLocal( EntityType &en ,
            const DFAdapt<DiscreteFunctionSpaceType> &g)
  {
    localFunc_ = localFunction( en );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunc_ = G.localFunction(en);

    int length = localFunc_.numDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] += G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType>
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  subtractLocal( EntityType &en ,
                 const DFAdapt<DiscreteFunctionSpaceType> &g)
  {
    localFunc_ = localFunction( en );

    DFAdapt<DiscreteFunctionSpaceType> &G =
      const_cast<DFAdapt<DiscreteFunctionSpaceType> &> (g);
    G.localFunc_ = G.localFunction(en);

    int length = localFunc_.numDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] -= G.localFunc_[i];
  }

  template<class DiscreteFunctionSpaceType>
  template<class EntityType>
  inline void DFAdapt< DiscreteFunctionSpaceType>::
  setLocal( EntityType &en , const RangeFieldType & scalar )
  {
    localFunc_ = localFunction( en );
    int length = localFunc_.numDofs();
    for(int i=0; i<length; i++)
      localFunc_[i] = scalar;
  }

  //**********************************************************************
  //  --LocalFunctionAdapt
  //**********************************************************************
  template<class DiscreteFunctionSpaceType>
  inline LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  LocalFunctionAdapt( const DiscreteFunctionSpaceType &f ,
                      DofArrayType & dofVec ) :
    tmp_(0.0),
    xtmp_(0.0),
    tmpGrad_(0.0),
    numOfDof_(-1),
    fSpace_ ( f ),
    values_ (),
    dofVec_ ( dofVec ),
    uniform_(true),
    init_(false)
  {}


  template<class DiscreteFunctionSpaceType>
  inline LocalFunctionAdapt < DiscreteFunctionSpaceType>::~LocalFunctionAdapt()
  {}

  template<class DiscreteFunctionSpaceType>
  inline typename LocalFunctionAdapt < DiscreteFunctionSpaceType>::RangeFieldType &
  LocalFunctionAdapt < DiscreteFunctionSpaceType>::operator [] (int num)
  {
    // check that storage (dofVec_) and mapper are in sync:
    assert(dofVec_.size() >= fSpace_.size());
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType>
  inline const typename LocalFunctionAdapt < DiscreteFunctionSpaceType>::RangeFieldType &
  LocalFunctionAdapt < DiscreteFunctionSpaceType>::operator [] (int num) const
  {
    // check that storage (dofVec_) and mapper are in sync:
    assert(dofVec_.size() >= fSpace_.size());
    return (* (values_[num]));
  }

  template<class DiscreteFunctionSpaceType>
  inline int LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  numberOfDofs () const
  {
    return numOfDof_;
  }

  template<class DiscreteFunctionSpaceType>
  inline int LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  numDofs () const
  {
    return numOfDof_;
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType> template <class EntityType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  evaluate (EntityType &en, const DomainType & x, RangeType & ret) const {
    ret = 0.0;
    xtmp_ = en.geometry().local(x);
    evaluateLocal(en, xtmp_, ret);
  }

  template<class DiscreteFunctionSpaceType> template <class EntityType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  evaluateLocal (EntityType &en, const DomainType & x, RangeType & ret) const
  {
    enum { dimRange = DiscreteFunctionSpaceType::DimRange };
    assert(init_);
    assert(en.geometry().checkInside(x));
    ret *= 0.0;
    const BaseFunctionSetType& bSet = fSpace_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.numBaseFunctions(); ++i)
    {
      bSet.eval(i, x, tmp_);
      for (int l = 0; l < dimRange; ++l) {
        ret[l] += (*values_[i]) * tmp_[l];
      }
    }
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType>
  template <class EntityType, class QuadratureType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  evaluate (EntityType &en, QuadratureType &quad, int quadPoint, RangeType & ret) const
  {
    evaluateLocal(en, quad.point(quadPoint), ret);
  }

  // hier noch evaluate mit Quadrature Regel einbauen
  template<class DiscreteFunctionSpaceType>
  template <class EntityType, class QuadratureType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  jacobian (EntityType &en, QuadratureType &quad, int quadPoint, JacobianRangeType & ret) const
  {
    jacobianLocal(en, quad.point(quadPoint), ret);
  }

  template<class DiscreteFunctionSpaceType>
  template <class EntityType>
  inline void LocalFunctionAdapt<DiscreteFunctionSpaceType>::
  jacobianLocal(EntityType& en, const DomainType& x,
                JacobianRangeType& ret) const
  {
    assert(init_);
    enum { dim = EntityType::dimension };
    enum { dimRange = DiscreteFunctionSpaceType::DimRange };

    ret *= 0.0;
    const BaseFunctionSetType& bSet = fSpace_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.numBaseFunctions(); ++i) {
      tmpGrad_ *= 0.0;
      bSet.jacobian(i, x, tmpGrad_);

      for (int l = 0; l < dimRange; ++l) {
        tmpGrad_[l] *= *values_[i];
        // * umtv or umv?
        en.geometry().jacobianInverseTransposed(x).umv(tmpGrad_[l], ret[l]);
      }
    }
  }

  template<class DiscreteFunctionSpaceType>
  template <class EntityType>
  inline void LocalFunctionAdapt<DiscreteFunctionSpaceType>::
  jacobian(EntityType& en, const DomainType& x, JacobianRangeType& ret) const {
    ret *= 0.0;
    xtmp_ = en.geometry().local(x);
    jacobianLocal(en, xtmp_, ret);
  }

  template<class DiscreteFunctionSpaceType>
  template <class EntityType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  init (const EntityType &en ) const
  {
    if(!uniform_ || !init_)
    {
      numOfDof_ =
        fSpace_.getBaseFunctionSet(en).numBaseFunctions();

      if(numOfDof_ > this->values_.size())
        this->values_.resize( numOfDof_ );

      init_ = true;
    }

    for(int i=0; i<numOfDof_; i++)
      values_ [i] = &(this->dofVec_[ fSpace_.mapToGlobal ( en , i) ]);

    return;
  }

  template<class DiscreteFunctionSpaceType>
  inline void LocalFunctionAdapt < DiscreteFunctionSpaceType>::
  assign(int numDof, const RangeType& dofs)
  {
    assert(false); // untested and most probably wrong
    for (size_t i = 0; i < dimrange; ++i) {
      *(values_[numDof + dimrange*i]) = dofs[i];
    }
  }

} // end namespace

#endif
