// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_INDENT_HH
#define DUNE_COMMON_INDENT_HH

#include <ostream>
#include <string>

namespace Dune {
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief  Utility class for handling nested indentation in output.
   * @author Jö Fahlke
   */
  //! Utility class for handling nested indentation in output.
  /**
   * An indentation object hast a string basic_indent and an indentation
   * level.  When it is put into a std::ostream using << it will print its
   * basic_indent as many times as its indentation level.  By default the
   * basic_indent will be two spaces and the indentation level will be 0.
   *
   * An Indent object may also have a reference to a parent Indent object.  If
   * it has, that object it put into the stream with the << operator before
   * the indentation of this object is put into the stream.  This effectively
   * chains Indent objects together.
   *
   * You can use the ++ operator to raise and the -- operator to lower the
   * indentation by one level.
   *
   * You can use the + operator with a numeric second argument morelevel to
   * create a copy of the Indent object with the indentation level increased
   * morelevel times.  This is mainly useful to pass indent+1 to a function,
   * where indent is an indentation object.
   *
   * You can use the + operator with a string second argument newindent to
   * create a new Indent object with this object as parent, a basic_indent of
   * newindent, and an indentation level of one.  This is mainly useful to
   * pass indent+"> " to a function, where "> " is a possibly different
   * indentation string then the one used by indent indentation object.
   *
   * \note The idea is for functions receive indentation objects as call by
   *       value parameters.  This way, the indentation object of the caller
   *       will not be modified by the function and the function can simply
   *       return at anytime without having to clean up.
   */
  class Indent
  {
    const Indent* parent;
    std::string basic_indent;
    unsigned level;

  public:
    //! setup without parent
    /**
     * \note Initial indentation level is 0 by default for this constructor.
     */
    inline Indent(const std::string& basic_indent_ = "  ", unsigned level_ = 0)
      : parent(0), basic_indent(basic_indent_), level(level_)
    { }

    //! setup without parent and basic_indentation of two spaces
    inline Indent(unsigned level_)
      : parent(0), basic_indent("  "), level(level_)
    { }

    //! setup with parent
    /**
     * \note Initial indentation level is 1 by default for this constructor.
     */
    inline Indent(const Indent* parent_,
                  const std::string& basic_indent_ = "  ", unsigned level_ = 1)
      : parent(parent_), basic_indent(basic_indent_), level(level_)
    { }

    //! setup with parent
    inline Indent(const Indent* parent_, unsigned level_)
      : parent(parent_), basic_indent("  "), level(level_)
    { }

    //! create new indentation object with this one as parent
    inline Indent operator+(const std::string& newindent) const {
      return Indent(this, newindent);
    }
    //! create a copy of this indentation object with raised level
    inline Indent operator+(unsigned morelevel) const {
      return Indent(parent, basic_indent, level+morelevel);
    }
    //! raise indentation level
    inline Indent& operator++() { ++level; return *this; }
    //! lower indentation level
    inline Indent& operator--() { if ( level > 0 ) --level; return *this; }

    //! write indentation to a stream
    friend inline std::ostream& operator<<(std::ostream& s,
                                           const Indent& indent);
  };

  //! write indentation to a stream
  inline std::ostream& operator<<(std::ostream& s, const Indent& indent) {
    if(indent.parent)
      s << *indent.parent;
    for(unsigned i = 0; i < indent.level; ++i)
      s << indent.basic_indent;
    return s;
  }

  /** }@ group Common */

} // namespace Dune

#endif // DUNE_COMMON_INDENT_HH
