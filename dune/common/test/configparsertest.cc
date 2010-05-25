// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <string>

#include <dune/common/configparser.hh>

int main ()
{
  Dune::ConfigParser parameterSet;

  double testDouble      = parameterSet.get<double>("testDouble");
  int testInt            = parameterSet.get<int>("testInt");
  std::string testString = parameterSet.get<std::string>("testString");

}
