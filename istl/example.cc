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
  Dune::base_array<Type> c(p+4,3); // c contains p[4]...p[6]

  // copy window
  b = c;

  // move window to p[6]...p[10]
  c.move(2,5);
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
  A.umvt(b,z);
  A.umvh(b,z);
  A.usmv(-1.0,z,b);
  A.usmvt(-1.0,b,z);
  A.usmvh(-1.0,b,z);

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

  BCRSMat C(N,N,N*(2*K+1));

  for (BCRSMat::CreateIterator i=C.createbegin(); i!=C.createend(); ++i)
    for (int j=-K; j<=K; ++j)
      i.insert((i.index()+N+j)%N);

  for (BCRSMat::RowIterator i=C.begin(); i!=C.end(); ++i)
    for (BCRSMat::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
      *j = D;
}

void test_FieldVector ()
{
  Dune::FieldVector<std::complex<double>,7> v;

  v = std::complex<double>(1.0,2.0);

  std::cout << " |v|1=" << v.one_norm()
            << " |v|1real=" << v.one_norm_real()
            << " |v|2=" << v.two_norm()
            << " |v|inf=" << v.infinity_norm()
            << " |v|infreal=" << v.infinity_norm_real()
            << std::endl;

  std::cout << " v*v=" << v*v << std::endl;

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

  // use of iterator
  int k = 0;
  for (Dune::BlockVector<R1>::Iterator i=x.begin(); i!=x.end(); ++i)
    *i = k++;

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
  typedef Dune::FieldVector<double,1> R1;

  try {
    typedef Dune::VariableBlockVector<R1> VBV;

    VBV w(20);

    for (VBV::CreateIterator i=w.createbegin(); i!=w.createend(); ++i)
      i.setblocksize(i.index()+1);

    w = 1.0;
  }
  catch (Dune::ISTLError& error)
  {
    std::cout << error << std::endl;
  }

}

template<int n, int m>
void test_dot ()
{
  //  const int n=8*1024*1024;
  //  const int m=1;

  typedef Dune::FieldVector<double,m> R;

  double sum;
  Timer watch;
  double t;
  int i;

  // blockvector
  Dune::BlockVector<R> x1(n),y1(n);

  x1 = 1.3576E-7;
  y1 = 5.3435E-6;

  sum = 0;
  for (i=1; i<1000000000; i*=2)
  {
    watch.start();
    sum = 0;
    for (int j=0; j<i; ++j)
      sum += x1*y1;
    t = watch.stop();
    if (t>1.0) break;
  }

  std::cout << "    blockvector dot:"
            << " n=" << x1.dim()
            << " i=" << i
            << " t=" << t
            << " mflop=" << 2.0*x1.dim()*((double)i)/t/1E6
            << " sum=" << sum
            << std::endl;

  // variable blockvector with size 1
  Dune::VariableBlockVector<R> x2(n),y2(n);
  for (typename Dune::VariableBlockVector<R>::CreateIterator it=x2.createbegin(); it!=x2.createend(); ++it)
    it.setblocksize(1);
  for (typename Dune::VariableBlockVector<R>::CreateIterator it=y2.createbegin(); it!=y2.createend(); ++it)
    it.setblocksize(1);

  x2 = 1.3576E-7;
  y2 = 5.3435E-6;

  sum = 0;
  for (i=1; i<1000000000; i*=2)
  {
    watch.start();
    sum = 0;
    for (int j=0; j<i; ++j)
      sum += x2*y2;
    t = watch.stop();
    if (t>1.0) break;
  }

  std::cout << "var blockvector dot:"
            << " n=" << x2.dim()
            << " i=" << i
            << " t=" << t
            << " mflop=" << 2.0*x2.dim()*((double)i)/t/1E6
            << " sum=" << sum
            << std::endl;

  x2 = 1.3576E-7;
  y2 = 5.3435E-6;

  typedef typename Dune::VariableBlockVector<R>::Iterator iterator;

  sum = 0;
  for (i=1; i<1000000000; i*=2)
  {
    watch.start();
    sum = 0;
    for (int j=0; j<i; ++j)
    {
      iterator xitend=x2.end(),xit=x2.begin(),yit=y2.begin();
      for (; xit!=xitend; ++xit,++yit)
        sum += (*xit)*(*yit);
    }
    t = watch.stop();
    if (t>1.0) break;
  }

  std::cout << "gen vblockvecto dot:"
            << " n=" << x2.dim()
            << " i=" << i
            << " t=" << t
            << " mflop=" << 2.0*x2.dim()*((double)i)/t/1E6
            << " sum=" << sum
            << std::endl;
}

int main (int argc , char ** argv)
{
  test_basearray();
  //  test_FieldMatrix();
  //test_BCRSMatrix();
  test_BlockVector();
  test_dot<480,1>();
  test_dot<480,10>();
  test_dot<8*1024*1024,1>();

  return 0;
}
