// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../io/gridwriter.hh"
#include "io/amirameshwriter.hh"
#include <string>
#include <stdio.h>

Dune::GridWriter* Dune::GridWriter::getWriter(const std::string& format)
{
  printf("Chosen format: %s\n", format.c_str());

  GridWriter* gridWriter = NULL;

  if (format.compare("amiramesh")==0) ;
  gridWriter = new AmiraMeshWriter;

  return gridWriter;
}
