// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __AMIRAMESHWRITER_HH__
#define __AMIRAMESHWRITER_HH__

//#include <string>
#include "../../io/gridwriter.hh"

namespace Dune {

  class AmiraMeshWriter : public GridWriter {

  public:

    virtual void write(void* grid,
                       const std::string& filename) const;

    AmiraMeshWriter() {}

  };

}

#endif
