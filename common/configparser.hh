// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __CONFIGPARSER__HH__
#define __CONFIGPARSER__HH__


#include <map>
#include <string>
#include <iostream>

using namespace std;


class Config
{
public:

  Config();

  void parseFile(string file);
  void parseCmd(int argc, char* argv []);

  bool Config::hasKey(const string& key);
  bool Config::hasSub(const string& key);

  string& operator[] (const string& key);

  void report() const;
  void report(const string prefix) const;

  Config& sub(const string& key);

  string get(const string& key, string defaultValue);
  string get(const string& key, char* defaultValue);
  int get(const string& key, int defaultValue);
  double get(const string& key, double defaultValue);
  bool get(const string& key, bool defaultValue);


private:
  map<string, string> values;
  map<string, Config> subs;
  string trim(string s);



};



#endif
