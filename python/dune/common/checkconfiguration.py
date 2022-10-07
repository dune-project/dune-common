# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import logging
import os
import re
import subprocess

import dune.common.module

from dune.common.utility import buffer_to_str
from dune.generator import ConfigurationError

logger = logging.getLogger(__name__)

def assertHave(identifier):
    '''check if an identifier is defined equal to 1 in the dune-py config.h file.
       use this to check if for example #define HAVE_DUNE_COMMON 1 is
       provided the config file by calling
       assertHave("HAVE_DUNE_COMMON")
    '''
    # the following simply will not work anymore - the issue is that the
    # approach used here requires dune-py to have already been setup before
    # calling 'assertHave' that is not guaranteed to be the case.
    # Possibly we can use the 'metadata' file instead.
    # We simply return for now
    raise RuntimeError("DEPRECATED USE OF assertHave")
    return
    config = os.path.join(dune.common.module.getDunePyDir(), "config.h")
    if not os.path.isfile(config):
        raise ConfigurationError("dune-py not configured yet")

    matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+1$', line) for line in open(config)] if match is not None]
    if not matches:
        matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+ENABLE', line) for line in open(config)] if match is not None]
    if not matches:
        raise ConfigurationError(identifier + " is not set in dune-py's config.h")
    elif matches.__len__() > 1:
        raise ConfigurationError(identifier + " found multiple times in dune-py's config.h")
def assertCMakeHave(identifier):
    '''check if an identifier is defined equal to 1 in the dune-py config.h file.
       use this to check if for example #define HAVE_DUNE_COMMON 1 is
       provided the config file by calling
       assertHave("HAVE_DUNE_COMMON")

       Note: note that this depends on a dune-py having been completely
       configured so should be used with caution, e.g., avoid usage in any
       code executed during import of a dune module
    '''
    config = os.path.join(dune.common.module.getDunePyDir(), "config.h")
    if not os.path.isfile(config):
        raise ConfigurationError("dune-py not configured yet")

    matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+1$', line) for line in open(config)] if match is not None]
    if not matches:
        matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+ENABLE', line) for line in open(config)] if match is not None]
    if not matches:
        raise ConfigurationError(identifier + " is not set in dune-py's config.h")
    elif matches.__len__() > 1:
        raise ConfigurationError(identifier + " found multiple times in dune-py's config.h")

def assertCMakeVariable(identifier,value,defaultFails):
    '''check if a variable in CMakeCache.txt in dune-py is defined and equal to 'value'
    '''
    cache = os.path.join(dune.common.module.getDunePyDir(), "CMakeCache.txt")

    identifier = identifier.lower().strip()
    matches = [line.lower() for line in open(cache) if re.match('^[ ]*'+identifier+':+', line.lower()) is not None]
    if not matches and defaultFails:
        raise ConfigurationError(identifier + " default behavior is used in dune-py and that is not allowed")
    elif len(matches) > 1:
        raise ConfigurationError(identifier + " found multiple times in dune-py's config.h")
    elif matches:
        # check for bool on/off type variables:
        bools = {True:["on","true","1"], False:["off","false","0"]}
        if not [x for x in bools[value] if x in matches[0]]:
            raise ConfigurationError(identifier + " dune-py wrongly configured wrt "+identifier)

def preprocessorAssert(tests):
    '''perform preprocessore checks.
       A list of checks can be provided each should contain a pair, the
       first being the preprocessor check to perform (e.g. #if or #ifdef)
       and the second being the message to print if the check fails. The
       generated code is of the form:
          tests[i][0]
          #else
          test failed
          #endif
       so the first argument of each test has to make this piece of code
       valid C++ code assuming config.h is included.
    '''
    source = "#include <config.h>\nint main() {\n"
    i = 0
    for t in tests:
        source = source + t[0]+"\n#else\nreturn "+str(i+1)+";\n#endif\n"
        i += 1
    source = source + "return 0;\n}\n"

    from dune.generator import builder
    with open(os.path.join(builder.generated_dir, "generated_module.hh"), 'w') as out:
        out.write(source)
    builder.compile("generated_test")

    test_args = ["./generated_test"]
    test = subprocess.Popen(test_args, cwd=builder.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = test.communicate()
    logger.debug(buffer_to_str(stdout))
    returncode = test.returncode

    if returncode > 0:
        logger.debug("failed testing:\n"+source)
        logger.critical("checking "+tests[returncode-1][0]+" failed: "+tests[returncode-1][1])
        raise ConfigurationError(tests[returncode-1][1])
