// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string>
#include <vector>

#include <dune/common/configparser.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>

int main ()
{
  try {
    Dune::ConfigParser parameterSet;

    parameterSet["testDouble"] = "3.14";
    parameterSet["testInt"]    = "42";
    parameterSet["testString"] = "Hallo Welt!";
    parameterSet["testVector"] = "2 3 5 7 11";

    double testDouble      = parameterSet.get<double>("testDouble");
    int testInt            = parameterSet.get<int>("testInt");
    ++testDouble;
    ++testInt;
    std::string testString = parameterSet.get<std::string>("testString");
    typedef Dune::FieldVector<unsigned, 5> FVector;
    FVector testFVector    = parameterSet.get<FVector>("testVector");
    typedef std::vector<unsigned> SVector;
    SVector testSVector    = parameterSet.get<SVector>("testVector");
    if(testSVector.size() != 5)
      DUNE_THROW(Dune::Exception, "Testing std::vector<unsigned>: expected "
                 "size()==5, got size()==" << testSVector.size());
    for(unsigned i = 0; i < 5; ++i)
      if(testFVector[i] != testSVector[i])
        DUNE_THROW(Dune::Exception,
                   "testFVector[" << i << "]==" << testFVector[i] << " but "
                   "testSVector[" << i << "]==" << testSVector[i]);
  }
  catch(const Dune::Exception& e) {
    std::cerr << "Exception thrown: " << e << std::endl;
    throw;
  }
}
