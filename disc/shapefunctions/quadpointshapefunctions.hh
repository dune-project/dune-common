// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_QUADPOINTSHAPEFUNCTIONS_HH
#define DUNE_QUADPOINTSHAPEFUNCTIONS_HH

#include <list>
#include <map>
#include <vector>
#include "shapefunctions.hh"
#include "quadrature/quadraturerules.hh"

/**
 * @file dune/disc/shapefunctions/quadpointshapefunctions.hh
 * @brief  abstract base class for shape functions
 * @author Christian Engwer
 */
namespace Dune
{
  /** @addtogroup DISC_Shapefnkt
   *
   * @{
   */

  /*!
     @brief Base class for a shape function evaluated at a quadrature point.

     Encapsulates the evaluation of a ShapeFunction f
     at an integration point q.
     This class offers both interfaces -
     that of ShapeFunction and that of QuadraturePoint.

        A QuadraturePointShapeFunction has the following template parameters

     -   C       type for input coordinates x \in R^d
     -   T       return type for function values and derivatives
     -   d       the dimension of the reference domain
     -   dq      the dimension of the integration domain
     -   N       number of components per shape function

   */
  template<typename C, typename T, int d, int dq, int N>
  class QuadraturePointShapeFunction
  {
  protected:
    const ShapeFunction<C,T,d,N> * f;
    const QuadraturePoint<C,dq> * q;
    FieldVector<C,N> val;
    FieldVector<FieldVector<C,d>,N> jac;
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=N };        // must be available at compile time
    typedef C CoordType;
    typedef T ResultType;

    QuadraturePointShapeFunction(const ShapeFunction<C,T,d,N> & _f,
                                 const QuadraturePoint<C,dq> & _q)
      : f(&_f), q(&_q)
    {
      for (int comp=0; comp<N; comp++)
      {
        val[comp] = f->evaluateFunction(comp, q->position());
        for (int dir=0; dir<d; dir++)
        {
          jac[comp][dir] = f->evaluateDerivative(comp, dir, q->position());
        }
      }
    };

    template<class G>
    QuadraturePointShapeFunction(const ShapeFunction<C,T,d,N> & _f,
                                 const QuadraturePoint<C,dq> & _q,
                                 const G & geom)
      : f(&_f), q(&_q)
    {
      for (int comp=0; comp<N; comp++)
      {
        val[comp] = f->evaluateFunction(comp, geom.global(q->position()));
        for (int dir=0; dir<d; dir++)
        {
          jac[comp][dir] = f->evaluateDerivative(comp, dir,
                                                 geom.global(q->position()));
        }
      }
    };

    //! evaluate component comp at this integration point
    ResultType evaluateFunction (int comp) const
    {
      return val[comp];
    };

    //! evaluate derivative of component comp in direction dir
    //! at this integration point
    ResultType evaluateDerivative (int comp, int dir) const
    {
      return jac[comp][dir];
    };

    //! consecutive number of associated dof within element
    int localindex (int comp) const
    {
      return f->localindex(comp);
    };

    //! codim of associated dof
    int codim () const
    {
      return f->codim();
    };

    //! entity (of codim) of associated dof
    int entity () const
    {
      return f->entity();
    };

    //! consecutive number of dof within entity
    int entityindex () const
    {
      return f->entityindex();
    };

    //! return local coordinates of this integration point
    const FieldVector<C, dq>& position () const
    {
      return q->position();
    }

    //! return weight associated with this integration point
    double weight () const
    {
      return q->weight();
    }

  };

  /*!
     @brief Base class for a shape function evaluated for a quadrature rule.

     Encapsulates the evaluation of all shape functions in a ShapeFunctionSet
     at an integration point i.

     You can use QuadraturePointShapeFunctionSet::const_iterator of operator[]
     to access a single QuadraturePointShapeFunction.

        A QuadraturePointShapeFunctionSet has the following template parameters

     -    C       type for input coordinates
     -    T       return type for function values and derivatives
     -    d       the dimension of the reference domain
     -    dq      the dimension of the integration domain
     -    N       number of components per shape function

   */
  template<typename C, typename T, int d, int dq, int N>
  class QuadraturePointShapeFunctionSet :
    public std::vector< QuadraturePointShapeFunction<C,T,d,dq,N> >
  {
    const ShapeFunctionSet<C,T,d,N> & fset;
    const QuadraturePoint<C,dq> & q;
  public:
    // compile time sizes
    enum { dim=d };
    enum { dimQuad=dq };
    enum { comps=N };

    // exported types
    typedef C CoordType;
    typedef T ResultType;

    QuadraturePointShapeFunctionSet(const ShapeFunctionSet<C,T,d,N> & _fset,
                                    const QuadraturePoint<C,dq> & _q) :
      fset(_fset), q(_q)
    {
      // cache the results
      for (int i=0; i<fset.size(); i++)
        push_back(QuadraturePointShapeFunction<C,T,d,dq,N>(fset[i],q));
    };

    template<class G>
    QuadraturePointShapeFunctionSet(const ShapeFunctionSet<C,T,d,N> & _fset,
                                    const QuadraturePoint<C,dq> & _q,
                                    const G & geom) :
      fset(_fset), q(_q)
    {
      // cache the results
      for (int i=0; i<fset.size(); i++)
        push_back(QuadraturePointShapeFunction<C,T,d,dq,N>(fset[i],q,geom));
    };

    //! total number of shape functions, i.e. degrees of freedom
    int size () const
    {
      return fset.size();
    }

    //! total number of shape functions associated with entity in codim
    int size (int entity, int codim) const
    {
      return fset.size(entity, codim);
    }

    //! return order
    int order () const
    {
      return fset.order();
    }

    //! return type of element where this QuadraturePointShapeFunctionSet
    //! is for
    NewGeometryType type () const
    {
      return fset.type();
    };

    //! return local coordinates of integration point i
    const FieldVector<C, dq>& position () const
    {
      return q.position();
    }

    //! return weight associated with integration point i
    double weight () const
    {
      return q.weight();
    }

  };

  /*!
     @brief Base class for a shape function set evaluated for a quadrature rule.

     Encapsulates the evaluation of all shape functions in a ShapeFunctionSet
     at all integration point of a QuadratureRule.

     You can use QuadratureRuleShapeFunctionSet::const_iterator of operator[]
     to access a single QuadraturePointShapeFunctionSet.

        A QuadratureRuleShapeFunctionSet has the following template parameters

     -    C       type for input coordinates
     -    T       return type for function values and derivatives
     -    d       the dimension of the reference domain
     -    dq      the dimension of the integration domain
     -    N       number of components per shape function

   */
  template<typename C, typename T, int d, int dq, int N>
  class QuadratureRuleShapeFunctionSet :
    public std::list< QuadraturePointShapeFunctionSet<C,T,d,dq,N> >
  {
    const ShapeFunctionSet<C,T,d,N> & fset;
    const QuadratureRule<C,dq> & qr;

  public:
    // compile time sizes
    enum { dim=d };
    enum { dimQuad=dq };
    enum { comps=N };        // must be available at compile time
    typedef C CoordType;
    typedef T ResultType;

    QuadratureRuleShapeFunctionSet(const ShapeFunctionSet<C,T,d,N> & _fset,
                                   const QuadratureRule<C,dq> & _qr) :
      fset(_fset), qr(_qr)
    {
      typename QuadratureRule<C,dq>::const_iterator it = qr.begin();
      typename QuadratureRule<C,dq>::const_iterator endit = qr.end();
      for (; it != endit; ++it)
        push_back(QuadraturePointShapeFunctionSet<C,T,d,dq,N>(fset,*it));
    };

    template<class G>
    QuadratureRuleShapeFunctionSet(const ShapeFunctionSet<C,T,d,N> & _fset,
                                   const QuadratureRule<C,dq> & _qr,
                                   const G & geom) :
      fset(_fset), qr(_qr)
    {
      typename QuadratureRule<C,dq>::const_iterator it = qr.begin();
      typename QuadratureRule<C,dq>::const_iterator endit = qr.end();
      for (; it != endit; ++it)
        push_back(QuadraturePointShapeFunctionSet<C,T,d,dq,N>(fset,*it,geom));
    };

    //! return order
    int order () const
    {
      return qr.order();
    }

    //! return type of element
    NewGeometryType type () const
    {
      return qr.type();
    }

  };

  namespace QuadPointHash {
    /* Fnv_hash is copied from gcc stl extensions */

    // Dummy generic implementation (for sizeof(size_t) != 4, 8).
    template<std::size_t = sizeof(std::size_t)>
    struct Fnv_hash
    {
      static std::size_t
      hash(const char* first, std::size_t length)
      {
        std::size_t result = 0;
        for (; length > 0; --length)
          result = (result * 131) + *first++;
        return result;
      }
    };

    template<>
    struct Fnv_hash<4>
    {
      static std::size_t
      hash(const char* first, std::size_t length)
      {
        std::size_t result = 2166136261UL;
        for (; length > 0; --length)
        {
          result ^= (std::size_t)*first++;
          result *= 16777619UL;
        }
        return result;
      }
    };

    template<>
    struct Fnv_hash<8>
    {
      static std::size_t
      hash(const char* first, std::size_t length)
      {
        std::size_t result = 14695981039346656037ULL;
        for (; length > 0; --length)
        {
          result ^= (std::size_t)*first++;
          result *= 1099511628211ULL;
        }
        return result;
      }
    };
  }

  /*!
     @brief Abstract base class for a container of sets of shape functions.

     Base class for a shape function set evaluated for a quadrature rule.

     A QuadratureRuleShapeFunctionSetContainer is a container of
        QuadratureRuleShapeFunctionSets. Its elements can be accessed by
        providing a ShapeFunctionSets, an element type and the order. The
        returned QuadratureRuleShapeFunctionSet can be used to cache the
        evaluation of shape functions in a ShapeFunctionSets and at
        integration points of QuadratureRule.

        A QuadratureRuleShapeFunctionSetContainer
     has the following template parameters

     -    C       type for input coordinates
     -    T       return type for function values and derivatives
     -    d       the dimension of the reference domain
     -    dq      the dimension of the integration domain
     -    N       number of components per shape function

   */
  template<typename C, typename T, int d, int dq, int N>
  class QuadratureRuleShapeFunctionSetCache
  {
    friend struct Id;
    int hit;
    int miss;
    int cacheSize;
  public:

    class Id {
    public:
      int operator== (const Id & id) const
      {
        if (typeid(*this) == typeid(id))
        {
          return id.equals(*this);
        }
        return false;
      };
      virtual int equals(const Id & id) const = 0;
      virtual std::size_t hash() const = 0;
    };

    class BasicId : public Id {
      const void * p_fset;
      const void * p_quad;
    public:
      BasicId(const ShapeFunctionSet<C,T,d,N> & fset,
              const QuadratureRule<C,dq> & quad) :
        p_fset(&fset), p_quad(&quad) {};
      virtual int equals(const Id & _id) const
      {
        const BasicId & id = dynamic_cast<const BasicId&>(_id);
        return (p_fset == id.p_fset
                && p_quad == id.p_quad);
      }
      virtual std::size_t hash() const
      {
        const char * data = reinterpret_cast<const char *>(this);
        return QuadPointHash::Fnv_hash<>::hash(data, sizeof(*this));
      }
    };

    template<class Geometry>
    class GeometryId : public Id {
      const void * p_fset;
      const void * p_quad;
      const Geometry geom;
    public:
      GeometryId(const ShapeFunctionSet<C,T,d,N> & fset,
                 const QuadratureRule<C,dq> & quad,
                 const Geometry & g) :
        p_fset(&fset), p_quad(&quad), geom(g) {};
      virtual int equals(const Id & _id) const
      {
        const GeometryId & id = dynamic_cast<const GeometryId&>(_id);
        return (p_fset == id.p_fset
                && p_quad == id.p_quad
                && geom == id.geom);
      }
      virtual std::size_t hash() const
      {
        const char * data = reinterpret_cast<const char *>(this);
        return QuadPointHash::Fnv_hash<>::hash(data, sizeof(*this));
      }
    };

    // compile time sizes
    enum { dim=d };
    enum { comps=N };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef QuadratureRuleShapeFunctionSet<C,T,d,dq,N>
    QuadratureRuleShapeFunctionSet;
    typedef std::pair<Id*,QuadratureRuleShapeFunctionSet*> CacheEntry;
    typedef std::vector<CacheEntry> Cache;

    //! default Constructor
    QuadratureRuleShapeFunctionSetCache() :
      hit(0), miss(0), cacheSize(163), cache(cacheSize)
    {
      initCache();
    };

    /*!
       @brief Create a QuadratureRuleShapeFunctionSetCache with sz cache lines.

       It is recommend the you use a prime number of cache lines.
     */
    QuadratureRuleShapeFunctionSetCache(int sz) :
      hit(0), miss(0), cacheSize(sz), cache(cacheSize)
    {
      initCache();
    };

    //! access a shape function via type and order
    const QuadratureRuleShapeFunctionSet&
    operator() (const ShapeFunctionSet<C,T,d,N> & fset, int order)
    {
      const QuadratureRule<C,dq> & quad =
        QuadratureRules<C,dq>::rule(fset.type(), order);
      Id * id = new BasicId(fset,quad);

      int hash = (id->hash() + 31) % cacheSize;
      assert(hash < cacheSize);

      CacheEntry & entry = cache[hash];
      if (entry.first != 0 && (*entry.first) == (*id))
      {
        assert(entry.second != 0);
        hit++;
        return *(entry.second);
      }
      else
      {
        miss++;
        if (entry.first != 0)
          delete entry.first;
        if (entry.second != 0)
          delete entry.second;
        entry.first = id;
        entry.second = new QuadratureRuleShapeFunctionSet(fset, quad);
        return *(entry.second);
      }
    }

    //! access a QuadratureRuleShapeFunctionSet via ShapeFunctionSet
    //! QuadratureRule
    const QuadratureRuleShapeFunctionSet&
    operator() (const ShapeFunctionSet<C,T,d,N> & fset,
                const QuadratureRule<C,dq> & quad)
    {
      Id * id = new BasicId(fset, quad);

      int hash = (id->hash() + 31) % cacheSize;
      assert(hash < cacheSize);

      CacheEntry & entry = cache[hash];
      if (entry.first != 0 && (*entry.first) == (*id))
      {
        assert(entry.second != 0);
        hit++;
        return *(entry.second);
      }
      else
      {
        miss++;
        if (entry.first != 0)
          delete entry.first;
        if (entry.second != 0)
          delete entry.second;
        entry.first = id;
        entry.second = new QuadratureRuleShapeFunctionSet(fset, quad);
        return *(entry.second);
      }
    }

    //! access a QuadratureRuleShapeFunctionSet via ShapeFunctionSet
    //! QuadratureRule and Geometry
    template<class G>
    const QuadratureRuleShapeFunctionSet&
    operator() (const ShapeFunctionSet<C,T,d,N> & fset,
                const QuadratureRule<C,dq> & quad,
                const G & geom)
    {
      Id * id = new GeometryId<G>(fset, quad, geom);

      int hash = (id->hash() + 31) % cacheSize;
      assert(hash < cacheSize);

      CacheEntry & entry = cache[hash];
      if (entry.first != 0 && (*entry.first) == (*id))
      {
        assert(entry.second != 0);
        hit++;
        return *(entry.second);
      }
      else
      {
        miss++;
        if (entry.first != 0)
          delete entry.first;
        if (entry.second != 0)
          delete entry.second;
        entry.first = id;
        cache[hash].second =
          new QuadratureRuleShapeFunctionSet(fset, quad, geom);
        return *(cache[hash].second);
      }
    }

    ~QuadratureRuleShapeFunctionSetCache()
    {
      // delete cache entries
    }

    void print()
    {
      std::cout << "Cache statistics:\n"
                << "Hits: " << hit << "\n"
                << "Misses: " << miss << "\n";
    }
  private:

    void initCache()
    {
      typename Cache::iterator it=cache.begin();
      typename Cache::iterator endit=cache.end();
      int c=0;
      for (; it!=endit; ++it)
      {
        it->first = 0;
        it->second = 0;
        c++;
      }
    }

    Cache cache;
  };

  /** @} */
}
#endif // DUNE_QUADPOINTSHAPEFUNCTIONS_HH
