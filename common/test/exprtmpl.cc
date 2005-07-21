// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/istl/bvector.hh>
#include <dune/istl/io.hh>
#include <dune/common/iteratorfacades.hh>

Indent INDENT;

void test_fvector()
{
  typedef Dune::FieldVector<double,2> VB;
  VB v1(1);
  VB v2(2);

  typedef Dune::ExprTmpl::ConstRef<VB> RVB;
  VB v = 0.5 * (v1 + v2 * 2) + 3 * v1 - v2;
  std::cout << " 0.5 * ( " << v1 << " + " << v2 << " * 2) + 3 * " << v1 << " - " << v2 << std::endl;
}

void test_blockvector()
{
  Dune::FieldVector<double,2> v(10);
  typedef Dune::FieldVector<double,2> VB;
  typedef Dune::BlockVector<VB> BV;

  const int sz = 3;
  BV bv1(sz), bv2(sz);
  bv1 = 1;
  bv2 = 0;
  bv2[1][0]=1;
  bv2[1][1]=2;

  BV bv(sz);
  bv = -17;
  printvector (std::cout, bv, "bv", "r");
  //  bv.emptyClone(bv1);
  std::cout << "Assingn from ConstRef\n";
  bv = 2 * (bv1 + bv2);
  bv -= 1;

  printvector (std::cout, bv1, "bv1", "r");
  printvector (std::cout, bv2, "bv2", "r");
  printvector (std::cout, bv, "bv", "r");

}

void test_blockblockvector()
{
  typedef Dune::FieldVector<double,2> VB;
  typedef Dune::BlockVector<VB> BV;
  typedef Dune::BlockVector<BV> BBV;
  typedef Dune::ExprTmpl::ConstRef<BV> RBV;
  const int sz = 3;
  BV bv1(sz), bv2(sz);
  bv1 = 1;
  bv2 = 0;
  bv2[1][0]=1;
  bv2[1][1]=2;

  Dune::ExprTmpl::ConstRef<BV> rbv1(bv1);
  Dune::ExprTmpl::ConstRef<BV> rbv2(bv2);

  BBV bbv(2);
  bbv[0].resize(bv1.N());
  bbv[0] = Dune::ExprTmpl::Expression<RBV>(rbv1);
  bbv[1].resize(bv2.N());
  bbv[1] = Dune::ExprTmpl::Expression<RBV>(rbv2);
  Dune::FlatIterator<BBV> fit(bbv.begin());
  Dune::FlatIterator<BBV> fend(bbv.end());
  int index = 0;
  for(; fit!=fend; ++fit)
  {
    BBV::field_type x;
    x = *fit;
    std::cout << index << "\t" << x << std::endl;
    index++;
  }
  printvector (std::cout, bv1, "bv1", "r");
  printvector (std::cout, bv2, "bv1", "r");
  printvector (std::cout, bbv, "bbv", "r");
}

namespace Dune {

  /**
     B: rhs Vector type (usually a BlockType)
     M: lhs Matrix type
     V: lhs Vector type
   */
  // !TODO!
  template <class B, class M, class V>
  class BlockRowSum
  {
  public:
    typedef typename M::ConstColIterator ConstColIterator;
    typedef typename M::row_type row_type;
    typedef BlockRowSum<typename BlockType<B>::type,M,V> SubBlockRowSum;
    typedef typename Dune::FieldType<V>::type field_type;
    //  typedef typename
    BlockRowSum(const row_type & _r, const V & _v) : r(_r), v(_v) {};
    SubBlockRowSum operator[] (int i) const {
      return SubBlockRowSum(r[i],v);
    }
    int N() const { return r.begin()->N(); }
  private:
    const row_type & r;
    const V & v;
  };

  // template <class K, int n, class M, class V>
  // class BlockRowSum< FieldVector<K,n>, M, V >
  template <class M, class V>
  class BlockRowSum< void, M, V >
  {
  public:
    typedef typename M::ConstColIterator ConstColIterator;
    typedef typename M::row_type row_type;
    typedef typename FieldType<V>::type K;
    BlockRowSum(const row_type & _r, const V & _v) : r(_r), v(_v) {};
    K operator[] (int i) const {
      K x=0;
      ConstColIterator j=r.begin();
      ConstColIterator endj = r.end();
      for (; j!=endj; ++j)
      {
        /*
           x += (MatrixBlock * VectorBlock  )[i]
         */
        //      std::cout << "BRS x=" << x << std::endl;
        x   += (     (*j)   * v[j.index()] )[i];
      }
      //    std::cout << "BRS x=" << x << std::endl;
      return x;
    }
    int N() const { return r.begin()->N(); }
  private:
    const row_type & r;
    const V & v;
  };

  template <class M, class V>
  class MV
  {
  public:
    MV(const M & _A, const V & _v) : A(_A), v(_v) {};
    BlockRowSum<void,M,V> operator[] (int i) const {
      // !TODO!
      return BlockRowSum<void,M,V>(A[i],v);
    }
    int N() const { return A.N(); }
  private:
    const M & A;
    const V & v;
  };

  template <class K, int iM, int iN>
  class MV< Dune::FieldMatrix<K,iN,iM>, Dune::FieldVector<K,iM> >
  {
  public:
    typedef Dune::FieldMatrix<K,iN,iM> M;
    typedef Dune::FieldVector<K,iM> V;
    typedef typename Dune::FieldType<V>::type field_type;
    MV(const M & _A, const V & _v) : A(_A), v(_v) {};
    field_type operator[] (int i) const {
      field_type x=0;
      for (int j=0; j<iM; ++j) {
        x += A[i][j] * v[j];
      }
      return x;
    }
    int N() const { return A.N(); }
  private:
    const M & A;
    const V & v;
  };

  template<class K, int N, int M>
  ExprTmpl::Expression< MV< FieldMatrix<K,N,M>, FieldVector<K,M> > >
  operator * ( const FieldMatrix<K,N,M> & A, const FieldVector<K,M> & v )
  {
    return ExprTmpl::Expression< MV< FieldMatrix<K,N,M>, FieldVector<K,M> > >
             ( MV< FieldMatrix<K,N,M>, FieldVector<K,M> > (A,v) );
  }

  template<class BM, class BV>
  ExprTmpl::Expression< MV< BCRSMatrix<BM>, BlockVector<BV> > >
  operator * ( const BCRSMatrix<BM> & A, const BlockVector<BV> & v )
  {
    return ExprTmpl::Expression< MV< BCRSMatrix<BM>, BlockVector<BV> > >
             ( MV< BCRSMatrix<BM>, BlockVector<BV> > (A,v) );
  }

  template <class A, class B>
  struct FieldType< MV<A,B> >
  {
    typedef typename FieldType<B>::type type;
  };

  template <class B, class A, class V>
  struct FieldType< BlockRowSum<B,A,V> >
  {
    typedef typename FieldType<V>::type type;
  };

  namespace ExprTmpl {

    // !TODO!
    template <class A, class B>
    struct BlockExpression< MV< BCRSMatrix<A>,BlockVector<B> > >
    {
      typedef ExprTmpl::Expression< BlockRowSum< void, BCRSMatrix<A>,BlockVector<B> > > type;
    };

    // !TODO!
    template <class A, class B>
    struct BlockExpression< BlockRowSum< void, BCRSMatrix<A>,BlockVector<B> > >
    {
      typedef typename B::field_type type;
    };

    template <class K, int N, int M>
    struct BlockExpression< MV< FieldMatrix<K,N,M>, FieldVector<K,M> > >
    {
      typedef K type;
    };

  }
}

void test_matrix()
{
  static const int BlockSize = 2;
  typedef double matvec_t;
  typedef Dune::FieldVector<matvec_t,BlockSize+1> VB;
  typedef Dune::FieldVector<matvec_t,BlockSize> LVB;
  typedef Dune::FieldMatrix<matvec_t,BlockSize,BlockSize+1> MB;
  typedef Dune::BlockVector<LVB> LeftVector;
  typedef Dune::BlockVector<VB> Vector;
  typedef Dune::BCRSMatrix<MB> Matrix;

  LVB a(0);
  VB b(2);
  MB _M(1);
  _M[1][1] = 3;

  // a += M * b
  _M.umv(b,a);

  printmatrix (std::cout, _M, "Matrix", "r");
  printvector (std::cout, a, "Vector", "r");

  // a = M * b
  a = _M*b;

  printvector (std::cout, a, "Vector", "r");

  int N = 4;
  int M = 5;

  Matrix A(N,M,Matrix::row_wise);
  Matrix::CreateIterator i=A.createbegin();
  Matrix::CreateIterator end=A.createend();
  // build up the matrix structure
  int c=0;
  for (; i!=end; ++i)
  {
    // insert a non zero entry for myself
    i.insert(c);
    // insert index M-1
    i.insert(M-1);
    c++;
  }
  A = 0.0;

  std::cout << "Matrix coldim=" << A.coldim() << std::endl;
  std::cout << "Matrix rowdim=" << A.rowdim() << std::endl;
  std::cout << "Matrix N=" << A.M() << std::endl;
  std::cout << "Matrix M=" << A.N() << std::endl;

  Matrix::Iterator cit=A.begin();
  Matrix::Iterator cend=A.end();
  for (; cit!=cend; ++cit)
  {
    *cit = cit.index();
  }

  printmatrix (std::cout, A, "Matrix", "r");

  LeftVector v(N);
  LeftVector v2(N);
  v = 0;
  Vector x(M);
  x = 1;
  x[M-1] = 2;

  std::cout << A.M() << " " << x.N() << " " << v.N() << std::endl;

  A.umv(x,v);

  printvector (std::cout, v, "Vector", "r");
  /*
     v=1;
     MV<Matrix,Vector> eximp (A,x);
     Dune::ExprTmpl::Expression< MV<Matrix,Vector> > ex ( eximp );
     v =
     printvector (std::cout, v, "Vector", "r");
   */
  v2 = A * x;

  printvector (std::cout, v2, "Vector", "r");

}

int main()
{
  //  Dune::dvverb.attach(std::cout);
  try
  {
    test_fvector();
    test_blockvector();
    test_blockblockvector();
    test_matrix();
    exit(0);
  }
  catch (Dune::Exception & e)
  {
    std::cout << e << std::endl;
  }
}
