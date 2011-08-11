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
  std::string prefix;
  std::set<std::string> keysInFile;
  while(!in.eof())
  {
    std::string line;
    getline(in, line);
    line = ltrim(line);
    switch (line[0]) {
    case '#' :
      break;
    case '[' :
      line = rtrim(line);
      if (line[line.length()-1] == ']')
      {
        prefix = rtrim(ltrim(line.substr(1, line.length()-2)));
        if (prefix != "")
          prefix += ".";
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
          if ((value[0]=='\'')or (value[0]=='"'))
          {
            char quote = value[0];
            value=value.substr(1);
            while (*(rtrim(value).rbegin())!=quote)
            {
              if (not in.eof())
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
          DUNE_THROW(Exception, "Key '" << key <<
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
  std::string v = "";
  std::string k = "";

  for(int i=1; i<argc; i++)
  {
    std::string s(argv[i]);

    if ((argv[i][0]=='-') && (argv[i][1]!='\000'))
    {
      k = argv[i]+1;
      continue;
    }
    else
    {
      if (k.size())
        pt[k] = argv[i];
      k.clear();
    }

  }

}
