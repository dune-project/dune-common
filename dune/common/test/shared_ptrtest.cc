// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

// make sure assert works even when not compiling for debugging
#ifdef NDEBUG
#undef NDEBUG
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/classname.hh>
#include <dune/common/shared_ptr.hh>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <vector>

template<class T>
class Deleter
{
public:
  Deleter(bool& deleted) :
    deleted_(deleted)
  {}

  void operator() (T* p) const
  {
    delete p;
    deleted_ = true;
  }

private:
  bool& deleted_;
};

class A {};
class B : public A {};
class C : A {};


Dune::shared_ptr<A> test_make_shared()
{
  return Dune::make_shared<B>();
}


int main(){
  using namespace Dune;
  int ret=0;
  {
    // test default constructor
    shared_ptr<int> foo;

    // test conversion in make_shared
    shared_ptr<A> a=test_make_shared();

    {
      shared_ptr<B> b(new B);
      a=b;

      if(b.use_count()!=2) {
        std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }

      if(a.use_count()!=2) {
        std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }

    }
    // print the type of the shared_ptr, so we know whether we are
    // checking dune's shared_ptr or some std one
    std::cout << "=== checking shared_ptr type: " << className(foo)
              << std::endl;

    // make sure default-constructed shared_ptr values can be copied
    {
      shared_ptr<int> bar(foo);
      shared_ptr<int> baz;
      baz = foo;
    }

    // test cast-to-bool
    if (foo) {
      std::cout << "Default constructor doesn't create a NULL pointer!" << std::endl;
      ret=1;
    }

    // test custom deleter
    bool deleted = false;
    {
      shared_ptr<int> bar(new int(1), Deleter<int>(deleted));

      //test if deleter is called
      deleted = false;
      bar.reset(new int(2));        // this should call the deleter in the constructor
      if (not (deleted))
      {
        std::cout << "Custom deleter not called!" << std::endl;
        ret=1;
      }

      //test if old deleter is not called
      deleted = false;
      bar.reset();        // this should call no deleter
      if (deleted)
      {
        std::cout << "Old deleter was called!" << std::endl;
        ret=1;
      }

      //test if old deleter is not called
      deleted = false;
      bar.reset(new int(3), Deleter<int>(deleted));        // this should call no deleter
      if (deleted)
      {
        std::cout << "Old deleter was called!" << std::endl;
        ret=1;
      }
      // going out of scope should call the deleter
    }
    if (not (deleted))
    {
      std::cout << "Custom deleter not called!" << std::endl;
      ret=1;
    }
    {
      shared_ptr<int> bar(new int(1), Deleter<int>(deleted));

      bar.reset(new int(4));        // this should call the deleter...

      deleted = false;
      // ... but going out of scope should call no deleter
    }
    if (deleted)
    {
      std::cout << "1Old deleter was called!" << std::endl;
      ret=1;
    }

    // test constructor from a given pointer
    shared_ptr<double> bar(new double(43.0));
    assert(bar);

    // test constructor from nullptr
#ifndef HAVE_NULLPTR
        #warning Construction of shared_ptr from a nullptr will not work as compiler
        #warning does not support the latter.
    shared_ptr<double> bar_null=shared_ptr<double>();
#else
    shared_ptr<double> bar_null(nullptr);
#endif
    assert(!bar_null);
    assert(!bar_null.get());

    // test reset()
    bar.reset();
    assert(!bar);

    // test get() for empty shared_ptr
    assert(!bar.get());

    // test reset(T*)
    double* p = new double(44.0);
    bar.reset(p);
    assert(bar);
    assert(bar.use_count()==1);

    // test get()
    double* barPtr = bar.get();
    assert(barPtr==p);

    // test constructor from a given pointer
    shared_ptr<double> b(new double(42.0));
    {
      shared_ptr<double> d(b);
      *b = 7;
    }

    if(b.use_count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }
    {
      shared_ptr<double> c(b);

      if(*b!=*c) {
        std::cerr<<"References do not match! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
      if(b.use_count()!=2 || c.use_count()!=2) {
        std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
      *b=8;
      if(*b!=8 || *c!=8) {
        std::cout<<"Assigning new value failed! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
    }

    if(b.use_count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }

    // test assignment from null ptr
    // (should trigger FS 936, needs valgrind to check)
    {
      shared_ptr<int> foobar = shared_ptr<int>(new int(42));
      shared_ptr<int> foobaz;        //null ptr

      foobar = foobaz;        // should release memory held by foo
    }

    // test shared_ptr for stack allocation
    {
      int i = 10;
      shared_ptr<int> pi = stackobject_to_shared_ptr(i);
    }

    // test shared_ptr for stack allocation with down cast
    {
      B b2;
      shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b2);
#ifdef SHARED_PTR_COMPILE_FAIL
      C c;
      pa = stackobject_to_shared_ptr<A>(c);       // A is an inaccessible base of C
#endif
    }
  }
  return (ret);
}
