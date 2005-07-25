// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/*
   TODO:
   - test deeper Matrix nesting
   - get rid of
    int *M;
   - fix RowBlock::N()
   - remove second template parameter of FlatColIterator
 */

#include <iostream>
#include <fstream>
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

  //! Type Traits for Vector::Iterator vs (const Vector)::ConstIterator
  template<class T>
  struct ColIteratorType
  {
    typedef typename T::ColIterator type;
  };

  template<class T>
  struct ColIteratorType<const T>
  {
    typedef typename T::ConstColIterator type;
  };

  //! Iterator class for flat sequential access to a nested Matrix Row
  template<class A, class V>
  class FlatColIterator :
    public ForwardIteratorFacade<FlatColIterator<A,V>,
        typename FieldType<A>::type,
        typename FieldType<A>::type&,
        int>
  {
  public:
    typedef typename ColIteratorType<A>::type ColBlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    //    typedef typename BlockIterator::DifferenceType DifferenceType;
    typedef typename BlockType<A>::type block_type;
    typedef typename BlockType<V>::type vblock_type;
    typedef typename FieldType<A>::type field_type;
    typedef FlatColIterator<block_type, vblock_type> SubBlockIterator;
    FlatColIterator(const ColBlockIterator & i, const int* _M) :
      M(_M), it(i),
      bit((*i)[(*M)].begin(), M+1),
      bend((*i)[(*M)].end(), M+1) {};
    void increment ()
    {
      ++bit;
      if (bit == bend)
      {
        ++it;
        bit = (*it)[(*M)].begin();
        bend = (*it)[(*M)].end();
      }
    }
    bool equals (const FlatColIterator & fit) const
    {
      return fit.it == it && fit.bit == bit;
    }
    const field_type& dereference() const
    {
      return *bit;
    }
    const field_type& vectorentry(const V & v) const
    {
      return bit.vectorentry(v[it.index()]);
    }
    //! return index
    DifferenceType index () const
    {
      return bit.index();
    }
    FlatColIterator operator = (const ColBlockIterator & _i)
    {
      it = _i;
      bit = (*it)[(*M)].begin();
      bend = (*it)[(*M)].end();
      return *this;
    }
  private:
    const int* M;
    ColBlockIterator it;
    SubBlockIterator bit;
    SubBlockIterator bend;
  };

  template<class K, int N, int M>
  class FlatColIterator<FieldMatrix<K,N,M>, FieldVector<K,M> > :
    public ForwardIteratorFacade<
        FlatColIterator< FieldMatrix<K,N,M>, FieldVector<K,M> >, K, K&, int>
  {
  public:
    typedef
    typename ColIteratorType< FieldMatrix<K,N,M> >::type ColBlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    typedef K field_type;
    FlatColIterator(const ColBlockIterator & i, const int*) :
      it(i) {};
    void increment ()
    {
      ++it;
    }
    bool equals (const FlatColIterator & fit) const
    {
      return fit.it == it;
    }
    field_type& dereference() const
    {
      return *it;
    }
    const field_type& vectorentry(const FieldVector<K,M> & v) const
    {
      return v[it.index()];
    }
    //! return index
    DifferenceType index () const
    {
      return it.index();
    }
    FlatColIterator operator = (const ColBlockIterator & _i)
    {
      it = _i;
      return *this;
    }
  private:
    ColBlockIterator it;
  };

  template<class K, int N, int M>
  class FlatColIterator<const FieldMatrix<K,N,M>, const FieldVector<K,M> > :
    public ForwardIteratorFacade<
        FlatColIterator< const FieldMatrix<K,N,M>, const FieldVector<K,M> >,
        const K, const K&, int>
  {
  public:
    typedef
    typename ColIteratorType< const FieldMatrix<K,N,M> >::type ColBlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    typedef const K field_type;
    FlatColIterator(const ColBlockIterator & i, const int*) :
      it(i) {};
    void increment ()
    {
      ++it;
    }
    bool equals (const FlatColIterator & fit) const
    {
      return fit.it == it;
    }
    field_type& dereference() const
    {
      return *it;
    }
    const field_type& vectorentry(const FieldVector<K,M> & v) const
    {
      return v[it.index()];
    }
    //! return index
    DifferenceType index () const
    {
      return it.index();
    }
    FlatColIterator operator = (const ColBlockIterator & _i)
    {
      it = _i;
      return *this;
    }
  private:
    ColBlockIterator it;
  };

  template<class M>
  struct NestedDepth
  {
    enum { value = NestedDepth<typename BlockType<M>::type>::value + 1 };
  };

  template<class K, int N, int M>
  struct NestedDepth< FieldMatrix<K,N,M> >
  {
    enum { value = 1 };
  };

  template<class Me, class M>
  struct MyDepth
  {
    enum { value = MyDepth<Me,typename BlockType<M>::type>::value+1 };
  };

  template<class Me>
  struct MyDepth<Me,Me>
  {
    enum { value = 0 };
  };

  /**
     B: BlockMatrix type -> indicated the current level.
     M: ,,global'' Matrix type
     V: ,,global'' Vector type
   */
  template <class B, class Mat, class Vec>
  class RowBlock
  {
  public:
    typedef RowBlock<typename BlockType<B>::type,Mat,Vec> SubRowBlock;
    typedef typename Dune::FieldType<Vec>::type field_type;
    RowBlock(const Mat & _A, const Vec & _v, int* _M) :
      M(_M), A(_A), v(_v) {};
    SubRowBlock operator[] (int i) const {
      M[MyDepth<B,Mat>::value] = i;
      return SubRowBlock(A,v,M);
    }
    int N() const { return -1; }; //r.begin()->N(); }
  private:
    mutable int* M;
    const Mat & A;
    const Vec & v;
  };

  // template <class K, int n, class M, class V>
  // class BlockRowSum< FieldVector<K,n>, M, V >
  template <class K, int iN, int iM, class Mat, class Vec>
  class RowBlock< FieldMatrix<K,iN,iM>, Mat, Vec >
  {
  public:
    RowBlock(const Mat & _A, const Vec & _v, int* _M) :
      M(_M), A(_A), v(_v) {};
    K operator[] (int i) const {
      K x=0;
      M[MyDepth<FieldMatrix<K,iN,iM>,Mat>::value] = i;

      FlatColIterator<const Mat, const Vec> j(A[*M].begin(),M+1);
      FlatColIterator<const Mat, const Vec> endj(A[*M].end(),M+1);
      for (; j!=endj; ++j)
      {
        x   += (*j) * j.vectorentry(v);
      }
      return x;
    }
    int N() const { return -1; }; //r.begin()->N(); }
  private:
    mutable int* M;
    const Mat & A;
    const Vec & v;
  };

  template <class M, class V>
  class MV
  {
  public:
    MV(const M & _A, const V & _v) : A(_A), v(_v) {};
    RowBlock<typename BlockType<M>::type,M,V> operator[] (int i) const {
      indizes[0] = i;
      return RowBlock<typename BlockType<M>::type,M,V>(A, v, indizes);
    }
    int N() const { return A.N(); }
  private:
    mutable int indizes[256];
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
      std::cout << "ARGH\n";
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
  struct FieldType< RowBlock<B,A,V> >
  {
    typedef typename FieldType<V>::type type;
  };

  namespace ExprTmpl {

    template <class A, class B>
    struct BlockExpression< MV< BCRSMatrix<A>,BlockVector<B> > >
    {
      typedef ExprTmpl::Expression< RowBlock< A, BCRSMatrix<A>,BlockVector<B> > > type;
    };

#if 0
    // !TODO!
    template <class A, class B>
    struct BlockExpression< RowBlock< A, BCRSMatrix<A>,BlockVector<B> > >
    {
      typedef ExprTmpl::Expression< RowBlock< typename BlockType<A>::type, RowBlock< A, BCRSMatrix<A>,BlockVector<B> >,BlockVector<B> > > type;
    };
#endif

    template <class K, int N, int M, class A, class B>
    struct BlockExpression< RowBlock< FieldMatrix<K,N,M>, A, B > >
    {
      typedef K type;
    };

    template <class K, int N, int M>
    struct BlockExpression< MV< FieldMatrix<K,N,M>, FieldVector<K,M> > >
    {
      typedef K type;
    };

  } // NS ExpreTmpl

  template <class T>
  struct BlockType< BCRSMatrix<T> >
  {
    typedef T type;
  };

  template <class K, int N, int M>
  struct BlockType< FieldMatrix<K,N,M> >
  {
    typedef K type;
  };

  template <>
  struct BlockType< double >
  {
    typedef double type;
  };

  template <class T>
  struct FieldType< BCRSMatrix<T> >
  {
    typedef typename FieldType<T>::type type;
  };

  template <class K, int N, int M>
  struct FieldType< FieldMatrix<K,N,M> >
  {
    typedef K type;
  };

} // NS Dune

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

  Matrix::Iterator rit=A.begin();
  Matrix::Iterator rend=A.end();
  for (; rit!=rend; ++rit)
  {
    Matrix::ColIterator cit=rit->begin();
    Matrix::ColIterator cend=rit->end();
    for (; cit!=cend; ++cit)
    {
      //    *rit = rit.index();
      *cit = 10*cit.index()+rit.index();
    }
  }

  printmatrix (std::cout, A, "Matrix", "r");

  LeftVector v(N);
  LeftVector v2(N);
  v = 0;
  Vector x(M);
  x = 1;
  Dune::FlatIterator<Vector> fit = x.begin();
  Dune::FlatIterator<Vector> fend = x.end();
  c = 0;
  for (; fit!=fend; ++fit)
    *fit=c++;

  std::cout << A.M() << " " << x.N() << " " << v.N() << std::endl;

  A.umv(x,v);

  using namespace Dune;
  printvector (std::cout, x, "Vector X", "r");
  printvector (std::cout, v, "Vector", "r");
  v=1;
  MV<Matrix,Vector> eximp (A,x);
  Dune::ExprTmpl::Expression< MV<Matrix,Vector> > ex ( eximp );
  v = ex;
  printvector (std::cout, v, "Vector", "r");
  v2 = A * x;

  printvector (std::cout, v2, "Vector2", "r");
#if 0
  int rowIndex[]={1};
  FlatColIterator<Matrix,Vector> it(A[2].begin(),rowIndex);
  for (int i=0; i<5; i++)
  {
    std::cout << *it << " ";
    ++it;
  }
  std::cout << std::endl;
#endif
}

int main()
{
  //  Dune::dvverb.attach(std::cout);
  try
  {
    std::ofstream mylog("/dev/null");
    Dune::dvverb.attach(mylog);
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
