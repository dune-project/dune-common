// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CONFIGPARSER_HH
#define DUNE_CONFIGPARSER_HH


#include <istream>
#include <map>
#include <ostream>
#include <typeinfo>
#include <vector>
#include <string>
#include <iostream>

#include <dune/common/fvector.hh>

namespace Dune {

  /** \brief Parser for hierarchical configuration files
   * \ingroup Common
   *
   * This class parses config files into a hierarchical structure.
   * Config files should look like this
   *
     \verbatim
   # this file configures fruit colors in fruitsalad


   ##these are no fruit but could also appear in fruit salad
     honeydewmelon = yellow
     watermelon = green

     fruit.tropicalfruit.orange = orange

     [fruit]
     strawberry = red
     pomegranate = red

     [fruit.pipfruit]
     apple = green/red/yellow
     pear = green

     [fruit.stonefruit]
     cherry = red
     plum = purple

     \endverbatim
   *
   *
   * If a '[prefix]' statement appears all following entries use this prefix
   * until the next '[prefix]' statement. Fruitsalads for example contain:
     \verbatim
     honeydewmelon = yellow
     fruit.tropicalfruit.orange = orange
     fruit.pipfruit.apple = green/red/yellow
     fruit.stonefruit.cherry = red
     \endverbatim
   *
   * All keys with a common 'prefix.' belong to the same substructure called 'prefix'.
   * Leading and trailing spaces and tabs are removed from the values unless you use
   * single or double quotes around them.
   * Using single or double quotes you can also have multiline values.
   *
   */
  class ConfigParser
  {
    // class providing a single static parse() function, used by the
    // generic get() method
    template<typename T>
    struct Parser;

  public:

    typedef std::vector<std::string> KeyVector;

    /** \brief Create new ConfigParser
     */
    ConfigParser();


    /** \brief parse C++ stream
     *
     * Parses C++ stream and build hierarchical config structure.
     *
     * \param in        The stream to parse
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     *
     * \note This method is identical to parseStream(std::istream&,
     *       const std::string&, bool) with the exception that that
     *       method allows to give a custom name for the stream.
     */
    void parseStream(std::istream& in,
                     bool overwrite);


    /** \brief parse C++ stream
     *
     * Parses C++ stream and build hierarchical config structure.
     *
     * \param in      The stream to parse
     * \param srcname Name of the configuration source for error
     *                messages, "stdin" or a filename.
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     */
    void parseStream(std::istream& in,
                     const std::string srcname = "stream",
                     bool overwrite = true);


    /** \brief parse file
     *
     * Parses file with given name and build hierarchical config structure.
     *
     * \param file filename
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     */
    void parseFile(std::string file, bool overwrite = true);


    /** \brief parse command line
     *
     * Parses command line options and build hierarchical config structure.
     *
     * \param argc arg count
     * \param argv arg values
     */
    void parseCmd(int argc, char* argv []);


    /** \brief test for key
     *
     * Tests wether given key exists.
     *
     * \param key key name
     * \return true if key exists in structure, otherwise false
     */
    bool hasKey(const std::string& key) const;


    /** \brief test for substructure
     *
     * Tests wether given substructure exists.
     *
     * \param sub substructure name
     * \return true if substructure exists in structure, otherwise false
     */
    bool hasSub(const std::string& sub) const;


    /** \brief get value reference for key
     *
     * Returns refference to value for given key name.
     * This creates the key, if not existent.
     *
     * \param key key name
     * \return reference to coresponding value
     */
    std::string& operator[] (const std::string& key);


    /** \brief print structure to std::cout
     */
    void report() const;


    /** \brief print distinct substructure to std::cout
     *
     * Prints all entries with given prefix.
     *
     * \param prefix for key and substructure names
     */
    void report(const std::string prefix) const;


    /** \brief print distinct substructure to stream
     *
     * Prints all entries with given prefix.
     *
     * \param stream Stream to print to
     * \param prefix for key and substructure names
     */
    void reportStream(std::ostream& stream,
                      const std::string& prefix = "") const;


    /** \brief get substructure by name
     *
     * \param sub substructure name
     * \return reference to substructure
     */
    ConfigParser& sub(const std::string& sub);


    /** \brief get const substructure by name
     *
     * \param sub substructure name
     * \return reference to substructure
     */
    const ConfigParser& sub(const std::string& sub) const;


    /** \brief get value as string
     *
     * Returns pure string value for given key.
     *
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as string
     */
    std::string get(const std::string& key, const std::string& defaultValue);

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
    std::string get(const std::string& key, const char* defaultValue);


    /** \brief get value as int
     *
     * Returns value for given key interpreted as int.
     *
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as int
     */
    int get(const std::string& key, int defaultValue);


    /** \brief get value as double
     *
     * Returns value for given key interpreted as double.
     *
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as double
     */
    double get(const std::string& key, double defaultValue);


    /** \brief get value as bool
     *
     * Returns value for given key interpreted as bool.
     *
     * \param key key name
     * \param defaultValue default if key does not exist
     * \return value as bool, false if values = '0', true if value = '1'
     */
    bool get(const std::string& key, bool defaultValue);


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
    T get(const std::string& key, const T& defaultValue) {
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
    T get(const std::string& key) {
      if(not hasKey(key))
        DUNE_THROW(RangeError, "Key '" << key << "' not found in "
                   "parameter file!");
      try {
        return Parser<T>::parse((*this)[key]);
      }
      catch(const RangeError&) {
        DUNE_THROW(RangeError, "Cannot parse value \"" <<
                   (*this)[key] << "\" for key \"" << key << "\" "
                   "as a " << typeid(T).name());
      }
    }

    /** \brief get value keys
     *
     * Returns a vector of all keys associated to (key,values) entries in order of appearance
     *
     * \return reference to entry vector
     */
    const KeyVector& getValueKeys() const;


    /** \brief get substructure keys
     *
     * Returns a vector of all keys associated to (key,substructure) entries in order of appearance
     *
     * \return reference to entry vector
     */
    const KeyVector& getSubKeys() const;

  private:
    KeyVector valueKeys;
    KeyVector subKeys;

    std::map<std::string, std::string> values;
    std::map<std::string, ConfigParser> subs;
    static std::string ltrim(const std::string& s);
    static std::string rtrim(const std::string& s);
  };

  template<typename T>
  struct ConfigParser::Parser {
    static T parse(const std::string& str) {
      T val;
      std::istringstream s(str);
      s >> val;
      if(!s)
        DUNE_THROW(RangeError, "Cannot parse value \"" << str << "\" "
                   "as a " << typeid(T).name());
      T dummy;
      s >> dummy;
      // now extraction should have failed, and eof should be set
      if(not s.fail() or not s.eof())
        DUNE_THROW(RangeError, "Cannot parse value \"" << str << "\" "
                   "as a " << typeid(T).name());
      return val;
    }
  };

  template<typename T, int n>
  struct ConfigParser::Parser<FieldVector<T, n> > {
    static FieldVector<T, n>
    parse(const std::string& str) {
      FieldVector<T, n> val;
      std::istringstream s(str);
      for(int i = 0; i < n; ++i) {
        s >> val[i];
        if(!s)
          DUNE_THROW(RangeError, "Cannot parse value \"" << str <<
                     "\" as a FieldVector<" << typeid(T).name() <<
                     ", " << n << ">");
      }
      T dummy;
      s >> dummy;
      // now extraction should have failed, and eof should be set
      if(not s.fail() or not s.eof())
        DUNE_THROW(RangeError, "Cannot parse value \"" << str << "\" "
                   "as a FieldVector<double, " << n << ">");
      return val;
    }
  };

} // end namespace dune



#endif
