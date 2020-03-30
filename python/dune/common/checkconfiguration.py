import logging
import os
import re
import subprocess

import dune.common.module

from dune.common.utility import buffer_to_str
from dune.generator import builder, ConfigurationError

logger = logging.getLogger(__name__)

def assertHave(identifier):
    '''check if an identifier is defined equal to 1 in the dune-py config.h file.
       use this to check if for example #define HAVE_DUNE_COMMON 1 is
       provided the config file by calling
       assertHave("HAVE_DUNE_COMMON")
    '''
    config = os.path.join(dune.common.module.get_dune_py_dir(), "config.h")

    matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+1$', line) for line in open(config)] if match is not None]
    if not matches:
        matches = [match for match in [re.match('^[ ]*#define[ ]+' + identifier.strip() + '[ ]+ENABLE', line) for line in open(config)] if match is not None]

    if not matches:
        raise ConfigurationError(identifier + " is not set in dune-py's config.h")
    elif matches.__len__() > 1:
        raise ConfigurationError(identifier + " found multiple times in dune-py's config.h")

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
