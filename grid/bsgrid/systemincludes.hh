// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __BSGRID_SYSTEMINCLUDES_HH__
#define __BSGRID_SYSTEMINCLUDES_HH__

// here we include all system header files, otherwise the we have to
// put the BernhardSchuppGrid namespace in every file of the original
// source and we dont want to do that. so the idea is first to include
// the system header and then include the original BernhardSchuppGrid
// sources within the namespace defined in bsinclude.hh

#include <ulimit.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include <rpc/rpc.h>

#ifdef _ANSI_HEADER
  #include <memory>
  #include <iostream>
  #include <fstream>
  #include <vector>
  #include <utility>
  #include <functional>
  #include <algorithm>
  #include <list>
  #include <iomanip>
  #include <strstream>
  #include <deque>
  #include <stack>
  #include <map>
#else
  #include <memory.h>
  #include <iostream.h>
  #include <fstream.h>
  #include <vector.h>
  #include <pair.h>
  #include <function.h>
  #include <algo.h>
  #include <list.h>
  #include <iomanip.h>
  #include <strstream.h>
  #include <deque.h>
  #include <stack.h>
  #include <map.h>
#endif



#endif
