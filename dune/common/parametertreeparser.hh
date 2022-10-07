// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PARAMETER_PARSER_HH
#define DUNE_PARAMETER_PARSER_HH

/** \file
 * \brief Various parser methods to get data into a ParameterTree object
 */

#include <istream>
#include <string>
#include <vector>

#include <dune/common/parametertree.hh>
#include <dune/common/exceptions.hh>

namespace Dune {

  /** \brief report parser error while reading ParameterTree */
  class ParameterTreeParserError : public RangeError {};
  /** \brief exception thrown if the user wants to see help string

      this exception is only thrown if the command line parameters
      contain an option --help or -h
   */
  class HelpRequest : public Exception {};

  /** \brief Parsers to set up a ParameterTree from various input sources
   * \ingroup Common
   *
   */
  class ParameterTreeParser
  {

    static std::string ltrim(const std::string& s);
    static std::string rtrim(const std::string& s);

  public:

    /** @name Parsing methods for the INITree file format
     *
     *  INITree files should look like this
     *  \verbatim
     * # this file configures fruit colors in fruitsalad
     *
     *
     * #these are no fruit but could also appear in fruit salad
     * honeydewmelon = yellow
     * watermelon = green
     *
     * fruit.tropicalfruit.orange = orange
     *
     * [fruit]
     * strawberry = red
     * pomegranate = red
     *
     * [fruit.pipfruit]
     * apple = green/red/yellow
     * pear = green
     *
     * [fruit.stonefruit]
     * cherry = red
     * plum = purple
     *
     * \endverbatim
     *
     *
     * If a '[prefix]' statement appears all following entries use this prefix
     * until the next '[prefix]' statement. Fruitsalads for example contain:
     * \verbatim
     * honeydewmelon = yellow
     * fruit.tropicalfruit.orange = orange
     * fruit.pipfruit.apple = green/red/yellow
     * fruit.stonefruit.cherry = red
     * \endverbatim
     *
     * All keys with a common 'prefix.' belong to the same substructure called
     * 'prefix'.  Leading and trailing spaces and tabs are removed from the
     * values unless you use single or double quotes around them.  Using single
     * or double quotes you can also have multiline values.
     */
    //@{

    /** \brief parse C++ stream
     *
     * Parses C++ stream and build hierarchical config structure.
     *
     * \param in        The stream to parse
     * \param[out] pt        The parameter tree to store the config structure.
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     */
    static void readINITree(std::istream& in, ParameterTree& pt,
                            bool overwrite);

    /** \brief parse C++ stream
     *
     * Parses C++ stream and returns hierarchical config structure.
     *
     * \param in        The stream to parse
     */
    static Dune::ParameterTree readINITree(std::istream& in);


    /** \brief parse C++ stream
     *
     * Parses C++ stream and build hierarchical config structure.
     *
     * \param in      The stream to parse
     * \param[out] pt      The parameter tree to store the config structure.
     * \param srcname Name of the configuration source for error
     *                messages, "stdin" or a filename.
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     */
    static void readINITree(std::istream& in, ParameterTree& pt,
                            const std::string srcname = "stream",
                            bool overwrite = true);


    /** \brief parse file
     *
     * Parses file with given name and build hierarchical config structure.
     *
     * \param file filename
     * \param[out] pt   The parameter tree to store the config structure.
     * \param overwrite Whether to overwrite already existing values.
     *                  If false, values in the stream will be ignored
     *                  if the key is already present.
     */
    static void readINITree(std::string file, ParameterTree& pt, bool overwrite = true);

    /** \brief parse file and return tree
     *
     * Parses file with given name and returns hierarchical config structure.
     *
     * \param file filename
     */
    static Dune::ParameterTree readINITree(const std::string& file);

    //@}

    /** \brief parse command line options and build hierarchical ParameterTree structure
     *
     * The list of command line options is searched for pairs of the type <kbd>-key value</kbd>
     * (note the hyphen in front of the key).
     * For each such pair of options a key-value pair with the corresponding names
     * is then created in the ParameterTree.
     *
     * \param argc arg count
     * \param argv arg values
     * \param[out] pt   The parameter tree to store the config structure.
     */
    static void readOptions(int argc, char* argv [], ParameterTree& pt);

    /**
     * \brief read [named] command line options and build hierarchical ParameterTree structure
     *
     * Similar to pythons named options we expect the parameters in the
     * ordering induced by keywords, but allow the user to pass named options
     * in the form of --key=value. Optionally the user can pass an additional
     * vector with help strings.
     *
     * \param argc arg count
     * \param argv arg values
     * \param[out] pt   The parameter tree to store the config structure.
     * \param keywords vector with keywords names
     * \param required number of required options (the first n keywords are required, default is all are required)
     * \param allow_more allow more options than these listed in keywords (default = true)
     * \param overwrite  allow to overwrite existing options (default = true)
     * \param help vector containing help strings
    */
    static void readNamedOptions(int argc, char* argv[],
      ParameterTree& pt,
      std::vector<std::string> keywords,
      unsigned int required = std::numeric_limits<unsigned int>::max(),
      bool allow_more = true,
      bool overwrite = true,
      std::vector<std::string> help = std::vector<std::string>());

  private:
    static std::string generateHelpString(std::string progname, std::vector<std::string> keywords, unsigned int required, std::vector<std::string> help);
  };

} // end namespace Dune

#endif // DUNE_PARAMETER_PARSER_HH
