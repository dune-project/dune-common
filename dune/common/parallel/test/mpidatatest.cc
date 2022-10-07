// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <config.h>

#include <iostream>

#include <dune/common/dynmatrix.hh>
#include <dune/common/parallel/mpidata.hh>

#include <dune/common/parallel/mpihelper.hh>
using namespace Dune;

int main(int argc, char** argv){
  Dune::MPIHelper & mpihelper = Dune::MPIHelper::instance(argc, argv);
  auto cc = mpihelper.getCommunication();

  if(mpihelper.rank() == 0)
    std::cout << "Test 1: static data (int)" << std::endl;
  if(mpihelper.rank() == 0){
    cc.send<int>(42, 1, 0);
    int i = 42;
    const int& j = i;
    cc.send(j, 1, 0);
  }
  else if(mpihelper.rank() == 1){
    std::cout << "receive: " << cc.recv<int>(0, 0, 0) << std::endl;
    int i = 0;
    cc.recv(i, 0, 0);
    std::cout << i << std::endl;
  }

  if(mpihelper.rank() == 0)
    std::cout << "Test 2: dynamic data (std::vector<double>)" << std::endl;
  if(mpihelper.rank() == 0){
    cc.send(std::vector<double>{ 42.0, 43.0, 4711}, 1, 0);
    std::vector<double> vec{ 42.0, 43.0, 4711};
    const std::vector<double>& vec_ref = vec;
    cc.send(vec_ref, 1, 0);
    cc.send(std::move(vec), 1, 0);
  }
  else if(mpihelper.rank() == 1){
    auto vec = cc.recv(std::vector<double>{0,0,0}, 0, 0);
    std::cout << "receive: ";
    for(double d : vec)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    std::vector<double> vec2(3);
    cc.recv(vec2, 0, 0);
    for(double d : vec2)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;

    std::vector<double> vec3(3);
    auto d = vec3.data();
    std::vector<double> vec4 = cc.recv(std::move(vec3), 0, 0);
    for(double d : vec4)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    if(d != vec4.data())
      DUNE_THROW(Exception, "The vector has not the same memory");
  }

  if(mpihelper.rank() == 0)
    std::cout << "Test 3: DynamicVector" << std::endl;
  if(mpihelper.rank() == 0){
    cc.send(DynamicVector<double>{ 42.0, 43.0, 4711}, 1, 0);
    DynamicVector<double> vec{ 42.0, 43.0, 4711};
    const DynamicVector<double>& vec_ref = vec;
    cc.send(vec_ref, 1, 0);
    cc.send(std::move(vec), 1, 0);
  }
  else if(mpihelper.rank() == 1){
    auto vec = cc.recv(DynamicVector<double>{0,0,0}, 0, 0);
    std::cout << "receive: ";
    for(double d : vec)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    DynamicVector<double> vec2(3);
    cc.recv(vec2, 0, 0);
    for(double d : vec2)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;

    DynamicVector<double> vec3(3);
    auto d = vec3.container().data();
    DynamicVector<double> vec4 = cc.recv(std::move(vec3), 0, 0);
    for(double d : vec4)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    if(d != vec4.container().data())
      DUNE_THROW(Exception, "The vector has not the same memory");
  }


  if(mpihelper.rank() == 0)
    std::cout << "Test 3: DynamicVector (resize receive)" << std::endl;
  if(mpihelper.rank() == 0){
    cc.send(DynamicVector<double>{ 42.0, 43.0, 4711}, 1, 0);
    DynamicVector<double> vec{ 42.0, 43.0, 4711};
    const DynamicVector<double>& vec_ref = vec;
    cc.send(vec_ref, 1, 0);
    cc.send(std::move(vec), 1, 0);
  }
  else if(mpihelper.rank() == 1){
    auto vec = cc.rrecv(DynamicVector<double>{}, 0, 0);
    std::cout << "receive: ";
    for(double d : vec)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    DynamicVector<double> vec2(3);
    cc.recv(vec2, 0, 0);
    for(double d : vec2)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;

    DynamicVector<double> vec3(3);
    auto d = vec3.container().data();
    DynamicVector<double> vec4 = cc.recv(std::move(vec3), 0, 0);
    for(double d : vec4)
      std::cout << d << ",";
    std::cout << "\b" << std::endl;
    if(d != vec4.container().data())
      DUNE_THROW(Exception, "The vector has not the same memory");
  }

  return 0;
}
