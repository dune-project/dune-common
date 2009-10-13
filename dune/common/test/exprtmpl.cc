// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/*
   TODO:
   - test deeper Matrix nesting
   - get rid of
    int *M;
   - fix RowBlock::N()
   - remove second template parameter of FlatColIterator
   - vectorentry -> exrpressionentry
   - FlatColIterator<Matrix> does not work if Matrix is mutable
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <fstream>
#include <dune/common/fvector.hh>
#include <dune/common/timer.hh>
//#include <dune/istl/bvector.hh>
//#include <dune/istl/io.hh>
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
  const int bs = 2;
  const int sz = 3;
  typedef Dune::FieldVector<double,bs> VB;
  typedef Dune::BlockVector<VB> BV;
  typedef Dune::BlockVector<BV> BBV;
  typedef Dune::ExprTmpl::ConstRef<BV> RBV;
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

  Dune::Timer stopwatch;
  stopwatch.reset();
  for (int i=0; i<10; i++) bbv *= 2;
  std::cout << "Time bbv*2: " << stopwatch.elapsed() << std::endl;
#ifndef NOPRINT
  //   Dune::FlatIterator<BBV> fit(bbv.begin());
  //   Dune::FlatIterator<BBV> fend(bbv.end());
  //   int index = 0;
  //   for(;fit!=fend;++fit)
  //   {
  //     BBV::field_type x;
  //     x = *fit;
  //     std::cout << index << "\t" << x << std::endl;
  //     index++;
  //   }
  printvector (std::cout, bv1, "bv1", "r");
  printvector (std::cout, bv2, "bv1", "r");
  printvector (std::cout, bbv, "bbv", "r");
#endif
  std::cout << "infinity_norm(bbv)=" << infinity_norm(bbv) << std::endl;
  std::cout << "two_norm(bbv)=" << two_norm(bbv) << std::endl;
  std::cout << "bbv.two_norm()=" << bbv.two_norm() << std::endl;
  std::cout << "two_norm2(bbv)=" << two_norm2(bbv) << std::endl;
  std::cout << "one_norm(bbv)=" << one_norm(bbv) << std::endl;
}

// namespace Dune {

// namespace ExprTmpl {

// template <class K, int iN, int iM>
// class MatrixMulVector< FieldMatrix<K,iN,iM>,
//                        BCRSMatrix< FieldMatrix<K,iN,iM> >,
//                        BlockVector< FieldVector<K,iM> > >
// {
// public:
//   typedef BCRSMatrix< FieldMatrix<K,iN,iM> > Mat;
//   typedef BlockVector< FieldVector<K,iM> > Vec;
//   typedef typename
//     BlockTypeN<MatrixMulVector<Mat,Mat,Vec>,
//                MyDepth<FieldMatrix<K,iN,iM>,Mat>::value-1>::type
//     ParentBlockType;
//   /* constructor */
//   MatrixMulVector(const Mat & _A, const Vec & _v, int* _M,
//                   const ParentBlockType & _parent) :
//     parent(_parent), M(_M), A(_A), v(_v )
//     {
//       int parent_i = M[0];
//       typename Mat::ConstColIterator it = A[parent_i].begin();
//       typename Mat::ConstColIterator end = A[parent_i].end();
//       tmp = 0;
//       for (; it!=end; ++it)
//       {
//         it->umv(tmp,v[it.index()]);
//       }
//     };
//   K operator[] (int i) const {
//     return tmp[i];
//   }
//   int N() const { iN; };
//   const ParentBlockType & parent;
// private:
//   FieldVector<K,iN> tmp;
//   mutable int* M;
//   const Mat & A;
//   const Vec & v;
// };

// } // NS ExpreTmpl

// } // NS Dune

//template<int BlockSize, int N, int M>
template<int BN, int BM, int N, int M>
void test_matrix()
{
  std::cout << "test_matrix<" << BN << ", " << BM << ", "
            << N << ", " << M << ">\n";

  typedef double matvec_t;
  typedef Dune::FieldVector<matvec_t,BN> LVB;
  typedef Dune::FieldVector<matvec_t,BM> VB;
  typedef Dune::FieldMatrix<matvec_t,BN,BM> MB;
  typedef Dune::BlockVector<LVB> LeftVector;
  typedef Dune::BlockVector<VB> Vector;
  typedef Dune::BCRSMatrix<MB> Matrix;

  LVB a(0);
  VB b(2);
  MB _M(1);
  _M[1][1] = 3;

  // a += M * b
  _M.umv(b,a);

#ifndef NOPRINT
  printmatrix (std::cout, _M, "Matrix", "r");
  printvector (std::cout, a, "Vector", "r");
#endif

  // a = M * b
#if 0
  a = _M*b;
#endif

#ifndef NOPRINT
  printvector (std::cout, a, "Vector", "r");
#endif

  Matrix A(N,M,Matrix::row_wise);
  typename Matrix::CreateIterator i=A.createbegin();
  typename Matrix::CreateIterator end=A.createend();
  std::cout << "Building matrix structure\n";
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
  std::cout << "...done\n";

#ifndef NOPRINT
  std::cout << "Matrix coldim=" << A.coldim() << std::endl;
  std::cout << "Matrix rowdim=" << A.rowdim() << std::endl;
  std::cout << "Matrix N=" << A.M() << std::endl;
  std::cout << "Matrix M=" << A.N() << std::endl;

  std::cout << "Assembling matrix\n";
  typename Matrix::Iterator rit=A.begin();
  typename Matrix::Iterator rend=A.end();
  for (; rit!=rend; ++rit)
  {
    typename Matrix::ColIterator cit=rit->begin();
    typename Matrix::ColIterator cend=rit->end();
    for (; cit!=cend; ++cit)
    {
      //    *rit = rit.index();
      *cit = 10*cit.index()+rit.index();
    }
  }
  std::cout << "...done\n";

  printmatrix (std::cout, A, "Matrix", "r");
#endif

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

  Dune::Timer stopwatch;
  stopwatch.reset();
  A.umv(x,v);
  std::cout << "Time umv: " << stopwatch.elapsed() << std::endl;

  using namespace Dune;
#ifndef NOPRINT
  printvector (std::cout, x, "Vector X", "r");
  printvector (std::cout, v, "Vector", "r");
#endif

  v2 = 0;
  stopwatch.reset();
  v2 += A * x;
  std::cout << "Time v2+=A*x: " << stopwatch.elapsed() << std::endl;
#ifndef NOPRINT
  printvector (std::cout, v2, "Vector2", "r");
#endif

#ifndef NOPRINT
  //   int rowIndex[]={1};
  //   FlatColIterator<const Matrix> it(A[2].begin(),rowIndex);
  //   for (int i=0; i<5; i++)
  //   {
  //     std::cout << *it << " ";
  //     ++it;
  //   }
  //   std::cout << std::endl;
#endif
  std::cout << std::endl;
}

void test_norm()
{
  Dune::FieldVector<double,3> a,b;
  double c;
  c = (a-b).two_norm();
  c = two_norm(a-b);
}

void test_sproduct()
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

  double x;
  x = bv1[0] * bv2[0];
  x = bv1 * bv2;
}

int main()
{
  //  Dune::dvverb.attach(std::cout);
  try
  {
    //      test_fvector();
    //      test_blockvector();
    test_norm();
    test_sproduct();
    test_blockblockvector();
    test_matrix<2,3,3,4>();
#ifdef NOPRINT
    test_matrix<3,6,400000,500000>();
    test_matrix<6,3,400000,500000>();
    test_matrix<30,60,4000,5000>();
    test_matrix<150,150,500,4000>();
    test_matrix<150,150,1000,2000>();
#endif
    //      test_matrix<150,150,2000,1000>(); // fails in fmeta_something
    //      test_matrix<150,150,4000,500>(); // fails in fmeta_something
  }
  catch (Dune::Exception & e)
  {
    std::cout << e << std::endl;
  }
  exit(0);
}
