// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ISTL_AMG_PROPERTIES_HH
#define DUNE_ISTL_AMG_PROPERTIES_HH

#include "dune/common/propertymap.hh"

namespace Dune
{

  namespace Amg
  {
    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */
    /** @file
     * @author Markus Blatt
     * @brief Provides classes for handling internal properties in a graph
     */

    /**
     * @brief Tag idnetifying the visited property of a vertex.
     */
    struct VertexVisitedTag
    {};


    /**
     * @brief A property map that extracts one property out of a bundle
     * using operator[]()
     *
     * Using this access class properties can be stored in std::bitset.
     */
    template<typename C, typename K, std::size_t i,typename T=typename C::ValueType,
        typename R = typename C::Reference>
    class RandomAccessBundledPropertyMap
      : public RAPropertyMapHelper<R,
            RandomAccessBundledPropertyMap<C,K,i,T,R> >
    {
    public:
      /** @brief The container that holds the properties */
      typedef C Container;

      /** @brief The reference type of the container. */
      typedef R Reference;

      /** @brief The key of the property map. */
      typedef K Key;

      /**
       * @brief The category of the property map.
       */
      typedef LvaluePropertyMapTag Category;

      enum {
        /** @brief The index of the property in the bundle. */
        index = i
      };

      /**
       * @brief Get the property for a key.
       * @param key The key.
       * @return The corresponding property.
       */
      Reference operator[](const Key& key) const
      {
        return container_[key][index];
      }

      /**
       * @brief Constructor.
       * @param container The container with the property bundle.
       */
      RandomAccessBundledPropertyMap(Container& container)
        : container_(&container)
      {}

      /** @brief The default constructor. */
      RandomAccessBundledPropertyMap()
        : container_(0)
      {}

    private:
      /** @brief The container with property bundles. */
      Container* container_;
    };
  }
}

#endif
