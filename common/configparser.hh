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

    void parseFile(string file);
    void parseCmd(int argc, char* argv []);

    bool hasKey(const string& key);
    bool hasSub(const string& key);

    string& operator[] (const string& key);

    void report() const;
    void report(const string prefix) const;

    ConfigParser& sub(const string& key);

    string get(const string& key, string defaultValue);
    string get(const string& key, char* defaultValue);
    int get(const string& key, int defaultValue);
    double get(const string& key, double defaultValue);

    /** \brief kurz
     *
     * langes
     * \param key Der Suchschlüssel
     * \param defaultValue
     * \return
     */
    bool get(const string& key, bool defaultValue);


  private:
    map<string, string> values;
    map<string, Config> subs;
    string trim(string s);


  };
} // end namespace dune



#endif
