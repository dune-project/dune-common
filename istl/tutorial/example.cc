// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// start with including some headers
#include <iostream>               // for input/output to shell
#include <fstream>                // for input/output to files
#include <vector>                 // STL vector class
#include <complex>

#include <math.h>                 // Yes, we do some math here
#include <stdio.h>                // There is nothing better than sprintf
#include <sys/times.h>            // for timing measurements

#include "istlexception.hh"
#include "basearray.hh"
#include "dune/common/fvector.hh"
#include "bvector.hh"
#include "vbvector.hh"
#include "fmatrix.hh"
#include "bcrsmatrix.hh"
#include "io.hh"
#include "gsetc.hh"
#include "ilu.hh"
#include "operators.hh"
#include "solvers.hh"
#include "preconditioners.hh"

// a simple stop watch
class Timer
{
public:
  Timer ()
  {
    struct tms buf;
    cstart = times(&buf);
  }

  void start ()
  {
    struct tms buf;
    cstart = times(&buf);
  }

  double stop ()
  {
    struct tms buf;
    cend = times(&buf);
    return ((double)(cend-cstart))/100.0;
  }

  double gettime ()
  {
    return ((double)(cend-cstart))/100.0;
  }

private:
  clock_t cstart,cend;
};


// testing codes
void test_basearray ()
{
  // what you can do with base_array

  // allocation
  typedef double Type; // any type
  Dune::base_array<Type> a(20);

  // modifying iterator
  for (Dune::base_array<Type>::iterator i=a.begin(); i!=a.end(); ++i)
    *i = 1.0;

  // read only iterator
  Type sum=0;
  for (Dune::base_array<Type>::const_iterator i=a.begin(); i!=a.end(); ++i)
    sum += *i;

  // random access
  a[4] = 17;
  sum = a[3];

  // empty array
  Dune::base_array<Type> b;

  // assignment
  b = a;

  // window mode
  Type p[13];
  Dune::base_array_window<Type> c(p+4,3); // c contains p[4]...p[6]

  // copy window
  b = c;
  Dune::base_array<Type> d(c);

  // move window to p[6]...p[10]
  c.move(2,5);
}

template<class V>
void f (V& v)
{
  typedef typename V::Iterator iterator;
  for (iterator i=v.begin(); i!=v.end(); ++i)
    *i = i.index();

  typedef typename V::ConstIterator const_iterator;
  for (const_iterator i=v.begin(); i!=v.end(); ++i)
    std::cout << (*i).two_norm() << std::endl;
}

void test_BlockVector ()
{
  Dune::BlockVector<Dune::FieldVector<std::complex<double>,2> > v(20);

  v[1] = 3.14;
  v[3][0] = 2.56;
  v[3][1] = std::complex<double>(1,-1);

  f(v);

  typedef Dune::FieldVector<double,1> R1;

  const int n=480;

  // make two vectors of size n
  Dune::BlockVector<R1> x(n),y(n);

  // assign from scalar
  x = 1.0;
  y = 5.3435E-6;

  // use of Iterator
  int k = 0;
  for (Dune::BlockVector<R1>::Iterator i=x.begin(); i!=x.end(); ++i)
    *i = k++;

  // and ConstIterator
  R1 z;
  for (Dune::BlockVector<R1>::ConstIterator i=x.begin(); i!=x.end(); ++i)
    z += *i;

  // assignment
  y = x;

  // random access
  x[2] = y[7];

  // timing the axpy operation
  Timer watch;
  double t;
  int i;

  for (i=1; i<1000000000; i*=2)
  {
    watch.start();
    for (int j=0; j<i; ++j)
      x.axpy(1.001,y);
    t = watch.stop();
    if (t>1.0) break;
  }

  std::cout << "axpy:"
            << " n=" << x.dim()
            << " i=" << i
            << " t=" << t
            << " mflop=" << 2.0*x.dim()*((double)i)/t/1E6
            << std::endl;

  // timing the dot operation
  x = 1.234E-3;
  y = 4.938E-1;

  double sum = 0;
  for (i=1; i<1000000000; i*=2)
  {
    watch.start();
    sum = 0;
    for (int j=0; j<i; ++j)
      sum += x*y;
    t = watch.stop();
    if (t>1.0) break;
  }

  std::cout << " dot:"
            << " n=" << x.dim()
            << " i=" << i
            << " t=" << t
            << " mflop=" << 2.0*x.dim()*((double)i)/t/1E6
            << " sum=" << sum
            << std::endl;
}


void test_VariableBlockVector ()
{
  const int N=1;
  typedef Dune::FieldVector<double,N> RN;

  typedef Dune::VariableBlockVector<RN> Vector;

  Vector x(20);

  for (Vector::CreateIterator i=x.createbegin(); i!=x.createend(); ++i)
    i.setblocksize((i.index()%10)+1);

  x = 1.0;

  Vector::block_type xi;

  xi = x[13];

  RN b;

  b = x[13][1];
}

void test_FieldMatrix ()
{
  const int r=4, c=5;
  typedef Dune::FieldMatrix<double,r,c> Mrc;
  typedef Dune::FieldVector<double,r> Rr;
  typedef Dune::FieldVector<double,c> Rc;

  Mrc A,B;

  A[1][3] = 4.33;

  Rr b;
  Rc z;

  for (Mrc::RowIterator i=A.begin(); i!=A.end(); ++i)
    for (Mrc::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      *j = i.index()*j.index();

  for (Mrc::RowIterator i=A.begin(); i!=A.end(); ++i)
    for (Mrc::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      b[i.index()] = *j * z[j.index()];

  A = 1;
  B = 2;

  A += B;
  A -= B;
  A *= 3.14;
  A /= 3.14;

  A.umv(z,b);
  A.umtv(b,z);
  A.umhv(b,z);
  A.usmv(-1.0,z,b);
  A.usmtv(-1.0,b,z);
  A.usmhv(-1.0,b,z);

  std::cout << A.frobenius_norm() << " " << A.frobenius_norm2() << std::endl;
  std::cout << A.infinity_norm() << " " << A.infinity_norm_real() << std::endl;
}

void test_BCRSMatrix ()
{
  const int N=13,K=2;
  typedef Dune::FieldMatrix<double,2,2> LittleBlock;
  typedef Dune::BCRSMatrix<LittleBlock> BCRSMat;

  LittleBlock D;
  D = 2.56;

  BCRSMat C(N,N,N*(2*K+1),BCRSMat::row_wise);

  for (BCRSMat::CreateIterator i=C.createbegin(); i!=C.createend(); ++i)
    for (int j=-K; j<=K; ++j)
      i.insert((i.index()+N+j)%N);

  for (BCRSMat::RowIterator i=C.begin(); i!=C.end(); ++i)
    for (BCRSMat::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      *j = D;
}

void test_IO ()
{
  typedef Dune::FieldVector<double,2> R;
  Dune::BlockVector<R> x(84);

  for (Dune::BlockVector<R>::Iterator i=x.begin(); i!=x.end(); ++i)
    *i = 0.01*i.index();

  printvector(std::cout,x,"a test","entry",11,9,1);

  Dune::VariableBlockVector<R> y(20);

  for (Dune::VariableBlockVector<R>::CreateIterator i=y.createbegin(); i!=y.createend(); ++i)
    i.setblocksize((i.index()%3)+1);

  for (Dune::VariableBlockVector<R>::Iterator i=y.begin(); i!=y.end(); ++i)
    *i = (i.index()%3)+1;

  printvector(std::cout,y,"a test","entry",11,9,1);

  typedef Dune::FieldMatrix<double,2,2> M;
  M A;
  A = 3.14;
  printmatrix(std::cout,A,"a fixed size block matrix","row",9,1);

  const int N=9,K=2;
  Dune::BCRSMatrix<M> C(N,N,N*(2*K+1),Dune::BCRSMatrix<M>::row_wise);

  for (Dune::BCRSMatrix<M>::CreateIterator i=C.createbegin(); i!=C.createend(); ++i)
    for (int j=-K; j<=K; ++j)
      i.insert((i.index()+N+j)%N);

  for (Dune::BCRSMatrix<M>::RowIterator i=C.begin(); i!=C.end(); ++i)
    for (Dune::BCRSMatrix<M>::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      *j = A;

  Dune::BCRSMatrix<M> B(4,4,Dune::BCRSMatrix<M>::random);

  B.setrowsize(0,1);
  B.setrowsize(3,4);
  B.setrowsize(2,2);
  B.setrowsize(1,1);

  B.endrowsizes();

  B.addindex(0,0);
  B.addindex(3,1);
  B.addindex(2,2);
  B.addindex(1,1);
  B.addindex(2,0);
  B.addindex(3,2);
  B.addindex(3,0);
  B.addindex(3,3);

  B.endindices();

  B[0][0] = 1;
  B[1][1] = 2;
  B[2][0] = 3;
  B[2][2] = 4;
  B[3][1] = 5;
  B[3][2] = 6;
  B[3][0] = 7;
  B[3][3] = 8;

  printmatrix(std::cout,B,"a block compressed sparse matrix","row",9,1);
}

void test_Iter ()
{
  Timer t;

  // block types
  const int BlockSize = 6;
  typedef Dune::FieldVector<double,BlockSize> VB;
  typedef Dune::FieldMatrix<double,BlockSize,BlockSize> MB;

  // a fake discretization
  t.start();

  // build little blocks
  MB D=0;
  for (int i=0; i<BlockSize; i++)
    for (int j=0; j<BlockSize; j++)
      if (i==j) D[i][j] = 4+(BlockSize-1);else D[i][j] = -1;
  //   printmatrix(std::cout,D,"diagonal block","row",10,2);

  MB E=0;
  for (int i=0; i<BlockSize; i++)
    E[i][i] = -1;
  //   printmatrix(std::cout,E,"offdiagonal block","row",10,2);

  // make a block compressed row matrix with five point stencil
  const int N=10000, BW1=1, BW2=100;
  Dune::BCRSMatrix<MB> A(N,N,5*N,Dune::BCRSMatrix<MB>::row_wise);
  for (Dune::BCRSMatrix<MB>::CreateIterator i=A.createbegin(); i!=A.createend(); ++i)
  {
    i.insert(i.index());
    if (i.index()-BW1>=0) i.insert(i.index()-BW1);
    if (i.index()+BW1< N) i.insert(i.index()+BW1);
    if (i.index()-BW2>=0) i.insert(i.index()-BW2);
    if (i.index()+BW2< N) i.insert(i.index()+BW2);
  }
  for (Dune::BCRSMatrix<MB>::RowIterator i=A.begin(); i!=A.end(); ++i)
    for (Dune::BCRSMatrix<MB>::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      if (i.index()==j.index())
        (*j) = D;
      else
        (*j) = E;
  t.stop();
  std::cout << "time for build=" << t.gettime() << " seconds." << std::endl;
  //   printmatrix(std::cout,A,"system matrix","row",8,1);

  // set up system
  Dune::BlockVector<VB> x(N),b(N),d(N);
  x=0; x[0]=1; x[N-1]=2;
  //   printvector(std::cout,x,"exact solution","entry",10,10,2);
  b=0; A.umv(x,b); // set right hand side
  x=0;             // initial guess

  // solve in defect formulation
  std::cout.setf(std::ios_base::scientific, std::ios_base::floatfield);
  std::cout.precision(8);
  t.start();
  d=b; A.mmv(x,d); // compute defect
  std::cout << 0 << " " << d.two_norm() << std::endl;
  Dune::BlockVector<VB> v(x); // memory for update
  double w=1.0;               // damping factor
  //  printmatrix(std::cout,A,"system matrix","row",12,4);
  Dune::BCRSMatrix<MB> ILU(A);
  bilu0_decomposition(ILU);
  //  printmatrix(std::cout,ILU,"ilu decomposition","row",12,4);
  for (int k=1; k<=20; k++)
  {
    v=0;
    bilu_backsolve(ILU,v,d);
    //    dbgs(A,v,d,w);       // compute update
    //    dbjac(A,v,d,w);       // compute update
    //            bsorf(A,v,d,w);    //   compute update
    //            bsorb(A,v,d,w);    //   compute update
    x += v;                    // update solution
    A.mmv(v,d);                // update defect
    //    bltsolve(A,v,d,w);   // compute update
    //    x += v;              // update solution
    //    A.mmv(v,d);          // update defect
    //    butsolve(A,v,d,w);   // compute update
    //    x += v;              // update solution
    //    A.mmv(v,d);          // update defect
    std::cout << k << " " << d.two_norm() << std::endl;
    if (d.two_norm()<1E-4) break;
  }
  t.stop();
  std::cout << "time for solve=" << t.gettime() << " seconds." << std::endl;
}


void test_Interface ()
{
  // define Types
  const int BlockSize = 1;
  typedef Dune::FieldVector<double,BlockSize> VB;
  typedef Dune::FieldMatrix<double,BlockSize,BlockSize> MB;
  typedef Dune::BlockVector<VB> Vector;
  typedef Dune::BCRSMatrix<MB> Matrix;

  // build little blocks
  MB D=0;
  for (int i=0; i<BlockSize; i++)
    for (int j=0; j<BlockSize; j++)
      if (i==j) D[i][j] = 4+(BlockSize-1);else D[i][j] = -1;

  MB E=0;
  for (int i=0; i<BlockSize; i++)
    E[i][i] = -1;

  // make a block compressed row matrix with five point stencil
  const int BW2=64, BW1=1, N=BW2*BW2;
  Matrix A(N,N,5*N,Dune::BCRSMatrix<MB>::row_wise);
  for (Matrix::CreateIterator i=A.createbegin(); i!=A.createend(); ++i)
  {
    i.insert(i.index());
    if (i.index()-BW1>=0) i.insert(i.index()-BW1);
    if (i.index()+BW1< N) i.insert(i.index()+BW1);
    if (i.index()-BW2>=0) i.insert(i.index()-BW2);
    if (i.index()+BW2< N) i.insert(i.index()+BW2);
  }
  for (Matrix::RowIterator i=A.begin(); i!=A.end(); ++i)
    for (Matrix::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      if (i.index()==j.index())
        (*j) = D;
      else
        (*j) = E;

  // set up system
  Vector x(N),b(N);
  x=0; x[0]=1; x[N-1]=2; // prescribe known solution
  b=0; A.umv(x,b);       // set right hand side accordingly
  x=0;                   // initial guess

  // set up the high-level solver objects
  Dune::MatrixAdapter<Matrix,Vector,Vector> op(A);        // make linear operator from A
  Dune::SeqSSOR<Matrix,Vector,Vector> ssor(A,1,1.78);     // SSOR preconditioner
  Dune::SeqILU0<Matrix,Vector,Vector> ilu0(A);            // preconditioner object
  Dune::CGSolver<Vector> cg(op,ilu0,1E-8,8000,2);         // an inverse operator
  Dune::BiCGSTABSolver<Vector> bcgs(op,ilu0,1E-8,8000,2); // an inverse operator

  // call the solver
  Dune::InverseOperatorResult r;
  cg.apply(x,b,r);
}



int main (int argc , char ** argv)
{
  try {
    //  test_basearray();
    //  test_BlockVector();
    //  test_VariableBlockVector();
    //  test_FieldMatrix();
    //  test_BCRSMatrix();
    //  test_IO();
    //	test_Iter();
    test_Interface();
  }
  catch (Dune::ISTLError& error)
  {
    std::cout << error << std::endl;
  }
  catch (Dune::Exception& error)
  {
    std::cout << error << std::endl;
  }
  catch (const std::bad_alloc& e)
  {
    std::cout << "memory exhausted" << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown exception caught" << std::endl;
  }

  return 0;
}
