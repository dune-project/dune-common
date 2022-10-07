// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PROPERTYMAP_HH
#define DUNE_PROPERTYMAP_HH

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace Dune
{

  template<class PM>
  struct PropertyMapTraits
  {
    /**
     * @brief The type of the key of the property map.
     */
    typedef typename PM::KeyType KeyType;
    /**
     * @brief The type of the values of the property map.
     */
    typedef typename PM::ValueType ValueType;
    /**
     * @brief The type of the reference to the values.
     */
    typedef typename PM::Reference Reference;
    /**
     * @brief The category the property map belongs to.
     */
    typedef typename PM::Category Category;
  };

  /** @brief Tag for the category of readable property maps. */
  struct ReadablePropertyMapTag
  {};

  /** @brief Tag for the category of writable property maps. */
  struct WritablePropertyMapTag
  {};

  /**
   * @brief Tag for the category of readable and writable property
   * maps.
   */
  struct ReadWritePropertyMapTag
    : public ReadablePropertyMapTag, public WritablePropertyMapTag
  {};

  /**
   * @brief Tag for the category of lvalue property maps.
   */
  struct LvaluePropertyMapTag
    : public ReadWritePropertyMapTag
  {};

  template<class T>
  struct PropertyMapTraits<T*>
  {
    typedef T ValueType;
    typedef ValueType& Reference;
    typedef std::ptrdiff_t KeyType;
    typedef LvaluePropertyMapTag Category;
  };


  template<class T>
  struct PropertyMapTraits<const T*>
  {
    typedef T ValueType;
    typedef const ValueType& Reference;
    typedef std::ptrdiff_t KeyType;
    typedef LvaluePropertyMapTag Category;
  };

  template<class Reference, class PropertyMap>
  struct RAPropertyMapHelper
  {};

  template<class Reference, class PropertyMap, class Key>
  inline Reference
  get(const RAPropertyMapHelper<Reference,PropertyMap>& pmap,
      const Key& key)
  {
    return static_cast<const PropertyMap&>(pmap)[key];
  }

  template<class Reference, class PropertyMap, class Key, class Value>
  inline void
  put(const RAPropertyMapHelper<Reference,PropertyMap>& pmap,
      const Key& key, const Value& value)
  {
    static_assert(std::is_convertible<typename PropertyMap::Category,WritablePropertyMapTag>::value,
                  "WritablePropertyMapTag required!");
    static_cast<const PropertyMap&>(pmap)[key] = value;
  }

  /**
   * @brief Adapter to turn a random access iterator into a property map.
   */
  template<class RAI, class IM,
      class T = typename std::iterator_traits<RAI>::value_type,
      class R = typename std::iterator_traits<RAI>::reference>
  class IteratorPropertyMap
    : public RAPropertyMapHelper<R,IteratorPropertyMap<RAI,IM,T,R> >
  {
  public:
    /**
     * @brief The type of the random access iterator.
     */
    typedef RAI RandomAccessIterator;

    /**
     * @brief The type of the index map.
     *
     * This will convert the KeyType to std::ptrdiff_t via operator[]().
     */
    typedef IM IndexMap;

    /**
     * @brief The key type of the property map.
     */
    typedef typename IndexMap::KeyType KeyType;

    /**
     * @brief The value type of the property map.
     */
    typedef T ValueType;

    /**
     * @brief The reference type of the property map.
     */
    typedef R Reference;

    /**
     * @brief The category of this property map.
     */
    typedef LvaluePropertyMapTag Category;

    /**
     * @brief Constructor.
     * @param iter The random access iterator that
     * provides the mapping.
     * @param im The index map that maps the KeyType
     * to the difference_type of the iterator.
     */
    inline IteratorPropertyMap(RandomAccessIterator iter,
                               const IndexMap& im=IndexMap())
      : iter_(iter), indexMap_(im)
    {}

    /** @brief Constructor. */
    inline IteratorPropertyMap()
      : iter_(), indexMap_()
    {}

    /** @brief Access the a value by reference. */
    inline Reference operator[](KeyType key) const
    {
      return *(iter_ + get(indexMap_, key));
    }

  private:
    /** @brief The underlying iterator. */
    RandomAccessIterator iter_;
    /** @brief The index map to use for the lookup. */
    IndexMap indexMap_;
  };

  /**
   * @brief An adapter to turn an unique associative container
   * into a property map.
   */
  template<typename T>
  class AssociativePropertyMap
    : RAPropertyMapHelper<typename T::value_type::second_type&,
          AssociativePropertyMap<T> >
  {
    /**
     * @brief The type of the unique associative container.
     */
    typedef T UniqueAssociativeContainer;

    /**
     * @brief The key type of the property map.
     */
    typedef typename UniqueAssociativeContainer::value_type::first_type
    KeyType;

    /**
     * @brief The value type of the property map.
     */
    typedef typename UniqueAssociativeContainer::value_type::second_type
    ValueType;

    /**
     * @brief The reference type of the property map.
     */
    typedef ValueType& Reference;

    /**
     * @brief The category of the property map.
     */
    typedef LvaluePropertyMapTag Category;

    /** @brief Constructor */
    inline AssociativePropertyMap()
      : map_(0)
    {}

    /** @brief Constructor. */
    inline AssociativePropertyMap(UniqueAssociativeContainer& map)
      : map_(&map)
    {}

    /**
     * @brief Access a property.
     * @param key The key of the property.
     */
    inline Reference operator[](KeyType key) const
    {
      return map_->find(key)->second;
    }
  private:
    UniqueAssociativeContainer* map_;
  };

  /**
   * @brief An adaptor to turn an unique associative container
   * into a property map.
   */
  template<typename T>
  class ConstAssociativePropertyMap
    : RAPropertyMapHelper<const typename T::value_type::second_type&,
          ConstAssociativePropertyMap<T> >
  {
    /**
     * @brief The type of the unique associative container.
     */
    typedef T UniqueAssociativeContainer;

    /**
     * @brief The key type of the property map.
     */
    typedef typename UniqueAssociativeContainer::value_type::first_type
    KeyType;

    /**
     * @brief The value type of the property map.
     */
    typedef typename UniqueAssociativeContainer::value_type::second_type
    ValueType;

    /**
     * @brief The reference type of the property map.
     */
    typedef const ValueType& Reference;

    /**
     * @brief The category of the property map.
     */
    typedef LvaluePropertyMapTag Category;

    /** @brief Constructor */
    inline ConstAssociativePropertyMap()
      : map_(0)
    {}

    /** @brief Constructor. */
    inline ConstAssociativePropertyMap(const UniqueAssociativeContainer& map)
      : map_(&map)
    {}

    /**
     * @brief Access a property.
     * @param key The key of the property.
     */
    inline Reference operator[](KeyType key) const
    {
      return map_->find(key)->second;
    }
  private:
    const UniqueAssociativeContainer* map_;
  };

  /**
   * @brief A property map that applies the identity function to integers.
   */
  struct IdentityMap
    : public RAPropertyMapHelper<std::size_t, IdentityMap>
  {
    /** @brief The key type of the map. */
    typedef std::size_t KeyType;

    /** @brief The value type of the map. */
    typedef std::size_t ValueType;

    /** @brief The reference type of the map. */
    typedef std::size_t Reference;

    /** @brief The category of the map. */
    typedef ReadablePropertyMapTag Category;

    inline ValueType operator[](const KeyType& key) const
    {
      return key;
    }
  };


  /**
   * @brief Selector for the property map type.
   *
   * If present the type of the property map is accessible via the typedef Type.
   */
  template<typename T, typename C>
  struct PropertyMapTypeSelector
  {
    /**
     * @brief the tag identifying the property.
     */
    typedef T Tag;
    /**
     * @brief The container type to whose entries the properties
     * are attached.
     */
    typedef C Container;
  };

}

#endif
