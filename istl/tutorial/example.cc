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

#include "basearray.hh"
#include "fvector.hh"
#include "bvector.hh"
#include "vbvector.hh"
#include "fmatrix.hh"
#include "bcrsmatrix.hh"
#include "io.hh"
#include "gsetc.hh"

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

void test_BlockVector ()
{
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
  // block types
  Timer t;
  const int BlockSize=1;
  typedef Dune::FieldVector<double,BlockSize> VB;
  typedef Dune::FieldMatrix<double,BlockSize,BlockSize> MB;

  // a fake discretization
  t.start();
  const int N=1000000, K1=1, K2=1000;
  Dune::BCRSMatrix<MB> A(N,N,5*N,Dune::BCRSMatrix<MB>::row_wise);
  for (Dune::BCRSMatrix<MB>::CreateIterator i=A.createbegin(); i!=A.createend(); ++i)
  {
    i.insert(i.index());
    if (i.index()-K1>=0) i.insert(i.index()-K1);
    if (i.index()-K2>=0) i.insert(i.index()-K2);
    if (i.index()+K2< N) i.insert(i.index()+K2);
    if (i.index()+K1< N) i.insert(i.index()+K1);
  }
  for (Dune::BCRSMatrix<MB>::RowIterator i=A.begin(); i!=A.end(); ++i)
    for (Dune::BCRSMatrix<MB>::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      if (i.index()==j.index())
        (*j) = 4;
      else
        (*j) = -1;
  t.stop();
  std::cout << "time for build=" << t.gettime() << " seconds." << std::endl;
  // printmatrix(std::cout,A,"system matrix","row",10,2);

  // set up system
  Dune::BlockVector<VB> x(N),b(N),d(N);
  b=0; b[0] = 1; b[N-1] = 1;
  // printvector(std::cout,b,"rhs","entry",1,10,2);

  // same in defect formulation
  x = 0;           // initial guess
  std::cout.setf(std::ios_base::scientific, std::ios_base::floatfield);
  std::cout.precision(8);
  t.start();
  d=b; A.mmv(x,d); // compute defect
  std::cout << 0 << " " << d.two_norm() << std::endl;
  Dune::BlockVector<VB> v(x); // memory for update
  v = 1.23E-4;
  double w=1.0;               // damping factor
  for (int k=1; k<=50; k++)
  {
    bgs_update(A,v,d);        // compute update
    x.axpy(w,v);               // update solution
    A.usmv(-w,v,d);            // update defect
    std::cout << k << " " << d.two_norm() << std::endl;
  }
  t.stop();
  std::cout << "time for solve=" << t.gettime() << " seconds." << std::endl;
  return;

  // printvector(std::cout,x,"solution","entry",1,10,2);

  // a simple iteration
  x = 0;           // initial guess
  d=b; A.mmv(x,d);
  std::cout.setf(std::ios_base::scientific, std::ios_base::floatfield);
  std::cout.precision(8);
  std::cout << 0 << " " << d.two_norm() << std::endl;
  for (int k=1; k<=50; k++)
  {
    bsor(A,x,b,1.0);
    d=b; A.mmv(x,d);
    std::cout << k << " " << d.two_norm() << std::endl;
  }
  // printvector(std::cout,x,"solution","entry",1,10,2);
}

int main (int argc , char ** argv)
{
  try {
    //  test_basearray();
    //  test_BlockVector();
    //  test_VariableBlockVector();
    //  test_FieldMatrix();
    //  test_BCRSMatrix();
    //	test_IO();
    test_Iter();
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
