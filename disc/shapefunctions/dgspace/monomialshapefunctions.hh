// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MONOMIALSHAPEFUNCTIONS_HH
#define DUNE_MONOMIALSHAPEFUNCTIONS_HH

#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/common/array.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/stdstreams.hh>
#include <dune/common/misc.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/referenceelements.hh>

#include <dune/disc/shapefunctions/shapefunctions.hh>

/**
   @file
   @brief  define monomial type shape functions
   @author Christian Engwer
 */

namespace Dune
{

  /**
     @addtogroup DISC
     @{
   */

  enum {
    /**
       Default value for maximum order for the
       MonomialShapeFunctionSetContainer
     */
    MonomialShapeFunctionDefaultMaxOrder = 5
  };

  template<typename C,typename T, int dim>
  class MonomialShapeFunction;
  template<typename C,typename T, int dim>
  class MonomialShapeFunctionSet;
  template<typename C,typename T, int dim,
      int maxOrder=MonomialShapeFunctionDefaultMaxOrder>
  class MonomialShapeFunctionSetContainer;

  /**
     \class MonomialShapeFunction
     \brief multivariate monomials of the form x^a.y^b
     monomial order/total degree == p ; p <= (a+b)
   */
  template<typename C,typename T, int d>
  class MonomialShapeFunction : public ShapeFunction<C,T,d,1>
  {
  public:
    enum { dim=d /*!< Dimension of the ShapeFunction */ };
    typedef C CoordType;
    typedef T ResultType;

    /**
       Create the n-th MonomialShapeFunction of the ShapeFunctionSet of
       the given order.

       \param n this is the n-th shape functions in the set
       \param exp exponent a,b of x^a * y^b
     */
    MonomialShapeFunction(int n, const FieldVector<int, dim> & exp) :
      n_(n), exp_(exp) {}

    /** \copydoc ShapeFunction::evaluateFunction */
    virtual ResultType
    evaluateFunction (int, const FieldVector<CoordType,dim>& x) const
    {
      ResultType phi = 1.0;
      // phi= x^(alpha-beta) * y^beta
      // The compiler should be able to eliminate this loop
      for (int c=0; c<dim; c++)
      {
        phi *= power(x[c],exp_[c]);
      }
      return phi;
    }

    /** \copydoc ShapeFunction::evaluateDerivative */
    virtual ResultType
    evaluateDerivative(int, int dir, const FieldVector<CoordType,dim>& x) const
    {
      ResultType dphi = 1.0;
      // The compiler should be able to eliminate this loop
      for (int c=0; c<dim; c++)
      {
        dphi *= power(x[c],exp_[c]);
      }
      dphi *= exp_[dir] / x[dir];
      return dphi;
    }

    /** \copydoc ShapeFunction::localindex */
    virtual int localindex (int comp) const { return n_; };

    /** \copydoc ShapeFunction::codim */
    virtual int codim () const { return 0; };

    /** \copydoc ShapeFunction::entity */
    virtual int entity () const { return 0; };

    /** \copydoc ShapeFunction::entityindex */
    virtual int entityindex () const { return 0; };

    /** pretty print this shape function */
    void print (std::ostream& s) const
    {
      static const char names[] =
      { 'x', 'y', 'z', 'u', 'v', 'w', 'q', 'r', 's' };
      assert( dim < 9 ); /* avoid buffer overrun un names */;
      bool all_zero = true;
      for (int c=0; c<dim; c++)
      {
        if (exp_[c] > 0) {
          if (exp_[c] == 1) {
            s << names[c] << " ";
          }
          else {
            s << names[c] << "^" << exp_[c] << " ";
          }
          all_zero = false;
        }
      }
      if (all_zero) {
        s << 1;
      }
    };
  private:
    /** this is the n-th shape functions in the set */
    int n_;
    /** exponents a,b of x^a * y^b */
    FieldVector<int, dim> exp_;
    /** calculate the power x^p */
    ResultType power(CoordType x, int p) const {
      if (p <= 0)
        return 1.0;
      return x*power(x, p-1);
    }
  };

  /**
     \class MonomialShapeFunctionSetCreator
     \internal
     \brief Template Metaprogramm creating the entries for
            MonomialShapeFunctionSet
   */
  template <typename C,typename T, int dim, int c>
  struct MonomialShapeFunctionSetCreator
  {
    typedef MonomialShapeFunction<C,T,dim> ShapeFunction;
    typedef FieldVector<int, dim> expV;
    typedef Array< ShapeFunction* > sfV;
    static void create (int bound, int& cnt, expV& exp, sfV& SFnts)
    {
      for (int a=0; a<=bound; a++)
      {
        exp[c-1] = bound-a;
        MonomialShapeFunctionSetCreator<C,T,dim,c+1>::create(a,cnt,exp,SFnts);
      }
    }
  };

  /**
     \copydoc MonomialShapeFunctionSetCreator
     \internal
     Specializeses the beginning of the recursion
   */
  template <typename C,typename T, int dim>
  struct MonomialShapeFunctionSetCreator<C,T,dim,0>
  {
    typedef MonomialShapeFunction<C,T,dim> ShapeFunction;
    typedef FieldVector<int, dim> expV;
    typedef Array< ShapeFunction* > sfV;
    static int create (int order, sfV& SFnts)
    {
      SFnts = 0;
      expV exp(0);
      int cnt = 0;
      for (int a=0; a<=order; a++)
      {
        MonomialShapeFunctionSetCreator<C,T,dim,1>::create(a,cnt,exp,SFnts);
      }
      return cnt;
    }
  };

  /**
     \copydoc MonomialShapeFunctionSetCreator
     \internal
     Specializeses the end of the recursion
   */
  template <typename C,typename T, int dim>
  struct MonomialShapeFunctionSetCreator<C,T,dim,dim>
  {
    typedef MonomialShapeFunction<C,T,dim> ShapeFunction;
    typedef FieldVector<int, dim> expV;
    typedef Array< ShapeFunction* > sfV;
    static void create (int bound, int& cnt, expV& exp, sfV& SFnts)
    {
      exp[dim-1] = bound;
      SFnts[cnt] = new ShapeFunction(cnt, exp);
      for (int i=0; i<dim; i++)
        dvverb << exp[i] << " ";
      dvverb << std::endl;
      cnt++;
    }
  };

  /**
     \class MonomialShapeFunctionSet
     \brief a ShapeFunctionSet of monomial shapefunctions

     The shape functions map from C^d to T.

     template parameters are:
     <ul>
     <li>C: coordinate type
     <li>T: result type
     <li>d: dimension of the function space
     </ul>

     The order of the shape functions is a parameter to the constructor
   */
  template<typename C,typename T, int d>
  class MonomialShapeFunctionSet : public ShapeFunctionSet<C, T, d, 1>
  {
  public:
    enum { dim=d /*!< Dimension of the ShapeFunctionSet */ };
    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialShapeFunction<C,T,dim> ShapeFunction;

    /**
       Construct a monomial shape function set of given order
     */
    MonomialShapeFunctionSet(int order) :
      order_(order), n_(sz(order)), shapeFunctions(n_)
    {
      dvverb << "Constructing monomial shape function set of order "
             << order << std::endl;
      int i = MonomialShapeFunctionSetCreator<C,T,dim,0>::
              create(order,shapeFunctions);
      assert (n_==i);
    }

    virtual ~MonomialShapeFunctionSet()
    {
      for (int i=0; i < n_; i++)
      {
        if (shapeFunctions[i]) delete shapeFunctions[i];
      }
    }

    //! \copydoc ShapeFunctionSet::size
    virtual int size() const
    {
      assert(shapeFunctions.size() == n_);
      return n_;
    }

    //! \copydoc ShapeFunctionSet::size(int, int)
    virtual int size (int entity, int codim) const
    {
      if (codim == 0) return n_;
      return 0;
    };

    //! \copydoc ShapeFunctionSet::operator[]
    virtual const ShapeFunction& operator[](int i) const
    {
      assert (i < n_);
      assert (n_ == shapeFunctions.size());
      return *shapeFunctions[i];
    }

    //! \copydoc ShapeFunctionSet::order
    virtual int order() const
    {
      return order_;
    }

    //! \copydoc ShapeFunctionSet::type
    virtual GeometryType type () const
    {
      //! throw an exception... we dont' have a special geometry
#warning what to do here?
      DUNE_THROW(MathError, "monomial shape function don't have a geometry type");
    };

  private:
    /** order of the ShapeFunctionSet */
    const int order_;
    /** total number of shapefunctions in this set */
    const int n_;
    /** \brief calculate total number of shapefunctions in a set of given order */
    static int sz(int order)
    {
      int s;
      s = 1;
      for (int i=1; i<=dim; i++) {
        s = s * (order + i) / i;
      }
      return s;
    }
    /** vector which holds the shape functions */
    Array< ShapeFunction* > shapeFunctions;
  };

  /**
     \class MonomialShapeFunctionSetSize
     \internal
     \brief calculates the size of a MonomialShapeFunctionSet
   */
  template <int dim, int order>
  struct MonomialShapeFunctionSetSize
  {
    enum
    {
      maxSize =
        MonomialShapeFunctionSetSize<dim-1,order>::maxSize
        * (order+dim) / dim
    };
  };

  /**
     \copydoc MonomialShapeFunctionSetSize
     \internal
     end of recursion
   */
  template <int order>
  struct MonomialShapeFunctionSetSize<1,order>
  {
    enum { maxSize = order + 1 };
  };

  /**
     \class MonomialShapeFunctionSetContainer
     \brief a ShapeFunctionSetContainer of monomial shapefunctions

     The shape functions map from C^d to T.

     template parameters are:
     <ul>
     <li>C: coordinate type
     <li>T: result type
     <li>d: dimension of the function space
     <li>maxOrder: container will contain shapefunctions up to order maxOrder
     </ul>
   */
  template<typename C,typename T, int d, int maxOrder>
  class MonomialShapeFunctionSetContainer :
    public ShapeFunctionSetContainer<C,T,d,1,MonomialShapeFunctionSetSize<d,maxOrder>::maxSize >
  {
  public:
    enum { dim = d /*!< Dimension of the ShapeFunctionSetContainer */ };
    enum { maxsize = MonomialShapeFunctionSetSize<d,maxOrder>::maxSize };
    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialShapeFunctionSet<C,T,dim> ShapeFunctionSet;

    /**
       \brief Create a ShapeFunctionSetContainer of monomial shapefunctions
     */
    MonomialShapeFunctionSetContainer() :
      shapeFunctionSets(maxsize)
    {
      for (int order=0; order<=maxOrder; order++)
      {
        shapeFunctionSets[order] =
          new ShapeFunctionSet(order);
      }
    }

    virtual ~MonomialShapeFunctionSetContainer()
    {
      for (int i=0; i < maxOrder; i++)
      {
        if (shapeFunctionSets[i]) delete shapeFunctionSets[i];
      }
    }

    /** \copydoc ShapeFunctionSetContainer::operator()(Geometry type, int order) */
    virtual const ShapeFunctionSet& operator() (GeometryType type, int order) const
    {
      assert(order <= maxOrder);
      return *shapeFunctionSets[order];
    }
  private:
    /** vector which holds the shape functions */
    Array< ShapeFunctionSet* > shapeFunctionSets;
  };

  /** @} */

} //end of namespace


#endif // DUNE_MONOMIALSHAPEFUNCTIONS_HH
