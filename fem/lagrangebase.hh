// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__LAGRANGEBASE_HH__
#define __DUNE__LAGRANGEBASE_HH__

#include <dune/grid/common/grid.hh>

#include "common/discretefunctionspace.hh"
#include "lagrangebase/lagrangebasefunctions.hh"
#include "lagrangebase/lagrangemapper.hh"
#include "dofmanager.hh"
#include "dgspace/dgmapper.hh"

namespace Dune {

  //****************************************************************
  //
  //  --LagrangeDiscreteFunctionSpace
  //
  /*! Provides access to base function set for different element
      type in one grid and size of functionspace
      and map from local to global dof number
   */
  template< class FunctionSpaceType, class GridType,int polOrd, class
      DofManagerType = DofManager<GridType> >
  class LagrangeDiscreteFunctionSpace
    : public DiscreteFunctionSpaceDefault <  FunctionSpaceType , GridType,
          LagrangeDiscreteFunctionSpace < FunctionSpaceType , GridType, polOrd, DofManagerType >,
          FastBaseFunctionSet < LagrangeDiscreteFunctionSpace
              < FunctionSpaceType , GridType, polOrd, DofManagerType > > >
  {
  public:
    /** \todo Please doc me! */
    typedef LagrangeDiscreteFunctionSpace
    < FunctionSpaceType , GridType , polOrd , DofManagerType > LagrangeDiscreteFunctionSpaceType;

    /** \todo Please doc me! */
    typedef DiscreteFunctionSpaceDefault <
        FunctionSpaceType , GridType, LagrangeDiscreteFunctionSpaceType,
        FastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType > >  DiscreteFunctionSpaceType;

    /** \todo Please doc me! */
    typedef FastBaseFunctionSet <LagrangeDiscreteFunctionSpaceType > BaseFunctionSetType;
    /** \todo Please doc me! */
    typedef BaseFunctionSetType FastBaseFunctionSetType;

    //! id is neighbor of the beast
    static const IdentifierType id = 665;

    // Lagrange 1 , to be revised in this matter
    /** \todo Please doc me! */
    enum { numOfDiffBase_ = 20 };
    /** \todo Please doc me! */
    enum { DimRange = FunctionSpaceType::DimRange };

  public:
    /** \todo Please doc me! */
    typedef LagrangeMapper<typename DofManagerType::IndexSetType,polOrd,DimRange> LagrangeMapperType;

    /** \todo Please doc me! */
    typedef typename DofManagerType::MemObjectType MemObjectType;

    // for gcc ( gcc sucks )
    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::Domain Domain;
    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::Range Range;
    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::RangeField DofType;
    typedef typename FunctionSpaceType::DomainField DomainField;

    //! dimension of value
    enum { dimVal = 1 };

    //! remember polynomial order
    enum { polynomialOrder =  polOrd };

    //! Constructor generating for each different element type of the grid a
    //! LagrangeBaseSet with polOrd
    LagrangeDiscreteFunctionSpace ( GridType & g, DofManagerType & dm , int level );

    //! return max number of baseset that holds this space
    int maxNumberBase () const;

    //! Desctructor
    virtual ~LagrangeDiscreteFunctionSpace ();

    //! return type of this fucntion space
    DFSpaceIdentifier type () const;

    //! provide the access to the base function set for a given entity
    template <class EntityType>
    const FastBaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const;

    //! default for polOrd 0
    template <class EntityType>
    bool evaluateLocal (int baseFunc, EntityType &en,Domain &local, Range & ret) const;

    //! default for polOrd 0
    template <class EntityType, class QuadratureType>
    bool evaluateLocal ( int baseFunc, EntityType &en, QuadratureType &quad,
                         int quadPoint, Range & ret) const;

    //! return true if we have continuous discrete functions
    bool continuous () const;

    //! get maximal global polynom order
    int polynomOrder () const;

    //! get dimension of value
    int dimensionOfValue () const;

    //! get local polynom order on entity
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const;

    //! number of unknows for this function space
    int size () const;

    //! return boundary type for given boundary id
    //! uses the parameter class BoundaryIdentifierType
    BoundaryType boundaryType ( int id ) const;

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const;

    //! sign in to dofmanager, return is the memory
    template <class DiscFuncType>
    MemObjectType & signIn (DiscFuncType & df);

    //! sign out to dofmanager, dofmanager frees the memory
    template <class DiscFuncType>
    bool signOut (DiscFuncType & df);

  protected:
    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    FastBaseFunctionSetType* setBaseFuncSetPointer ( EntityType &en );

    //! make base function set depending on GeometryType and polynomial order
    template <GeometryType ElType, int pO >
    FastBaseFunctionSetType* makeBaseSet ();

    // max number of basesets
    int maxNumBase_;

    //! the corresponding vector of base function sets
    //! length is different element types
    FieldVector<FastBaseFunctionSetType*, numOfDiffBase_ > baseFuncSet_;

  protected:
    //! DofManager manages the memory
    DofManagerType & dm_;

  private:
    //! the corresponding LagrangeMapper
    LagrangeMapperType *mapper_;

  }; // end class LagrangeDiscreteFunctionSpace


  //*******************************************************************
  //
  //! DGSpace using Lagrange basis functions, used for visualisation
  //
  //*******************************************************************
  template< class FunctionSpaceType, class GridType,int polOrd, class
      DofManagerType = DofManager<GridType> >
  class LagrangeDGSpace
    : public LagrangeDiscreteFunctionSpace<  FunctionSpaceType , GridType,
          polOrd , DofManagerType >
  {
  public:
    typedef LagrangeDiscreteFunctionSpace
    < FunctionSpaceType , GridType , polOrd , DofManagerType > ChefType;
    typedef ChefType LagrangeDiscreteFunctionSpaceType;

    typedef DiscreteFunctionSpaceInterface <
        FunctionSpaceType , GridType, LagrangeDiscreteFunctionSpaceType,
        FastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType > >  DiscreteFunctionSpaceType;

    typedef FastBaseFunctionSet <ChefType> BaseFunctionSetType;
    typedef BaseFunctionSetType FastBaseFunctionSetType;

    //! id is neighbor of the beast
    static const IdentifierType id = 668;

    // Lagrange 1 , to be revised in this matter
    enum { numOfDiffBase_ = 20 };
    enum { DimRange = FunctionSpaceType::DimRange };

  public:
    typedef DGMapper<typename DofManagerType::IndexSetType,polOrd,DimRange> DGMapperType;

    typedef typename DofManagerType::MemObjectType MemObjectType;

    // for gcc ( gcc sucks )
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::RangeField DofType;
    typedef typename FunctionSpaceType::DomainField DomainField;

    //! dimension of value
    enum { dimVal = 1 };

    //! remember polynomial order
    enum { polynomialOrder =  polOrd };

    //! Constructor generating for each different element type of the grid a
    //! LagrangeBaseSet with polOrd
    LagrangeDGSpace ( GridType & g, DofManagerType & dm , int level ) :
      ChefType (g,dm,level) , mapper_(0)
    {
      typedef typename GridType::template Traits<0> :: LevelIterator LevIt;
      LevIt it = g.template lbegin<0>(0);
      if(it != g.template lend<0>(0))
      {
        int basenum = this->getBaseFunctionSet(*it).getNumberOfBaseFunctions();
        mapper_ = new DGMapperType ( this->dm_.indexSet() , basenum , level );
      }
    };

    //! Destructor
    ~LagrangeDGSpace () {
      if (mapper_) delete mapper_;
    }

    /** \todo Please doc me! */
    template <class DiscFuncType>
    MemObjectType & signIn (DiscFuncType & df)
    {
      // only for gcc to pass type DofType
      assert(mapper_ != 0);
      DofType *fake=0;
      return this->dm_.addDofSet( fake, this->grid_ , *mapper_, df.name() );
      // do notin' at the moment
    }

    //! return max number of baseset that holds this space
    int maxNumberBase () const;

    //! return type of this fucntion space
    DFSpaceIdentifier type () const { return DGSpace_id; }

    //! return true if we have continuous discrete functions
    bool continuous () const { return false; }

    //! number of unknows for this function space
    int size () const { return mapper_->size() ; }

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_->mapToGlobal(en,localNum);
    }

  private:
    //! the corresponding LagrangeMapper
    DGMapperType *mapper_;

  }; // end class LagrangeDiscreteFunctionSpace


  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //*************************************************************************
  //************************************************************************
  //
  //  --Raviart Thomas
  //
  //
  //************************************************************************
  //
  //  --RTFastBaseFunctionSet
  //*********************************************************************
  //! default definition stays empty because implementation via
  //! specialization
  //! definition of LagrangeBaseFunction, implementation via specialization
  //template<class FunctionSpaceType, GeometryType ElType, int polOrd>
  //  class RaviartThomasBaseFunction;

  //! Raviart Thomas Elements for triangles
  template<class FunctionSpaceType, GeometryType ElType, int polOrd >
  class RaviartThomasBaseFunction
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;

    Domain point_;
    int baseNum_;

  public:
    RaviartThomasBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f) , baseNum_ (baseNum)
        , point_ (0.0)
    {
      /*
          (0,1)
          1|\    coordinates and local node numbers
       | \
       |  \
          0|   \2
       |    \
       |     \
          2|______\0
         (0,0) 1  (1,0)
       */

      std::cout << "Making Raviart Thomas Elements !\n";
      // we implement linear functions but we use them at only at the faces
      switch (baseNum)
      {
      // point has to be the vertex coordinates of the reference element
      case 0 : { // point0 = ( 1 , 0 )
        point_(0) = 1.0;
        break;
      }
      case 1 : { // point1 = ( 0 , 1 )
        point_(1) = 1.0;
        break;
      }
      case 2 : { // point2 = ( 0 , 0 )
        point_ = 0.0;
        break;
      }
      default : {
        std::cerr << "Wrong baseNum in RaviartThomasBaseFunction::Constructor! \n";
        abort();
      }
      }
    }

    /** \todo Please doc me! */
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    { // q(x) = (x - point_ ) * 1/(2|T|) mit |T|=0.5
      phi = (x - point_);
    }

    /** \todo Please doc me! */
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int comp = diffVariable[0];
      phi = 0.0;
      phi(comp) = 1.0;
    }

    /** \todo Please doc me! */
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }
  };

  //! elements which are 1 at the faces
  template<class FunctionSpaceType, GeometryType ElType, int polOrd>
  class EdgeBaseFunction
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    RangeField factor[3];
    int baseNum_;

  public:
    EdgeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f) , baseNum_ (baseNum)
    {
      baseNum_ = baseNum;
      if(baseNum == 2)
      { // x + y
        factor[0] = 1.0;
        factor[1] = 1.0;
        factor[2] = 0.0;
      }
      else
      {
        factor[2] = 1.0;
        for(int i=0; i<2; i++) // -x , -y
          if(baseNum == i)
            factor[i] = -1.0;
          else
            factor[i] = 0.0;
      }
    }


    /** \todo Please doc me! */
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[2];
      for(int i=0; i<2; i++)
        phi += factor[i] * x(i);
    }

    /** \todo Please doc me! */
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // x or y ==> 1 or 2
      int num = diffVariable[0];
      phi = factor[num];
    }

    /** \todo Please doc me! */
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }
  };

  //*******************************************************
  //! default Raviart Thomas Definition
  //*******************************************************
  template <GeometryType ElType,int polOrd , int dimrange >
  struct RaviartThomasDefinition
  {
    enum { numOfBaseFct = 0 };
  };

  //! Raviart Thomas Definition for triangles
  template <int polOrd , int dimrange >
  struct RaviartThomasDefinition< triangle , polOrd, dimrange >
  {
    enum { numOfBaseFct = (3) };
  };

  //*******************************************************
  //! default Raviart Thomas Definition
  //*******************************************************
  template <GeometryType ElType,int polOrd , int dimrange >
  struct EdgeDefinition
  {
    enum { numOfBaseFct = 0 };
  };

  //! Raviart Thomas Definition for triangles
  template <int polOrd , int dimrange >
  struct EdgeDefinition< triangle , polOrd, dimrange >
  {
    enum { numOfBaseFct = (3) };
  };

  /** \todo Please doc me! */
  template<class FunctionSpaceType, GeometryType ElType, int polOrd >
  class RaviartThomasFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    enum { dimrange = FunctionSpaceType::DimRange };
    //! know the number of base functions
    enum { numOfBaseFct = RaviartThomasDefinition
                          < ElType, polOrd, dimrange >::numOfBaseFct };

    //! type of RaviartThomasBaseFunctions
    typedef RaviartThomasBaseFunction < FunctionSpaceType , ElType , polOrd>
    RaviartThomasBaseFunctionType;
  public:
    enum { dimVal = dimrange };

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    RaviartThomasFastBaseFunctionSet( FunctionSpaceType &fuSpace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuSpace, numOfBaseFct )
    {
      int numOfDifferentFuncs = (int) numOfBaseFct;
      for(int i=0; i<numOfDifferentFuncs; i++)
      {
        baseFuncList_(i) =
          new RaviartThomasBaseFunctionType ( fuSpace, i ) ;
        setBaseFunctionPointer ( i, baseFuncList_ (i) );
      }
      this->setNumOfDiffFct ( numOfDifferentFuncs );
    };

    //! Destructor deleting the base functions
    ~RaviartThomasFastBaseFunctionSet( )
    {
      for(int i=0; i<numOfBaseFct; i++)
        delete baseFuncList_(i);
    };

    //! return number of base function for this base function set
    int getNumberOfBaseFunctions() const { return numOfBaseFct; };

    int getNumberOfDiffrentBaseFunctions () const
    {
      return (int) (numOfBaseFct);
    }
  private:
    //! Vector with all base functions corresponding to the base function set
    FieldVector <RaviartThomasBaseFunctionType*, numOfBaseFct> baseFuncList_;
  };

  //*******************************************************************
  //
  //!  EdgeBaseFunctionSet
  //
  //*******************************************************************
  template<class FunctionSpaceType, GeometryType ElType, int polOrd >
  class EdgeFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    enum { dimrange = FunctionSpaceType::DimRange };
    //! know the number of base functions
    enum { numOfBaseFct = RaviartThomasDefinition
                          < ElType, polOrd, dimrange >::numOfBaseFct };

    //! type of RaviartThomasBaseFunctions
    typedef EdgeBaseFunction < FunctionSpaceType , ElType , polOrd>
    EdgeBaseFunctionType;
  public:
    enum { dimVal = dimrange };

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    EdgeFastBaseFunctionSet( FunctionSpaceType &fuSpace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuSpace, numOfBaseFct )
    {
      int numOfDifferentFuncs = (int) numOfBaseFct;
      for(int i=0; i<numOfDifferentFuncs; i++)
      {
        baseFuncList_(i) =
          new EdgeBaseFunctionType ( fuSpace, i ) ;
        setBaseFunctionPointer ( i, baseFuncList_ (i) );
      }
      this->setNumOfDiffFct ( numOfDifferentFuncs );
    };

    //! Destructor deleting the base functions
    ~EdgeFastBaseFunctionSet( )
    {
      for(int i=0; i<numOfBaseFct; i++)
        delete baseFuncList_(i);
    };

    //! return number of base function for this base function set
    int getNumberOfBaseFunctions() const { return numOfBaseFct; };

    //! \todo Please doc me!
    int getNumberOfDiffrentBaseFunctions () const
    {
      return (int) (numOfBaseFct);
    }
  private:
    //! Vector with all base functions corresponding to the base function set
    FieldVector<EdgeBaseFunctionType*, numOfBaseFct> baseFuncList_;
  };

  //! \todo Please doc me!
  template <class GridType, int polOrd>
  class RTMapper
    : public DofMapperDefault < RTMapper <GridType,polOrd> >
  {
    int numberOfDofs_;
    int level_;
  public:

    //! \todo Please doc me!
    RTMapper ( int numDof , int level ) : numberOfDofs_ (numDof), level_(level) {};

    //! \todo Please doc me!
    int size (const GridType &grid ) const
    {
      // return number of entities  * number of local faces
      return (numberOfDofs_ * grid.size( level_ , 0 ));
    }

    //! map Entity an local Dof number to global Dof number
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return ((en.index() * numberOfDofs_) + localNum);
    }

  }; // end class RTMapper

  //! \todo Please doc me!
  template< class FunctionSpaceType, class GridType, int polOrd  >
  class RaviartThomasSpace
    : public DiscreteFunctionSpaceInterface <  FunctionSpaceType , GridType,
          RaviartThomasSpace < FunctionSpaceType , GridType, polOrd >,
          FastBaseFunctionSet < RaviartThomasSpace
              < FunctionSpaceType , GridType, polOrd   > > >
  {
    enum {id = 77809 };

    typedef RaviartThomasSpace < FunctionSpaceType ,
        GridType , polOrd > RaviartThomasSpaceType;
    typedef DiscreteFunctionSpaceInterface <
        FunctionSpaceType , GridType,
        RaviartThomasSpace < FunctionSpaceType , GridType, polOrd >,
        FastBaseFunctionSet < RaviartThomasSpace
            < FunctionSpaceType , GridType, polOrd   > > >  DiscreteFunctionSpaceType;

    // i.e. number of diffrent element types
    enum { numOfDiffBase_ = 20 };
    enum { dimVal = FunctionSpaceType::DimRange };

  public:
    typedef FastBaseFunctionSet < RaviartThomasSpace
        < FunctionSpaceType , GridType, polOrd   > > BaseFunctionSetType;
    typedef BaseFunctionSetType FastBaseFunctionSetType;

    // for gcc
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;

    RaviartThomasSpace ( GridType & g ) :
      DiscreteFunctionSpaceType (g,id)
    {
      mapper_ = 0;

      for(int i=0; i<numOfDiffBase_; i++)
        baseFuncSet_(i) = 0;

      // search the macro grid for diffrent element types
      typedef typename GridType::template Traits<0>::LevelIterator LevelIterator;
      LevelIterator endit = g.template lend<0>(0);
      for(LevelIterator it = g.template lbegin<0>(0); it != endit; ++it)
      {
        GeometryType type = (*it).geometry().type(); // Hack
        if(baseFuncSet_( type ) == 0 )
          baseFuncSet_ ( type ) = setBaseFuncSetPointer(*it);
      }
    }

    ~RaviartThomasSpace ()
    {
      if(mapper_) delete mapper_;
    }

    DFSpaceIdentifier type () const
    {
      return RaviartThomasSpace_id;
    }

    //! provide the access to the base function set
    template <class EntityType>
    const FastBaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const
    {
      GeometryType type =  en.geometry().type();
      return (*baseFuncSet_( type ));
    }

    //! return true if we have continuous discrete functions
    bool continuous ( ) const
    {
      return false;
    }

    //! get maximal global polynom order
    int polynomOrder ( ) const
    {
      return polOrd;
    }

    //! get local polynom order on entity
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const
    {
      return polOrd;
    }

    //! get dimension of value
    int dimensionOfValue () const
    {
      return dimVal;
    }

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size () const
    {
      return (*mapper_).size ( this->grid_ );
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return (*mapper_).mapToGlobal ( en , localNum );
    };

    //! default for polOrd 0
    template <class EntityType>
    bool evaluateLocal ( int baseFunc, EntityType &en,
                         Domain &local, Range & ret) const
    {
      enum { dim = EntityType::dimension };
      const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);

      Mat<dim,dim> & inv =
        en.geometry().Jacobian_inverse(local);
      baseSet.eval( baseFunc , local , ret);
      ret = inv * ret;
      return true;
    }

    //! default for polOrd 0
    template <class EntityType, class QuadratureType>
    bool evaluateLocal ( int baseFunc, EntityType &en, QuadratureType &quad,
                         int quadPoint, Range & ret) const
    {
      enum { dim = EntityType::dimension };
      const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);

      Mat<dim,dim> & inv =
        en.geometry().Jacobian_inverse(quad.point(quadPoint));
      baseSet.eval( baseFunc , quad, quadPoint , ret);
      ret = inv * ret;
      return true;
    }

  private:
    RTMapper<GridType,polOrd> *mapper_;

    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    FastBaseFunctionSetType* setBaseFuncSetPointer ( EntityType &en )
    {
      switch (en.geometry().type())
      {
      case line         : return makeBaseSet<line,polOrd> ();
      case triangle     : return makeBaseSet<triangle,polOrd> ();
      case quadrilateral : return makeBaseSet<quadrilateral,polOrd> ();
      case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> ();
      case hexahedron   : return makeBaseSet<hexahedron,polOrd> ();
      default : {
        std::cerr << en.geometry().type() << " This element type is not provided yet! \n";
        abort();
        return 0;
      }
      }
    }

    //! the corresponding vector of base function sets
    //! lenght is diffrent element types
    FieldVector< FastBaseFunctionSetType*, numOfDiffBase_ > baseFuncSet_;

    //! make base function set depending on GeometryType and polynomial order
    template <GeometryType ElType, int pO >
    FastBaseFunctionSetType* makeBaseSet ()
    {

      typedef RaviartThomasFastBaseFunctionSet <
          RaviartThomasSpaceType,ElType, pO > BaseFuncSetType;

      BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

      mapper_ = new RTMapper < GridType, polOrd >
                  (baseFuncSet->getNumberOfBaseFunctions());

      return baseFuncSet;
    }
  };

  //********************************************************************
  //
  //!  EdgeSpace
  //
  //********************************************************************
  template< class FunctionSpaceType, class GridType, int polOrd  >
  class EdgeSpace
    : public DiscreteFunctionSpaceInterface <  FunctionSpaceType , GridType,
          EdgeSpace < FunctionSpaceType , GridType, polOrd >,
          FastBaseFunctionSet < EdgeSpace
              < FunctionSpaceType , GridType, polOrd   > > >
  {
    typedef FastBaseFunctionSet < EdgeSpace
        < FunctionSpaceType , GridType, polOrd   > > BaseFunctionSetType;

    typedef BaseFunctionSetType FastBaseFunctionSetType;
    enum {id = 77809 };

    typedef EdgeSpace < FunctionSpaceType ,
        GridType , polOrd > EdgeSpaceType;
    typedef DiscreteFunctionSpaceInterface <
        FunctionSpaceType , GridType,
        EdgeSpace < FunctionSpaceType , GridType, polOrd >,
        FastBaseFunctionSet < EdgeSpace
            < FunctionSpaceType , GridType, polOrd   > > >  DiscreteFunctionSpaceType;

    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;

    // i.e. number of diffrent element types
    enum { numOfDiffBase_ = 20 };
    enum { dimVal = FunctionSpaceType::DimRange };

    Array < int > edgeMap_;

    int level_;
  public:
    //! \todo Please doc me!
    EdgeSpace ( GridType & g , int level ) :
      DiscreteFunctionSpaceType (g,id) , level_ (level)
    {
      for(int i=0; i<numOfDiffBase_; i++)
        baseFuncSet_(i) = 0;

      typedef typename GridType::template Traits<0>::LevelIterator LevelIterator;

      int edgeSize = 3 * g.size ( level , 0);
      edgeMap_.resize( edgeSize );
      for(int i=0; i<edgeSize; i++) edgeMap_[i] = -1;

      LevelIterator it = g.lbegin<0>(level);
      LevelIterator endit = g.lend<0>(level);

      // walktrough grid
      int edge = 0;
      for( ; it != endit; ++it)
      {
        GeometryType type = (*it).geometry().type(); // Hack
        if(baseFuncSet_( type ) == 0 )
          baseFuncSet_ ( type ) = setBaseFuncSetPointer(*it);

        typedef typename
        GridType::template Traits<0>::Entity::Traits::IntersectionIterator EdgeIt;

        int index = it->index();
        EdgeIt nit    = it->ibegin();
        EdgeIt endnit = it->iend();
        for( ; nit != endnit; ++nit)
        {
          int num = nit.number_in_self();
          int col = mapElNum(*it,num);

          if(nit.neighbor())
          {
            if(edgeMap_[col] == -1)
            {
              edgeMap_[col] = edge;
              // the curr entity

              // the neighbor
              num = nit.number_in_neighbor();
              col = mapElNum(*nit,num);
              edgeMap_[col] = edge;
              edge++;
            }
          }
          else
          {
            edgeMap_[col] = -edge;
            edge++;
          }
        }
      }
      //for(int i=0; i<edgeSize; i++)
      //  printf("edge %d \n",edgeMap_[i]);
    }


    //! provide the access to the base function set
    template <class EntityType>
    const FastBaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const
    {
      GeometryType type =  en.geometry().type();
      return (*baseFuncSet_( type ));
    }

    //! return true if we have continuous discrete functions
    bool continuous ( ) const
    {
      return false;
    }

    //! get maximal global polynom order
    int polynomOrder ( ) const
    {
      return polOrd;
    }

    //! get local polynom order on entity
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const
    {
      return polOrd;
    }

    //! get dimension of value
    int dimensionOfValue () const
    {
      return dimVal;
    }

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size () const
    {
      return this->grid_.size( level_ , 1);
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return edgeMap_[mapElNum(en,localNum )];
    };

    //! default for polOrd 0
    template <class EntityType>
    bool evaluateLocal ( int baseFunc, EntityType &en,
                         Domain &local, Range & ret) const
    {
      enum { dim = EntityType::dimension };
      const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);

      Mat<dim,dim> & inv =
        en.geometry().Jacobian_inverse(local);
      baseSet.eval( baseFunc , local , ret);
      ret = inv * ret;
      return true;
    }

    //! default for polOrd 0
    template <class EntityType, class QuadratureType>
    bool evaluateLocal ( int baseFunc, EntityType &en, QuadratureType &quad,
                         int quadPoint, Range & ret) const
    {
      enum { dim = EntityType::dimension };
      const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);

      Mat<dim,dim> & inv =
        en.geometry().Jacobian_inverse(quad.point(quadPoint));
      baseSet.eval( baseFunc , quad, quadPoint , ret);
      ret = inv * ret;
      return true;
    }

  private:
    template <class EntityType>
    int mapElNum (EntityType &en , int loc ) const
    {
      return ((3 * en.index()) + loc);
    }

    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    FastBaseFunctionSetType* setBaseFuncSetPointer ( EntityType &en )
    {
      switch (en.geometry().type())
      {
      case line         : return makeBaseSet<line,polOrd> ();
      case triangle     : return makeBaseSet<triangle,polOrd> ();
      case quadrilateral : return makeBaseSet<quadrilateral,polOrd> ();
      case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> ();
      case hexahedron   : return makeBaseSet<hexahedron,polOrd> ();
      default : {
        std::cerr << en.geometry().type() << " This element type is not provided yet! \n";
        abort();
        return 0;
      }
      }
    }

    //! the corresponding vector of base function sets
    //! lenght is diffrent element types
    FieldVector<FastBaseFunctionSetType*, numOfDiffBase_ > baseFuncSet_;

    //! make base function set depending on GeometryType and polynomial order
    template <GeometryType ElType, int pO >
    FastBaseFunctionSetType* makeBaseSet ()
    {

      typedef EdgeFastBaseFunctionSet <
          EdgeSpaceType,ElType, pO > BaseFuncSetType;

      BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );
      return baseFuncSet;
    }
  };

} // end namespace Dune

// contains the implementation of LagrangeSpace
#include "lagrangebase/lagrangespace.cc"

#endif
