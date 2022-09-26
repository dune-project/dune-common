// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <ostream>
#include <string>

#include <dune/common/exceptions.hh>
#include <dune/common/ios_state.hh>
#include <dune/common/path.hh>

void setCode(int& code, bool status) {
  if(!status)
    code = 1;
  else
  if(code == 77)
    code = 0;
}

void concatPathsTests(int& code) {
  typedef const char* const triple[3];
  static const triple data[] = {
    {"a" , "b" , "a/b" },
    {"/a", "b" , "/a/b"},
    {"a/", "b" , "a/b" },
    {"a" , "b/", "a/b/"},
    {"..", "b" , "../b"},
    {"a" , "..", "a/.."},
    {"." , "b" , "./b" },
    {"a" , "." , "a/." },
    {""  , "b" , "b"   },
    {"a" , ""  , "a"   },
    {""  , ""  , ""    },
    {NULL, NULL, NULL  }
  };
  for(const triple* p = data; (*p)[0] != NULL; ++p) {
    const std::string& result = Dune::concatPaths((*p)[0], (*p)[1]);
    bool success = result == (*p)[2];
    setCode(code, success);
    if(!success)
      std::cerr << "concatPaths(\"" << (*p)[0] << "\", "
                << "\"" << (*p)[1] << "\"): got \"" << result << "\", "
                << "expected \"" << (*p)[2] << "\"" << std::endl;
  }
}

void processPathTests(int& code) {
  typedef const char* const pair[3];
  static const pair data[] = {
    {""      , ""     },
    {"."     , ""     },
    {"./"    , ""     },
    {"a/.."  , ""     },
    {".."    , "../"  },
    {"../a"  , "../a/"},
    {"a"     , "a/"   },
    {"a//"   , "a/"   },
    {"a///b" , "a/b/" },
    {"/"     , "/"    },
    {"/."    , "/"    },
    {"/.."   , "/"    },
    {"/a/.." , "/"    },
    {"/a"    , "/a/"  },
    {"/a/"   , "/a/"  },
    {"/../a/", "/a/"  },
    {NULL    , NULL   }
  };
  for(const pair* p = data; (*p)[0] != NULL; ++p) {
    const std::string& result = Dune::processPath((*p)[0]);
    bool success = result == (*p)[1];
    setCode(code, success);
    if(!success)
      std::cerr << "processPath(\"" << (*p)[0] << "\"): got "
                << "\"" << result << "\", expected "
                << "\"" << (*p)[1] << "\"" << std::endl;
  }
}

void prettyPathTests(int& code) {
  struct triple {
    const char* p;
    bool isDir;
    const char* result;
  };
  static const triple data[] = {
    {""      , true , "."    },
    {""      , false, "."    },
    {"."     , true , "."    },
    {"."     , false, "."    },
    {"./"    , true , "."    },
    {"./"    , false, "."    },
    {"a/.."  , true , "."    },
    {"a/.."  , false, "."    },
    {".."    , true , ".."   },
    {".."    , false, ".."   },
    {"../a"  , true , "../a/"},
    {"../a"  , false, "../a" },
    {"a"     , true , "a/"   },
    {"a"     , false, "a"    },
    {"a//"   , true , "a/"   },
    {"a//"   , false, "a"    },
    {"a///b" , true , "a/b/" },
    {"a///b" , false, "a/b"  },
    {"/"     , true , "/"    },
    {"/"     , false, "/"    },
    {"/."    , true , "/"    },
    {"/."    , false, "/"    },
    {"/.."   , true , "/"    },
    {"/.."   , false, "/"    },
    {"/a/.." , true , "/"    },
    {"/a/.." , false, "/"    },
    {"/a"    , true , "/a/"  },
    {"/a"    , false, "/a"   },
    {"/a/"   , true , "/a/"  },
    {"/a/"   , false, "/a"   },
    {"/../a/", true , "/a/"  },
    {"/../a/", false, "/a"   },
    {NULL,     false, NULL   }
  };

  Dune::ios_base_all_saver state(std::cerr);
  std::cerr << std::boolalpha;

  for(const triple* p = data; p->p != NULL; ++p) {
    const std::string& result = Dune::prettyPath(p->p, p->isDir);
    bool success = result == p->result;
    setCode(code, success);
    if(!success)
      std::cerr << "prettyPath(\"" << p->p << "\", " << p->isDir << "): got "
                << "\"" << result << "\", expected \"" << p->result << "\""
                << std::endl;
  }
}

void relativePathTests(int& code) {
  typedef const char* const triple[3];
  static const triple data[] = {
    {""  , ""  , ""     },
    {""  , "b" , "b/"   },
    {""  , "..", "../"  },
    {"a" , ""  , "../"  },
    {"a" , "b" , "../b/"},
    {"/" , "/" , ""     },
    {"/a", "/" , "../"  },
    {"/" , "/b", "b/"   },
    {"/a", "/b", "../b/"},
    {NULL, NULL, NULL   }
  };

  for(const triple* p = data; (*p)[0] != NULL; ++p) {
    const std::string& result = Dune::relativePath((*p)[0], (*p)[1]);
    bool success = result == (*p)[2];
    setCode(code, success);
    if(!success)
      std::cerr << "relativePath(\"" << (*p)[0] << "\", "
                << "\"" << (*p)[1] << "\"): got \"" << result << "\", "
                << "expected \"" << (*p)[2] << "\"" << std::endl;
  }

  typedef const char* const pair[2];
  static const pair except_data[] = {
    {""  , "/" },
    {"a" , "/" },
    {"/" , ""  },
    {"/" , "b" },
    {"..", ""  },
    {NULL, NULL}
  };

  for(const pair* p = except_data; (*p)[0] != NULL; ++p) {
    std::string result;
    try {
      result = Dune::relativePath((*p)[0], (*p)[1]);
    }
    catch(const Dune::NotImplemented&) {
      setCode(code, true);
      continue;
    }
    setCode(code, false);
    std::cerr << "relativePath(\"" << (*p)[0] << "\", "
              << "\"" << (*p)[1] << "\"): got \"" << result << "\", "
              << "expected exception thrown" << std::endl;
  }
}

int main ()
{
  try {
    int code = 77;

    concatPathsTests(code);
    processPathTests(code);
    prettyPathTests(code);
    relativePathTests(code);

    return code;
  }
  catch(const Dune::Exception& e) {
    std::cerr << "Exception thrown: " << e << std::endl;
    throw;
  }
}
