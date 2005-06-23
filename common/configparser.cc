// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "configparser.hh"

#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>


using namespace dune;
using namespace std;

ConfigParser::ConfigParser()
{}

void ConfigParser::parseFile(string file)
{
  ifstream in(file.c_str());
  assert(in);

  string prefix;
  while(!in.eof())
  {
    string line;
    getline(in, line);
    line = trim(line);
    if (line[0] == '#')
    {}
    else if ((line[0] == '[')and (line[line.length()-1] == ']'))
      prefix = trim(line.substr(1, line.length()-2)) + ".";
    else
    {
      int mid = line.find("=");
      if (mid != string::npos)
      {
        string key = prefix+trim(line.substr(0, mid));
        string value = trim(line.substr(mid+1));

        (*this)[key] = value;
      }
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
  int dot = key.find(".");

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
  int dot = key.find(".");

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
  int dot = key.find(".");

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
  int dot = key.find(".");

  if (dot != string::npos)
  {
    ConfigParser& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
    return values[key];
}

string ConfigParser::get(const string& key, string defaultValue)
{
  if (hasKey(key))
    return (*this)[key];
  else
    return defaultValue;
}


string ConfigParser::get(const string& key, char* defaultValue)
{
  string s = defaultValue;

  return get(key, s);
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

string ConfigParser::trim(string s)
{
  int i = 0;
  while (s[i] == ' ')
    i++;

  s.erase(0,i);

  i = s.length();
  while (s[i-1] == ' ')
    i--;

  s.erase(i);
  return s;
}
