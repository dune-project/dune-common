// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_MEMORY_DOMAIN_HH
#define DUNE_COMMON_MEMORY_DOMAIN_HH

namespace Dune {
  namespace Memory {
    namespace Domain {

      struct Host
      {};

      struct CUDA
      {};

      // to be perhaps changed later?
      typedef Host Default;

    } // namespace Domain

    template<typename Allocator>
    struct allocator_domain;

  } // namespace Memory
} //namespace Dune

#endif // DUNE_COMMON_MEMORY_DOMAIN_HH
