// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PARAMETERTREE_HH
#define DUNE_PARAMETERTREE_HH

/** \file
 * \brief A hierarchical structure of string parameters
 */

#include <array>
#include <cstddef>
#include <iostream>
#include <istream>
#include <iterator>
#include <locale>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <bitset>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/classname.hh>

namespace Dune {

  /** \brief Hierarchical structure of string parameters
   * \ingroup Common
   */
  class ParameterTree
  {
    // class providing a single static parse() function, used by the
    // generic get() method
    template<typename T>
    struct Parser;

  public:

    /** \brief storage for key lists
     */
    typedef std::vector<std::string> KeyVector;

    /** \brief Create new empty ParameterTree
     */
    ParameterTree();


    /** \brief test for key
     *
     * Tests whether given key exists.
     *
     * \param key key name
     * \return true if key exists in structure, otherwise false
     */
    bool hasKey(const std::string& key) const;


    /** \brief test for substructure
     *
     * Tests whether given substructure exists.
     *
     * \param sub substructure name
     * \return true if substructure exists in structure, otherwise false
     */
    bool hasSub(const std::string& sub) const;


    /** \brief get value reference for key
     *
     * Returns reference to value for given key name.
     * This creates the key, if not existent.
     *
     * \param key key name
     * \return reference to corresponding value
     */
    std::string& operator[] (const std::string& key);


    /** \brief get value reference for key
     *
     * Returns reference to value for given key name.
     * This creates the key, if not existent.
     *
     * \param key key name
     * \return reference to corresponding value
     * \throw Dune::RangeError if key is not found
     */
    const std::string& operator[] (const std::string& key) const;


    /** \brief print distinct substructure to stream
     *
     * Prints all entries with given prefix.
     *
     * \param stream Stream to print to
     * \param prefix for key and substructure names
     */
    void report(std::ostream& stream = std::cout,
                const std::string& prefix = "") const;


    /** \brief get substructure by name
     *
     * \param sub substructure name
     * \return reference to substructure
     */
    ParameterTree& sub(const std::string& sub);


    /** \brief get const substructure by name
     *
     * \param sub              substructure name
     * \param fail_if_missing  if true, throw an error if substructure is missing
     * \return                 reference to substructure
     */
    const ParameterTree& sub(const std::string& sub, bool fail_if_missing = false) const;


    /** \brief get value as string
     *
     * Returns pure string value for given key.
     *
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as string
     */
    std::string get(const std::string& key, const std::string& defaultValue) const;

    /** \brief get value as string
     *
     * Returns pure string value for given key.
     *
     * \todo This is a hack so get("my_key", "xyz") compiles
     * (without this method "xyz" resolves to bool instead of std::string)
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as string
     */
    std::string get(const std::string& key, const char* defaultValue) const;


    /** \brief get value converted to a certain type
     *
     * Returns value as type T for given key.
     *
     * \tparam T type of returned value.
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value converted to T
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const {
      if(hasKey(key))
        return get<T>(key);
      else
        return defaultValue;
    }

    /** \brief Get value
     *
     * \tparam T Type of the value
     * \param key Key name
     * \throws RangeError if key does not exist
     * \throws NotImplemented Type is not supported
     * \return value as T
     */
    template <class T>
    T get(const std::string& key) const {
      if(not hasKey(key))
        DUNE_THROW(Dune::RangeError, "Key '" << key
          << "' not found in ParameterTree (prefix " + prefix_ + ")");
      try {
        return Parser<T>::parse((*this)[key]);
      }
      catch(const RangeError& e) {
        // rethrow the error and add more information
        DUNE_THROW(RangeError, "Cannot parse value \"" << (*this)[key]
          << "\" for key \"" << prefix_ << "." << key << "\""
          << e.what());
      }
    }

    /** \brief get value keys
     *
     * Returns a vector of all keys associated to (key,values) entries in
     * order of appearance
     *
     * \return reference to entry vector
     */
    const KeyVector& getValueKeys() const;


    /** \brief get substructure keys
     *
     * Returns a vector of all keys associated to (key,substructure) entries
     * in order of appearance
     *
     * \return reference to entry vector
     */
    const KeyVector& getSubKeys() const;

  protected:

    static const ParameterTree empty_;

    std::string prefix_;

    KeyVector valueKeys_;
    KeyVector subKeys_;

    std::map<std::string, std::string> values_;
    std::map<std::string, ParameterTree> subs_;

    static std::string ltrim(const std::string& s);
    static std::string rtrim(const std::string& s);
    static std::vector<std::string> split(const std::string & s);

    // parse into a fixed-size range of iterators
    template<class Iterator>
    static void parseRange(const std::string &str,
                           Iterator it, const Iterator &end)
    {
      typedef typename std::iterator_traits<Iterator>::value_type Value;
      std::istringstream s(str);
      // make sure we are in locale "C"
      s.imbue(std::locale::classic());
      std::size_t n = 0;
      for(; it != end; ++it, ++n) {
        s >> *it;
        if(!s)
          DUNE_THROW(RangeError, "as a range of items of type "
            << className<Value>()
            << " (" << n << " items were extracted successfully)");
      }
      Value dummy;
      s >> dummy;
      // now extraction should have failed, and eof should be set
      if(not s.fail() or not s.eof())
        DUNE_THROW(RangeError, "as a range of "
          << n << " items of type "
          << className<Value>() << " (more items than the range can hold)");
    }
  };

  template<typename T>
  struct ParameterTree::Parser {
    static T parse(const std::string& str) {
      T val;
      std::istringstream s(str);
      // make sure we are in locale "C"
      s.imbue(std::locale::classic());
      s >> val;
      if(!s)
        DUNE_THROW(RangeError, " as a " << className<T>());
      char dummy;
      s >> dummy;
      // now extraction should have failed, and eof should be set
      if ((! s.fail()) || (! s.eof()))
        DUNE_THROW(RangeError, " as a " << className<T>());
      return val;
    }
  };

  // "How do I convert a string into a wstring in C++?"  "Why, that very simple
  // son. You just need a these hundred lines of code."
  // Instead im going to restrict myself to string with charT=char here.
  template<typename traits, typename Allocator>
  struct ParameterTree::Parser<std::basic_string<char, traits, Allocator> > {
    static std::basic_string<char, traits, Allocator>
    parse(const std::string& str) {
      std::string trimmed = ltrim(rtrim(str));
      return std::basic_string<char, traits, Allocator>(trimmed.begin(),
                                                        trimmed.end());
    }
  };

  template<>
  struct ParameterTree::Parser< bool > {
    struct ToLower {
      char operator()(char c)
      {
        return std::tolower(c, std::locale::classic());
      }
    };

    static bool
    parse(const std::string& str) {
      std::string ret = str;

      std::transform(ret.begin(), ret.end(), ret.begin(), ToLower());

      if (ret == "yes" || ret == "true")
        return true;

      if (ret == "no" || ret == "false")
        return false;

      return (Parser<int>::parse(ret) != 0);
    }
  };

  template<typename T, int n>
  struct ParameterTree::Parser<FieldVector<T, n> > {
    static FieldVector<T, n>
    parse(const std::string& str) {
      FieldVector<T, n> val;
      parseRange(str, val.begin(), val.end());
      return val;
    }
  };

  template<typename T, std::size_t n>
  struct ParameterTree::Parser<std::array<T, n> > {
    static std::array<T, n>
    parse(const std::string& str) {
      std::array<T, n> val;
      parseRange(str, val.begin(), val.end());
      return val;
    }
  };

  template<std::size_t n>
  struct ParameterTree::Parser<std::bitset<n> > {
    static std::bitset<n>
    parse(const std::string& str) {
      std::bitset<n> val;
      std::vector<std::string> sub = split(str);
      if (sub.size() != n)
        DUNE_THROW(RangeError, "as a bitset<" << n << "> "
          << "because of unmatching size " << sub.size());
      for (std::size_t i=0; i<n; ++i) {
        val[i] = ParameterTree::Parser<bool>::parse(sub[i]);
      }
      return val;
    }
  };

  template<typename T, typename A>
  struct ParameterTree::Parser<std::vector<T, A> > {
    static std::vector<T, A>
    parse(const std::string& str) {
      std::vector<std::string> sub = split(str);
      std::vector<T, A> vec;
      for (unsigned int i=0; i<sub.size(); ++i) {
        T val = ParameterTree::Parser<T>::parse(sub[i]);
        vec.push_back(val);
      }
      return vec;
    }
  };

} // end namespace Dune

#endif // DUNE_PARAMETERTREE_HH
