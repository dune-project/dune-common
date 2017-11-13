// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <config.h>

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <ios>
#include <iostream>
#include <utility>

#include <dune/common/debugalign.hh>

namespace Dune {

  //! default alignment violation handler
  /**
   * Prints it's arguments on `stderr` and aborts.
   */
  static void defaultViolatedAlignment(const char *className,
                                       std::size_t expectedAlignment,
                                       const void *address)
  {
    std::cerr << "Error: Detected invalid alignment for type " << className
              << ": Address " << address << " not aligned to 0x" << std::hex
              << expectedAlignment << std::endl;
    std::abort();
  }

  ViolatedAlignmentHandler &violatedAlignmentHandler()
  {
    static ViolatedAlignmentHandler handler = defaultViolatedAlignment;
    return handler;
  }

  void violatedAlignment(const char *className, std::size_t expectedAlignment,
                         const void *address)
  {
    const auto &handler = violatedAlignmentHandler();
    if(handler)
      handler(className, expectedAlignment, address);
  }

} // namespace Dune
