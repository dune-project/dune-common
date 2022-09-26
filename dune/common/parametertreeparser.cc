// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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

Dune::ParameterTree Dune::ParameterTreeParser::readINITree(const std::string& file)
{
  std::ifstream in(file);

  if (!in)
    DUNE_THROW(Dune::IOError, "Could not open configuration file " << file);

  Dune::ParameterTree pt;
  readINITree(in, pt, "file '" + file + "'", true);
  return pt;
}

Dune::ParameterTree Dune::ParameterTreeParser::readINITree(std::istream& in)
{
  Dune::ParameterTree pt;
  readINITree(in, pt, "stream", true);
  return pt;
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
  std::string prefix;
  std::set<std::string> keysInFile;
  while(!in.eof())
  {
    std::string line;
    getline(in, line);
    line = ltrim(line);
    if (line.size() == 0)
      continue;
    switch (line[0]) {
    case '#' :
      break;
    case '[' :
      {
        size_t pos = line.find(']');
        if (pos != std::string::npos) {
          prefix = rtrim(ltrim(line.substr(1, pos-1)));
          if (prefix != "")
            prefix += ".";
        }
      }
      break;
    default :
      std::string::size_type comment = line.find("#");
      line = line.substr(0,comment);
      std::string::size_type mid = line.find("=");
      if (mid != std::string::npos)
      {
        std::string key = prefix+rtrim(ltrim(line.substr(0, mid)));
        std::string value = ltrim(line.substr(mid+1));

        if (value.length()>0)
        {
          // handle quoted strings
          if ((value[0]=='\'') || (value[0]=='"'))
          {
            char quote = value[0];
            value=value.substr(1);
            while (*(rtrim(value).rbegin())!=quote)
            {
              if (! in.eof())
              {
                std::string l;
                getline(in, l);
                value = value+"\n"+l;
              }
              else
                value = value+quote;
            }
            value = rtrim(value);
            value = value.substr(0,value.length()-1);
          }
          else
            value = rtrim(value);
        }

        if (keysInFile.count(key) != 0)
          DUNE_THROW(ParameterTreeParserError, "Key '" << key <<
                     "' appears twice in " << srcname << " !");
        else
        {
          if(overwrite || ! pt.hasKey(key))
            pt[key] = value;
          keysInFile.insert(key);
        }
      }
      break;
    }
  }

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
