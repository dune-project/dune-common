# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

from io import StringIO
from numpy import array

import os
# enable DUNE_SAVE_BUILD to test various output options
os.environ['DUNE_LOG_LEVEL'] = 'info'
os.environ['DUNE_SAVE_BUILD'] = 'console'

classACode="""
struct MyClassA {
  MyClassA(int a,int b) : a_(a), b_(b) {}
  double b() { return b_; }
  int a_,b_;
};
"""
classBCode="""
#include <iostream>
#include <cmath>
#include <numeric>
#include <dune/python/common/numpyvector.hh>
template <class T> struct MyClassB {
  // the following fails to work correctly
  // MyClassB(T &t, int p, const Dune::Python::NumPyVector<double> &b)
  // - with 'const NumPyVector<double> b_;':  call to deleter ctor
  // - with 'const NumPyVector<double> &b_;': segfaults (this is expected
  //        since array_t->NumPyArray in the __init__ function is explicit and
  //        leads to dangling reference)
  // The following has to be used:
  MyClassB(T &t, int p, pybind11::array_t< double >& b)
  : a_(std::pow(t.a_,p)), b_(b) {}
  double b() { return std::accumulate(b_.begin(),b_.end(),0.); }
  int a_;
  const Dune::Python::NumPyVector<double> b_;
};
"""
runCode="""
#include <iostream>
template <class T> int run(T &t) {
  return t.a_ * t.b();
}
"""

runVec="""
#include <dune/python/common/numpyvector.hh>
template <class T>
void run(pybind11::array_t< T >& a)
{
  Dune::Python::NumPyVector< T > x( a );
  for( size_t i=0; i<x.size(); ++i )
    x[ i ] += i;
}
"""

def test_numpyvector():
    """
    Test correct exchange of numpy arrays to C++ side.
    """
    from dune.generator.algorithm import run
    x = array([0.]*100)
    run("run",StringIO(runVec),x)
    for i in range(len(x)):
        assert x[i] == float(i)

def test_class_export():
    from dune.generator.importclass import load
    from dune.generator.algorithm   import run
    from dune.typeregistry import generateTypeName
    a = 2.
    x = array([2.]*10)
    cls = load("MyClassA",StringIO(classACode),10,20)
    assert run("run",StringIO(runCode),cls) == 10*20
    clsName, _ = generateTypeName("MyClassB",cls)
    cls = load(clsName,StringIO(classBCode),cls,2,x)
    assert run("run",StringIO(runCode),cls) == 10**2*10*a
    x[:] = array([3.]*10)[:]
    assert run("run",StringIO(runCode),cls) == 10**2*10*3
    # the following does not work
    x = array([4.]*10)
    # the 'B' class still keeps the old vector 'x' alive
    assert run("run",StringIO(runCode),cls) == 10**2*10*3

if __name__ == "__main__":
    from dune.packagemetadata import getDunePyDir
    _ = getDunePyDir()
    test_class_export()
    test_numpyvector()
