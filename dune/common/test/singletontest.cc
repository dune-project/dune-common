// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/singleton.hh>
#include <iostream>
class Foo : public Dune::Singleton<Foo>
{
public:
  Foo()
  {
    bytes = new char[1000];
  }

  ~Foo()
  {
    delete[] bytes;
  }
private:
  char* bytes;
};

class Foo1
{
public:
  Foo1()
  {
    bytes = new char[1000];
  }

  ~Foo1()
  {
    delete[] bytes;
  }
private:
  char* bytes;
};

typedef Dune::Singleton<Foo1> FooSingleton;


Foo* globalFoo = 0;
Foo1* globalFoo1 = 0;

void setFoo()
{
  globalFoo = &Foo::instance();
}


void setFoo1()
{
  globalFoo1 = &FooSingleton::instance();
}

int testFoo()
{
  if(globalFoo != &Foo::instance()) {
    std::cerr<<" Foo is not a real singleton!"<<std::endl;
    return 1;
  }
  return 0;
}


int testFoo1()
{
  if(globalFoo1 != &FooSingleton::instance()) {
    std::cerr<<" Foo is not a real singleton!"<<std::endl;
    return 1;
  }
  return 0;
}

int main()
{
  int ret=0;
  {
    Foo& foo = Foo::instance();
    Foo& foo1 = Foo::instance();
    if(&foo!=&foo1) {
      std::cerr<<" Foo is not a real singleton!"<<std::endl;
      ++ret;
    }
  }
  setFoo();
  ret += testFoo();
  {
    Foo1& foo = FooSingleton::instance();
    Foo1& foo1 = FooSingleton::instance();
    if(&foo!=&foo1) {
      std::cerr<<" Foo is not a real singleton!"<<std::endl;
      ++ret;
    }
  }
  setFoo1();
  return ret += testFoo1();
}
