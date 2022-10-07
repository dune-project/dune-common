// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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

const Dune::ParameterTree Dune::ParameterTree::empty_;

void ParameterTree::report(std::ostream& stream, const std::string& prefix) const
{
  typedef std::map<std::string, std::string>::const_iterator ValueIt;
  ValueIt vit = values_.begin();
  ValueIt vend = values_.end();

  for(; vit!=vend; ++vit)
    stream << vit->first << " = \"" << vit->second << "\"" << std::endl;

  typedef std::map<std::string, ParameterTree>::const_iterator SubIt;
  SubIt sit = subs_.begin();
  SubIt send = subs_.end();
  for(; sit!=send; ++sit)
  {
    stream << "[ " << prefix + prefix_ + sit->first << " ]" << std::endl;
    (sit->second).report(stream, prefix);
  }
}

bool ParameterTree::hasKey(const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    std::string prefix = key.substr(0,dot);
    if (subs_.count(prefix) == 0)
      return false;

    if (values_.count(prefix) > 0)
      DUNE_THROW(RangeError,"key " << prefix << " occurs as value and as subtree");

    const ParameterTree& s = sub(prefix);
    return s.hasKey(key.substr(dot+1));
  }
  else
    if (values_.count(key) != 0)
      {
        if (subs_.count(key) > 0)
          DUNE_THROW(RangeError,"key " << key << " occurs as value and as subtree");
        return true;
      }
    else
      return false;

}

bool ParameterTree::hasSub(const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    std::string prefix = key.substr(0,dot);
    if (subs_.count(prefix) == 0)
      return false;

    if (values_.count(prefix) > 0)
      DUNE_THROW(RangeError,"key " << prefix << " occurs as value and as subtree");

    const ParameterTree& s = sub(prefix);
    return s.hasSub(key.substr(dot+1));
  }
  else
    if (subs_.count(key) != 0)
      {
        if (values_.count(key) > 0)
          DUNE_THROW(RangeError,"key " << key << " occurs as value and as subtree");
        return true;
      }
    else
      return false;
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
    if (values_.count(key) > 0)
      DUNE_THROW(RangeError,"key " << key << " occurs as value and as subtree");
    if (subs_.count(key) == 0)
      subKeys_.push_back(key.substr(0,dot));
    subs_[key].prefix_ = prefix_ + key + ".";
    return subs_[key];
  }
}

const ParameterTree& ParameterTree::sub(const std::string& key, bool fail_if_missing) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    const ParameterTree& s = sub(key.substr(0,dot));
    return s.sub(key.substr(dot+1),fail_if_missing);
  }
  else
  {
    if (values_.count(key) > 0)
      DUNE_THROW(RangeError,"key " << key << " occurs as value and as subtree");
    if (subs_.count(key) == 0)
      {
        if (fail_if_missing)
          {
            DUNE_THROW(Dune::RangeError, "SubTree '" << key
                       << "' not found in ParameterTree (prefix " + prefix_ + ")");
          }
        else
          return empty_;
      }
    return subs_.find(key)->second;
  }
}

std::string& ParameterTree::operator[] (const std::string& key)
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    ParameterTree& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
  {
    if (! hasKey(key))
      valueKeys_.push_back(key);
    return values_[key];
  }
}

const std::string& ParameterTree::operator[] (const std::string& key) const
{
  std::string::size_type dot = key.find(".");

  if (dot != std::string::npos)
  {
    const ParameterTree& s = sub(key.substr(0,dot));
    return s[key.substr(dot+1)];
  }
  else
  {
    if (! hasKey(key))
      DUNE_THROW(Dune::RangeError, "Key '" << key
        << "' not found in ParameterTree (prefix " + prefix_ + ")");
    return values_.find(key)->second;
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
  return valueKeys_;
}

const ParameterTree::KeyVector& ParameterTree::getSubKeys() const
{
  return subKeys_;
}
