// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "configparser.hh"

#include <cstdlib>

#include <string>
#include <sstream>
#include <fstream>
#include <set>

#include <dune/common/exceptions.hh>

using namespace Dune;
using namespace std;

ConfigParser::ConfigParser()
{}

void ConfigParser::parseFile(std::string file)
{
  ifstream in(file.c_str());

  if (!in)
    DUNE_THROW(IOError, "Could not open configuration file " << file);

  string prefix;
  set<string> keysInFile;
  while(!in.eof())
  {
    string line;
    getline(in, line);
    line = trim(line);
    switch (line[0]) {
    case '#' :
      break;
    case '[' :
      if (line[line.length()-1] == ']')
      {
        prefix = trim(line.substr(1, line.length()-2));
        if (prefix != "")
          prefix += ".";
      }
      break;
    default :
      string::size_type mid = line.find("=");
      if (mid != string::npos)
      {
        string key = prefix+trim(line.substr(0, mid));
        string value = trim(line.substr(mid+1));

        // handle quoted strings
        if (value.length()>1)
        {
          switch (value[0]) {
          case '\'' :
            if (value[value.length()-1] == '\'')
              value = value.substr(1, value.length()-2);
            break;
          case '"' :
            if (value[value.length()-1] == '"')
              value = value.substr(1, value.length()-2);
            break;
          default :
            break;
          }
        }

        if (keysInFile.count(key) != 0)
          DUNE_THROW(Exception, "Key '" << key << "' appears twice in file '" << file << "' !");
        else
        {
          (*this)[key] = value;
          keysInFile.insert(key);
        }
      }
      break;
    }
  }

  in.close();
  return;
}


void ConfigParser::parseCmd(int argc, char* argv [])
{
  string v = "";
  string k = "";

  for(int i=1; i<argc; i++)
  {
    string s(argv[i]);

    if ((argv[i][0]=='-') && (argv[i][1]!='\000'))
    {
      k = argv[i]+1;
      continue;
    }
    else
      (*this)[k] = argv[i];
  }

  return;
}

void ConfigParser::report() const
{
  report("");
}

void ConfigParser::report(const string prefix) const
{
  typedef map<string, string>::const_iterator ValueIt;
  ValueIt vit = values.begin();
  ValueIt vend = values.end();

  for(; vit!=vend; ++vit)
    cout << prefix + vit->first << " = " << vit->second << endl;

  typedef map<string, ConfigParser>::const_iterator SubIt;
  SubIt sit = subs.begin();
  SubIt send = subs.end();
  for(; sit!=send; ++sit)
  {
    cout << "[ " << prefix + sit->first << " ]" << endl;
    (sit->second).report(prefix + sit->first + ".");
  }
}

bool ConfigParser::hasKey(const string& key)
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    ConfigParser& s = sub(prefix);
    return s.hasKey(key.substr(dot+1));
  }
  else
    return (values.count(key) != 0);
}

bool ConfigParser::hasSub(const string& key)
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    ConfigParser& s = sub(prefix);
    return s.hasSub(key.substr(dot+1));
  }
  else
    return (subs.count(key) != 0);
}

ConfigParser& ConfigParser::sub(const string& key)
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    ConfigParser& s = sub(key.substr(0,dot));
    return s.sub(key.substr(dot+1));
  }
  else
    return subs[key];
}

string& ConfigParser::operator[] (const string& key)
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    if (not (hasSub(key.substr(0,dot))))
      subKeys.push_back(key.substr(0,dot));
    ConfigParser& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
  {
    if (not (hasKey(key)))
      valueKeys.push_back(key);
    return values[key];
  }
}

string ConfigParser::get(const string& key, const string& defaultValue)
{
  if (hasKey(key))
    return (*this)[key];
  else
    return defaultValue;
}

string ConfigParser::get(const string& key, const char* defaultValue)
{
  if (hasKey(key))
    return (*this)[key];
  else
    return defaultValue;
}


int ConfigParser::get(const string& key, int defaultValue)
{
  stringstream stream;
  stream << defaultValue;
  string ret = get(key, stream.str());

  return atoi(ret.c_str());
}

double ConfigParser::get(const string& key, double defaultValue)
{
  stringstream stream;
  stream << defaultValue;
  string ret = get(key, stream.str());

  return atof(ret.c_str());
}

bool ConfigParser::get(const string& key, bool defaultValue)
{
  stringstream stream;
  if (defaultValue)
    stream << 1;
  else
    stream << 0;

  string ret = get(key, stream.str());

  return (atoi(ret.c_str()) !=0 );
}

// This namespace here is needed to make the code compile...
namespace Dune {

  template<>
  string ConfigParser::get<string>(const string& key)
  {
    if (hasKey(key))
      return (*this)[key];

    DUNE_THROW(RangeError, "Key '" << key << "' not found in parameter file!");
  }

  template<>
  int ConfigParser::get<int>(const string& key)
  {
    if (hasKey(key))
      return std::atoi((*this)[key].c_str());

    DUNE_THROW(RangeError, "Key '" << key << "' not found in parameter file!");
  }

  template<>
  double ConfigParser::get<double>(const string& key)
  {
    if (hasKey(key))
      return std::atof((*this)[key].c_str());

    DUNE_THROW(RangeError, "Key '" << key << "' not found in parameter file!");
  }

  template<>
  bool ConfigParser::get<bool>(const string& key)
  {
    if (hasKey(key))
      return (std::atoi((*this)[key].c_str()) !=0 );

    DUNE_THROW(RangeError, "Key '" << key << "' not found in parameter file!");
  }


}  // end namespace Dune

string ConfigParser::trim(string s)
{
  int i = 0;
  while ((s[i] == ' ')or (s[i] == '\n') or (s[i] == '\r'))
    i++;

  s.erase(0,i);

  i = s.length();
  while ((s[i-1] == ' ')or (s[i-1] == '\n') or (s[i-1] == '\r'))
    i--;

  s.erase(i);
  return s;
}

const ConfigParser::KeyVector& ConfigParser::getValueKeys() const
{
  return valueKeys;
}

const ConfigParser::KeyVector& ConfigParser::getSubKeys() const
{
  return subKeys;
}
