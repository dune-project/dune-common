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

#include "fvector.hh"
#include "bvector.hh"
#include "vbvector.hh"

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

int main (int argc , char ** argv)
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

  typedef Dune::FieldVector<double,1> R1;
  typedef Dune::FieldVector<double,2> R2;
  typedef Dune::FieldVector<double,6> R6;
  typedef Dune::FieldVector<double,10> R10;
  typedef Dune::FieldVector<double,20> R20;
  typedef Dune::FieldVector<double,40> R40;
  typedef Dune::FieldVector<double,50> R50;
  typedef Dune::FieldVector<double,80> R80;

  //const int n=1024*1024*32;
  const int n=480;

  Dune::BlockVector<R1> x(n),y(n);

  x = 0.0;
  y = 5.3435E-6;

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


  try {
    typedef Dune::VariableBlockVector<R1> VBV;

    VBV w(20);

    for (VBV::CreateIterator i=w.createbegin(); i!=w.createend(); ++i)
      i.setblocksize(i.blockindex()+1);

    w = 1.0;
  }
  catch (Dune::ISTLError& error)
  {
    std::cout << error << std::endl;
  }

  return 0;
}
