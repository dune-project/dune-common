// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BITFIELD_HH
#define DUNE_BITFIELD_HH

#warning This file is deprecated! Use blockbitfield.hh instead!

#include <vector>
#include <iostream>

namespace Dune {

  /** \brief A dynamic array of booleans
   * \ingroup Common
   *
   * This class is basically std::vector<bool>, but with a few added
   * methods.
   */
  class BitField : public std::vector<bool> {

  public:

    //! Default constructor
    BitField() : std::vector<bool>() {}

    //! Constructor with a given length
    explicit BitField(int n) : std::vector<bool>(n) {}

    //! Constructor which initializes the field
    BitField(int n, bool v) : std::vector<bool>(n) {
      this->assign(size(), v);
    }

    //! Sets all entries to <tt> true </tt>
    void setAll() {
      this->assign(size(), true);
    }

    //! Sets all entries to <tt> false </tt>
    void unsetAll() {
      this->assign(size(), false);
    }

    //! Returns the number of set bits
    int nSetBits() const {
      int n = 0;
      for (size_t i=0; i<size(); i++)
        n += ((*this)[i]) ? 1 : 0;

      return n;
    }

    //! Send bitfield to an output stream
    friend std::ostream& operator<< (std::ostream& s, const BitField& v)
    {
      for (size_t i=0; i<v.size(); i++)
        s << v[i] << "  ";

      s << std::endl;
      return s;
    }

  } DUNE_DEPRECATED;

}

#endif
