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

  parseStream(in, "file '" + file + "'");
}


void ConfigParser::parseStream(std::istream& in,
                               const std::string srcname)
{
  string prefix;
  set<string> keysInFile;
  while(!in.eof())
  {
    string line;
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
      string::size_type mid = line.find("=");
      if (mid != string::npos)
      {
        string key = prefix+rtrim(ltrim(line.substr(0, mid)));
        string value = ltrim(line.substr(mid+1));

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
                string l;
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
          (*this)[key] = value;
          keysInFile.insert(key);
        }
      }
      break;
    }
  }

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
    cout << vit->first << " = \"" << vit->second << "\"" << endl;

  typedef map<string, ConfigParser>::const_iterator SubIt;
  SubIt sit = subs.begin();
  SubIt send = subs.end();
  for(; sit!=send; ++sit)
  {
    cout << "[ " << prefix + sit->first << " ]" << endl;
    (sit->second).report(prefix + sit->first + ".");
  }
}

bool ConfigParser::hasKey(const string& key) const
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    const ConfigParser& s = sub(prefix);
    return s.hasKey(key.substr(dot+1));
  }
  else
    return (values.count(key) != 0);
}

bool ConfigParser::hasSub(const string& key) const
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    const ConfigParser& s = sub(prefix);
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

const ConfigParser& ConfigParser::sub(const string& key) const
{
  string::size_type dot = key.find(".");

  if (dot != string::npos)
  {
    const ConfigParser& s = sub(key.substr(0,dot));
    return s.sub(key.substr(dot+1));
  }
  else
    return subs.find(key)->second;
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
  if(hasKey(key))
    return atof((*this)[key].c_str());
  else
    return defaultValue;
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

string ConfigParser::ltrim(const string& s)
{
  std::size_t firstNonWS = s.find_first_not_of(" \t\n\r");

  if (firstNonWS!=string::npos)
    return s.substr(firstNonWS);
  return string();
}

string ConfigParser::rtrim(const string& s)
{
  std::size_t lastNonWS = s.find_last_not_of(" \t\n\r");

  if (lastNonWS!=string::npos)
    return s.substr(0, lastNonWS+1);
  return string();
}

const ConfigParser::KeyVector& ConfigParser::getValueKeys() const
{
  return valueKeys;
}

const ConfigParser::KeyVector& ConfigParser::getSubKeys() const
{
  return subKeys;
}
