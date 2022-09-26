// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_TEST_COLLECTORSTREAM_HH
#define DUNE_COMMON_TEST_COLLECTORSTREAM_HH

#include <sstream>
#include <string>
#include <functional>


#include <dune/common/typeutilities.hh>


namespace Dune {



/**
 * \brief Data collector stream
 *
 * A class derived from std::ostringstream that allows to
 * collect data via a temporary returned object. To facilitate
 * this it stores a callback that is used to pass the collected
 * data to its creator on destruction.
 *
 * In order to avoid passing the same data twice, copy construction
 * is forbidden and only move construction is allowed.
 */
class CollectorStream : public std::ostringstream
{
public:

  /**
   * \brief Create from callback
   *
   * \tparam CallBack Type of callback. Must be convertible to std::function<void(std::string)>
   * \param callBack A copy of this function will be stored and called on destruction.
   */
  template<class CallBack,
    Dune::disableCopyMove<CollectorStream, CallBack> = 0>
  CollectorStream(CallBack&& callBack) :
    callBack_(callBack)
  {}

  CollectorStream(const CollectorStream& other) = delete;

  /**
   * \brief Move constructor
   *
   * This will take over the data and callback from the
   * moved from CollectorStream and disable the callback
   * in the latter.
   */
  CollectorStream(CollectorStream&& other) :
    callBack_(other.callBack_)
  {
    (*this) << other.str();
    other.callBack_ = [](std::string){};
  }

  /**
   * \brief Destructor
   *
   * This calls the callback function given on creation
   * passing all collected data as a single string argument.
   */
  ~CollectorStream()
  {
    callBack_(this->str());
  }

private:
  std::function<void(std::string)> callBack_;
};



} // namespace Dune


#endif // DUNE_COMMON_TEST_COLLECTORSTREAM_HH
