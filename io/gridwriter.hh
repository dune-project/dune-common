// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRIDWRITER_HH__
#define __GRIDWRITER_HH__

#include <string>

namespace Dune {

  class GridWriter {

  public:
    virtual void write(void* grid,
                       const std::string& filename) const = 0;

    static GridWriter* getWriter(const std::string& format);

  protected:

    GridWriter() {}

    static GridWriter* writer;

  };

}

#endif
