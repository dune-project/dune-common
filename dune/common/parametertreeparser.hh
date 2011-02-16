// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PARAMETER_PARSER_HH
#define DUNE_PARAMETER_PARSER_HH

#include <istream>
#include <string>

#include <dune/common/parametertree.hh>

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
   * All keys with a common 'prefix.' belong to the same substructure called
   * 'prefix'.  Leading and trailing spaces and tabs are removed from the
   * values unless you use single or double quotes around them.  Using single
   * or double quotes you can also have multiline values.
   *
   */
  class ParameterTreeParser
  {

    static std::string ltrim(const std::string& s);
    static std::string rtrim(const std::string& s);


  public:
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
    static void readINITree(std::istream& in, ParameterTree& pt,
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
    static void readINITree(std::istream& in, ParameterTree& pt,
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
    static void readINITree(std::string file, ParameterTree& pt, bool overwrite = true);


    /** \brief parse command line
     *
     * Parses command line options and build hierarchical ParameterTree structure.
     *
     * \param argc arg count
     * \param argv arg values
     */
    static void readOptions(int argc, char* argv [], ParameterTree& pt);

  };

} // end namespace Dune

#endif // DUNE_PARAMETER_PARSER_HH
