// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//==============================================================================
//!
//! \file shapefunctions.hpp
//!
//! \date Nov 9 2011
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Classes for shape functions. Loosely based on code in dune-grid-howto
//!
//==============================================================================

#include <dune/common/fvector.hh>
#include <dune/common/dynmatrixev.hh>

#include <complex>


//! \brief Represents a cardinal function on a line
template<class ctype, class rtype>
class LagrangeCardinalFunction
{
public:
  //! \brief Empty default constructor
  LagrangeCardinalFunction() {}

  //! \brief Construct a cardinal function with the given nodes
  //! \param[in] nodes_ The nodes
  //! \param[in] i The node this function is associated with
  LagrangeCardinalFunction(const std::vector<rtype>& nodes_,
                           size_t i)
    : nodes(nodes_), node(i) {}

  //! \brief Evaluate the shape function
  //! \param[in] local The local coordinates
  rtype evaluateFunction(const ctype& local) const
  {
    rtype result = 1;
    for (size_t i=0; i < nodes.size(); ++i) {
      if (i != node)
        result *= (local-nodes[i])/(nodes[node]-nodes[i]);
    }

    return result;
  }

  //! \brief Evaluate the derivative of the cardinal function
  //! \param[in] local The local coordinates
  rtype evaluateGradient(const ctype& local) const
  {
    rtype result = 0;
    for (size_t i=0; i < nodes.size(); ++i) {
      rtype f = 1;
      for (int j=0; j < nodes.size(); ++j) {
        if (i != j && j != node)
          f *= (local-nodes[j])/(nodes[node]-nodes[j]);
      }
      result += f/(nodes[node]-nodes[i]);
    }

    return result;
  }

private:
  //! \brief The nodes
  std::vector<rtype> nodes;

  size_t node;
};

//! \brief Represents a tensor-product of 1D functions
template<class rtype, class ctype, class ftype, int dim>
class TensorProductFunction
{
public:
  //! \brief The dimension of the function
  enum { dimension = dim };

  //! \brief Empty default constructor
  TensorProductFunction() {}

  //! \brief Construct a tensor-product function
  //! \param[in] funcs_ The functions
  TensorProductFunction(const Dune::FieldVector<ftype, dim>& funcs_)
    : funcs(funcs_) {}

  //! \brief Evaluate the function
  //! \param[in] local The local coordinates
  rtype evaluateFunction(const Dune::FieldVector<ctype,dim>& local) const
  {
    rtype result = 1;
    for (int i=0; i < dim; ++i)
      result *= funcs[i].evaluateFunction(local[i]);

    return result;
  }

  Dune::FieldVector<rtype, dim>
  evaluateGradient(const Dune::FieldVector<ctype,dim>& local) const
  {
    Dune::FieldVector<rtype, dim> result;
    for (int i=0; i < dim; ++i)
      result[i] = funcs[i].evaluateGradient(local[i]);
  }
private:
  Dune::FieldVector<ftype, dim> funcs;
};

template<int dim>
class PNShapeFunctionSet
{
public:
  typedef LagrangeCardinalFunction<double, double> CardinalFunction;

  typedef TensorProductFunction<double, double, CardinalFunction, dim>
  ShapeFunction;

  PNShapeFunctionSet(int n1, int n2, int n3=0)
  {
    int dims[3] = {n1, n2, n3};
    cfuncs.resize(dim);
    for (int i=0; i < dim; ++i) {
      std::vector<double> grid;
      grid = gaussLobattoLegendreGrid(dims[i]);
      for (int j=0; j<dims[i]; ++j)
        cfuncs[i].push_back(CardinalFunction(grid,j));
    }
    int l=0;
    Dune::FieldVector<CardinalFunction,dim> fs;
    if (dim == 3) {
      f.resize(n1*n2*n3);
      for (int k=0; k < n3; ++k) {
        for (int j=0; j < n2; ++j)
          for (int i=0; i< n1; ++i) {
            fs[0] = cfuncs[0][i];
            fs[1] = cfuncs[1][j];
            fs[2] = cfuncs[2][k];
            f[l++] = ShapeFunction(fs);
          }
      }
    } else {
      f.resize(n1*n2);
      for (int j=0; j < n2; ++j) {
        for (int i=0; i< n1; ++i) {
          fs[0] = cfuncs[0][i];
          fs[1] = cfuncs[1][j];
          f[l++] = ShapeFunction(fs);
        }
      }
    }
  }

  //! \brief Obtain a given shape function
  //! \param[in] i The requested shape function
  const ShapeFunction& operator[](int i) const
  {
    return f[i];
  }

  int size()
  {
    return f.size();
  }
protected:
  std::vector< std::vector<CardinalFunction> > cfuncs;
  std::vector<ShapeFunction> f;

  double legendre(double x, int n)
  {
    std::vector<double> Ln;
    Ln.resize(n+1);
    Ln[0] = 1.f;
    Ln[1] = x;
    if( n > 1 ) {
      for( int i=1; i<n; i++ )
        Ln[i+1] = (2*i+1.0)/(i+1.0)*x*Ln[i]-i/(i+1.0)*Ln[i-1];
    }

    return Ln[n];
  }

  double legendreDerivative(double x, int n)
  {
    std::vector<double> Ln;
    Ln.resize(n+1);

    Ln[0] = 1.0; Ln[1] = x;

    if( (x == 1.0) || (x == -1.0) )
      return( pow(x,n-1)*n*(n+1.0)/2.0 );
    else {
      for( int i=1; i<n; i++ )
        Ln[i+1] = (2.0*i+1.0)/(i+1.0)*x*Ln[i]-(double)i/(i+1.0)*Ln[i-1];
      return( (double)n/(1.0-x*x)*Ln[n-1]-n*x/(1-x*x)*Ln[n] );
    }
  }


  std::vector<double> gaussLegendreGrid(int n)
  {
    Dune::DynamicMatrix<double> A(n,n,0.0);

    A[0][1] = 1.f;
    for (int i=1; i<n-1; ++i) {
      A[i][i-1] = (double)i/(2.0*(i+1.0)-1.0);
      A[i][i+1] = (double)(i+1.0)/(2*(i+1.0)-1.0);
    }
    A[n-1][n-2] = (n-1.0)/(2.0*n-1.0);

    Dune::DynamicVector<std::complex<double> > eigenValues(n);
    Dune::DynamicMatrixHelp::eigenValuesNonSym(A, eigenValues);

    std::vector<double> result(n);
    for (int i=0; i < n; ++i)
      result[i] = std::real(eigenValues[i]);
    std::sort(result.begin(),result.begin()+n);

    return result;
  }

  std::vector<double> gaussLobattoLegendreGrid(int n)
  {
    assert(n > 1);
    const double tolerance = 1.e-15;

    std::vector<double> result(n);
    result[0] = 0.0;
    result[n-1] = 1.0;
    if (n == 3)
      result[1] = 0.5;

    if (n < 4)
      return result;

    std::vector<double> glgrid = gaussLegendreGrid(n-1);
    for (int i=1; i<n-1; ++i) {
      result[i] = (glgrid[i-1]+glgrid[i])/2.0;
      double old = 0.0;
      while (std::abs(old-result[i]) > tolerance) {
        old = result[i];
        double L = legendre(old, n-1);
        double Ld = legendreDerivative(old, n-1);
        result[i] += (1.0-old*old)*Ld/((n-1.0)*n*L);
      }
      result[i] = (result[i]+1.0)/2.0;
    }

    return result;
  }
};

int main()
{
  // Not really a test but better than nothing.
  PNShapeFunctionSet<2> lbasis(2, 3);
  return 0;
}
