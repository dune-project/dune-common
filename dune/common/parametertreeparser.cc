// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "parametertreeparser.hh"

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sstream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>

#include <dune/common/exceptions.hh>

std::string Dune::ParameterTreeParser::ltrim(const std::string& s)
{
  std::size_t firstNonWS = s.find_first_not_of(" \t\n\r");

  if (firstNonWS!=std::string::npos)
    return s.substr(firstNonWS);
  return std::string();
}

std::string Dune::ParameterTreeParser::rtrim(const std::string& s)
{
  std::size_t lastNonWS = s.find_last_not_of(" \t\n\r");

  if (lastNonWS!=std::string::npos)
    return s.substr(0, lastNonWS+1);
  return std::string();
}



void Dune::ParameterTreeParser::readINITree(std::string file,
                                            ParameterTree& pt,
                                            bool overwrite)
{
  std::ifstream in(file.c_str());

  if (!in)
    DUNE_THROW(Dune::IOError, "Could not open configuration file " << file);

  readINITree(in, pt, "file '" + file + "'", overwrite);
}


void Dune::ParameterTreeParser::readINITree(std::istream& in,
                                            ParameterTree& pt,
                                            bool overwrite)
{
  readINITree(in, pt, "stream", overwrite);
}


void Dune::ParameterTreeParser::readINITree(std::istream& in,
                                            ParameterTree& pt,
                                            const std::string srcname,
                                            bool overwrite)
{
  // reserved characters that can't occur in section or key names
  const static std::string reserved = "[]='\"#";

  std::string prefix;
  std::set<std::string> keysInFile;

  std::string line;
  while(getline(in, line))
  {
    // 1. trim leading whitespace
    auto buffer = ltrim(line);

    // 2. if line matches *=* (first =), set key
    auto mid = buffer.find('=');
    if(mid != buffer.npos) {
      auto key = prefix+rtrim(buffer.substr(0, mid));

      // check if key name is sane
      if(key.find_first_of(reserved) != key.npos)
        DUNE_THROW(ParameterTreeParserError, "Key name '" << key <<
                   "' in " << srcname <<
                   " containts invalid characters (one of "
                   << reserved << ")");

      // check if we already saw this key in this file
      if (keysInFile.count(key) != 0)
        DUNE_THROW(ParameterTreeParserError, "Key '" << key <<
                   "' appears twice in " << srcname << " !");
      keysInFile.insert(key);

      // determine value
      auto value = ltrim(buffer.substr(mid+1));
      // Note: value[0] is valid (and == '\0') even for empty value.
      if((value[0]=='\'') || (value[0]=='"'))
      {
        // handle quoted values
        // no comments allowed after quoted values
        char quote = value[0];
        value.erase(0,1);

        std::string trimmed;
        while(trimmed = rtrim(value),
              trimmed.empty() || trimmed.back() != quote)
        {
          if(!getline(in, buffer))
            DUNE_THROW(ParameterTreeParserError,
                       "Error while reading multi-line value for key " << key
                       << " in " << srcname);

          value += "\n";
          value += buffer;
        }
        value.erase(trimmed.length()-1);
      }
      else
      {
        // handle unquoted values
        // remove comments
        value = rtrim(value.substr(0, value.find("#")));
      }

      // set value
      if(overwrite || ! pt.hasKey(key))
        pt[key] = value;

      // next line
      continue;
    }

    // 3. trim comments and trailing whitespace
    buffer = rtrim(buffer.substr(0, buffer.find('#')));

    // 4. if line is empty, ignore it
    if(buffer.empty())
      continue;

    // 5. if line matches "[*", start section
    if(buffer.front() == '[' && buffer.back() == ']')
    {
      // check for closing bracket
      prefix = rtrim(ltrim(buffer.substr(1, buffer.length() - 2)));

      // check for reserved characters in section name
      if(prefix.find_first_of(reserved) != prefix.npos)
        DUNE_THROW(ParameterTreeParserError, "Section name '" << prefix <<
                   "' in " << srcname <<
                   " containts invalid characters (one of "
                   << reserved << ")");

      if(prefix != "") prefix += ".";

      continue;
    }

    // 6. otherwise error out
    DUNE_THROW(ParameterTreeParserError, "Invalid line '" << line << "' in "
               << srcname);
  }

  if(!in.eof())
    // we stopped reading the file due to something besides EOF
    DUNE_THROW(ParameterTreeParserError, "Error while reading " << srcname);

}

void Dune::ParameterTreeParser::readOptions(int argc, char* argv [],
                                            ParameterTree& pt)
{
  for(int i=1; i<argc; i++)
  {
    if ((argv[i][0]=='-') && (argv[i][1]!='\000'))
    {
      if(argv[i+1] == NULL)
        DUNE_THROW(RangeError, "last option on command line (" << argv[i]
                   << ") does not have an argument");
      pt[argv[i]+1] = argv[i+1];
      ++i; // skip over option argument
    }
  }
}

void Dune::ParameterTreeParser::readNamedOptions(int argc, char* argv[],
                                                 ParameterTree& pt,
                                                 std::vector<std::string> keywords,
                                                 unsigned int required,
                                                 bool allow_more,
                                                 bool overwrite,
                                                 std::vector<std::string> help)
{
  std::string helpstr = generateHelpString(argv[0], keywords, required, help);
  std::vector<bool> done(keywords.size(),false);
  std::size_t current = 0;

  for (std::size_t i=1; i<std::size_t(argc); i++)
  {
    std::string opt = argv[i];
    // check for help
    if (opt == "-h" || opt == "--help")
      DUNE_THROW(HelpRequest, helpstr);
    // is this a named parameter?
    if (opt.substr(0,2) == "--")
    {
      size_t pos = opt.find('=',2);
      if (pos == std::string::npos)
        DUNE_THROW(ParameterTreeParserError,
          "value missing for parameter " << opt << "\n" << helpstr);
      std::string key = opt.substr(2,pos-2);
      std::string value = opt.substr(pos+1,opt.size()-pos-1);
      auto it = std::find(keywords.begin(), keywords.end(), key);
      // is this param in the keywords?
      if (!allow_more && it == keywords.end())
          DUNE_THROW(ParameterTreeParserError,
            "unknown parameter " << key << "\n" << helpstr);
      // do we overwrite an existing entry?
      if (!overwrite && pt[key] != "")
        DUNE_THROW(ParameterTreeParserError,
          "parameter " << key << " already specified" << "\n" << helpstr);
      pt[key] = value;
      if(it != keywords.end())
        done[std::distance(keywords.begin(),it)] = true; // mark key as stored
    }
    else {
      // map to the next keyword in the list
      while(current < done.size() && done[current]) ++current;
      // are there keywords left?
      if (current >= done.size())
        DUNE_THROW(ParameterTreeParserError,
          "superfluous unnamed parameter" << "\n" << helpstr);
      // do we overwrite an existing entry?
      if (!overwrite && pt[keywords[current]] != "")
        DUNE_THROW(ParameterTreeParserError,
          "parameter " << keywords[current] << " already specified" << "\n" << helpstr);
      pt[keywords[current]] = opt;
      done[current] = true; // mark key as stored
    }
  }
  // check that we receive all required keywords
  std::string missing = "";
  for (unsigned int i=0; i<keywords.size(); i++)
    if ((i < required) && ! done[i]) // is this param required?
      missing += std::string(" ") + keywords[i];
  if (missing.size())
    DUNE_THROW(ParameterTreeParserError,
      "missing parameter(s) ... " << missing << "\n" << helpstr);
}

std::string Dune::ParameterTreeParser::generateHelpString(
  std::string progname, std::vector<std::string> keywords, unsigned int required, std::vector<std::string> help)
{
  static const char braces[] = "<>[]";
  std::string helpstr = "";
  helpstr = helpstr + "Usage: " + progname;
  for (std::size_t i=0; i<keywords.size(); i++)
  {
    bool req = (i < required);
    helpstr = helpstr +
      " " + braces[req*2] +
      keywords[i] +braces[req*2+1];
  }
  helpstr = helpstr + "\n"
    "Options:\n"
    "-h / --help: this help\n";
  for (std::size_t i=0; i<std::min(keywords.size(),help.size()); i++)
  {
    if (help[i] != "")
      helpstr = helpstr + "-" +
        keywords[i] + ":\t" + help[i] + "\n";
  }
  return helpstr;
}
