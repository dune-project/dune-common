// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __MISC_HH__
#define __MISC_HH__

//! Check condition at compilation time
template <bool flag> class CompileTimeChecker;

//! it exists only an implementation for true so the compiler throws an
//! error if the condition is false
template <> class CompileTimeChecker<true> { };


namespace Dune {
  /** @addtogroup Common

          @{
   */

  template<class T>
  T SQR (T t)
  {
    return t*t;
  }

  //********************************************************************
  //
  // generate filenames with timestep number in it
  //
  //********************************************************************

  template <typename T>
  inline const char *genFilename(T *path, T *fn, int ntime)
  {
    static char name[256];
    char         *cp;

    if (path == 0 || path[0] == '\0')
    {
      sprintf(name, "%s", fn);
    }
    else
    {
      const char *cp = path;
      while (*cp)
        cp++;
      cp--;
      if (*cp == '/')
        sprintf(name, "%s%s", path, fn);
      else
        sprintf(name, "%s/%s", path, fn);
    }
    cp = name;
    while (*cp)
      cp++;
    sprintf(cp, "%010d", ntime);

    return( (T *) name);
  }

  /** @} */

}


#endif
