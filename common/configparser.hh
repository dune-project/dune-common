// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CONFIGPARSER_HH
#define DUNE_CONFIGPARSER_HH


#include <map>
#include <string>
#include <iostream>

namespace dune {

  /** \brief Parser for hierarchical parameter files
   * \ingroup Common
   *
   * This class parses files into a hierachical structure.
   *
   */

  class ConfigParser
  {
  public:


    /** \brief Parser for hierarchical parameter files
     *
     * This class parses files into a hierachical structure.
     *
     */
    ConfigParser();

    void parseFile(std::string file);
    void parseCmd(int argc, char* argv []);

    bool hasKey(const std::string& key);
    bool hasSub(const std::string& key);

    std::string& operator[] (const std::string& key);

    void report() const;
    void report(const std::string prefix) const;

    ConfigParser& sub(const std::string& key);

    std::string get(const std::string& key, std::string defaultValue);
    std::string get(const std::string& key, char* defaultValue);
    int get(const std::string& key, int defaultValue);
    double get(const std::string& key, double defaultValue);

    /** \brief kurz
     *
     * langes
     * \param key Der Suchschlüssel
     * \param defaultValue
     * \return
     */
    bool get(const std::string& key, bool defaultValue);


  private:
    std::map<std::string, std::string> values;
    std::map<std::string, ConfigParser> subs;
    std::string trim(std::string s);


  };
} // end namespace dune



#endif
