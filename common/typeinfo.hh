// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// Last update: June 20, 2001

#ifndef DUNE_TYPEINFO_HH
#define DUNE_TYPEINFO_HH

#include <typeinfo>
#include <cassert>

namespace Dune
{
  //! Purpose: offer a first-class, comparable wrapper over std::type_info
  //! This implementation is borrowed from "Modern C++ Design" by A.
  //! Alexandrescu (see copyright information in typeinfo.hh)
  class TypeInfo
  {
  public:
    //! Default constructor
    TypeInfo(); // needed for containers
    //! Constructor and conversion operator from standard type_info struct
    TypeInfo(const std::type_info&); // non-explicit

    //! Access for the wrapped std::type_info
    const std::type_info& Get() const;
    //! Compatibility function for ordering
    bool before(const TypeInfo& rhs) const;
    //! Compatibility function for name
    const char* name() const;

  private:
    const std::type_info* pInfo_;
  };

  // Implementation

  inline TypeInfo::TypeInfo()
  {
    class Nil {};
    pInfo_ = &typeid(Nil);
    assert(pInfo_);
  }

  inline TypeInfo::TypeInfo(const std::type_info& ti)
    : pInfo_(&ti)
  {
    assert(pInfo_);
  }

  inline bool TypeInfo::before(const TypeInfo& rhs) const
  {
    assert(pInfo_);
    // type_info::before return type is int in some VC libraries
    return pInfo_->before(*rhs.pInfo_) != 0;
  }

  inline const std::type_info& TypeInfo::Get() const
  {
    assert(pInfo_);
    return *pInfo_;
  }

  inline const char* TypeInfo::name() const
  {
    assert(pInfo_);
    return pInfo_->name();
  }

  //! Comparison operators
  inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs)
  // type_info::operator== return type is int in some VC libraries
  {
    return (lhs.Get() == rhs.Get()) != 0;
  }

  inline bool operator<(const TypeInfo& lhs, const TypeInfo& rhs)
  {
    return lhs.before(rhs);
  }

  inline bool operator!=(const TypeInfo& lhs, const TypeInfo& rhs)
  {
    return !(lhs == rhs);
  }

  inline bool operator>(const TypeInfo& lhs, const TypeInfo& rhs)
  {
    return rhs < lhs;
  }

  inline bool operator<=(const TypeInfo& lhs, const TypeInfo& rhs)
  {
    return !(lhs > rhs);
  }

  inline bool operator>=(const TypeInfo& lhs, const TypeInfo& rhs)
  {
    return !(lhs < rhs);
  }
} // end namespace Dune

////////////////////////////////////////////////////////////////////////////////
// Change log:
// June 20, 2001: ported by Nick Thurn to gcc 2.95.3. Kudos, Nick!!!
////////////////////////////////////////////////////////////////////////////////

#endif
