// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sstream>
#include <fstream>
#include <set>
#include <algorithm>

#include <dune/common/exceptions.hh>
#include <dune/common/parametertree.hh>

using namespace Dune;

ParameterTree::ParameterTree()
{}

void ParameterTree::report(std::ostream& stream, const std::string& prefix) const
{
  typedef std::map<std::string, std::string>::const_iterator ValueIt;
  ValueIt vit = values.begin();
  ValueIt vend = values.end();

  for(; vit!=vend; ++vit)
    stream << vit->first << " = \"" << vit->second << "\"" << std::endl;

  typedef std::map<std::string, ParameterTree>::const_iterator SubIt;
  SubIt sit = subs.begin();
  SubIt send = subs.end();
  for(; sit!=send; ++sit)
  {
    stream << "[ " << prefix + sit->first << " ]" << std::endl;
    (sit->second).report(stream, prefix + sit->first + ".");
  }
}

bool ParameterTree::hasKey(const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    std::string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    const ParameterTree& s = sub(prefix);
    return s.hasKey(key.substr(dot+1));
  }
  else
    return (values.count(key) != 0);
}

bool ParameterTree::hasSub(const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    std::string prefix = key.substr(0,dot);
    if (subs.count(prefix) == 0)
      return false;

    const ParameterTree& s = sub(prefix);
    return s.hasSub(key.substr(dot+1));
  }
  else
    return (subs.count(key) != 0);
}

ParameterTree& ParameterTree::sub(const std::string& key)
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    ParameterTree& s = sub(key.substr(0,dot));
    return s.sub(key.substr(dot+1));
  }
  else
  {
    if (subs.count(key) == 0)
      subKeys.push_back(key.substr(0,dot));
    return subs[key];
  }
}

const ParameterTree& ParameterTree::sub(const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    const ParameterTree& s = sub(key.substr(0,dot));
    return s.sub(key.substr(dot+1));
  }
  else
  {
    if (subs.count(key) == 0)
      DUNE_THROW(Dune::RangeError, "Key '" << key << "' not found in ParameterTree");
    return subs.find(key)->second;
  }
}

std::string& ParameterTree::operator[] (const std::string& key)
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    if (not (hasSub(key.substr(0,dot))))
    {
      subs[key.substr(0,dot)];
      subKeys.push_back(key.substr(0,dot));
    }
    ParameterTree& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
  {
    if (not (hasKey(key)))
      valueKeys.push_back(key);
    return values[key];
  }
}

const std::string& ParameterTree::operator[] (const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    if (not (hasSub(key.substr(0,dot))))
      DUNE_THROW(Dune::RangeError, "Key '" << key << "' not found in ParameterTree");
    const ParameterTree& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
  {
    if (not (hasKey(key)))
      DUNE_THROW(Dune::RangeError, "Key '" << key << "' not found in ParameterTree");
    return values.find(key)->second;
  }
}

std::string ParameterTree::get(const std::string& key, const std::string& defaultValue) const
{
  if (hasKey(key))
    return (*this)[key];
  else
    return defaultValue;
}

std::string ParameterTree::get(const std::string& key, const char* defaultValue) const
{
  if (hasKey(key))
    return (*this)[key];
  else
    return defaultValue;
}


int ParameterTree::get(const std::string& key, int defaultValue) const
{
  std::stringstream stream;
  stream << defaultValue;
  std::string ret = get(key, stream.str());

  return atoi(ret.c_str());
}

double ParameterTree::get(const std::string& key, double defaultValue) const
{
  if(hasKey(key))
    return atof((*this)[key].c_str());
  else
    return defaultValue;
}

std::string ParameterTree::ltrim(const std::string& s)
{
  std::size_t firstNonWS = s.find_first_not_of(" \t\n\r");

  if (firstNonWS!=std::string::npos)
    return s.substr(firstNonWS);
  return std::string();
}

std::string ParameterTree::rtrim(const std::string& s)
{
  std::size_t lastNonWS = s.find_last_not_of(" \t\n\r");

  if (lastNonWS!=std::string::npos)
    return s.substr(0, lastNonWS+1);
  return std::string();
}

std::vector<std::string> ParameterTree::split(const std::string & s) {
  std::vector<std::string> substrings;
  std::size_t front = 0, back = 0, size = 0;

  while (front != std::string::npos)
  {
    // find beginning of substring
    front = s.find_first_not_of(" \t\n\r", back);
    back  = s.find_first_of(" \t\n\r", front);
    size  = back - front;
    if (size > 0)
      substrings.push_back(s.substr(front, size));
  }
  return substrings;
}

const ParameterTree::KeyVector& ParameterTree::getValueKeys() const
{
  return valueKeys;
}

const ParameterTree::KeyVector& ParameterTree::getSubKeys() const
{
  return subKeys;
}
