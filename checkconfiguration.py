from __future__ import print_function, unicode_literals

import logging
import os
import subprocess

from . import module
from dune.generator.generator import builder

logger = logging.getLogger(__name__)

from dune.generator.builder import buffer_to_str

class ConfigurationError(Exception):
    '''raise this when there's a problem with the configuration of dune-py'''
    def __init__(self, error):
        Exception.__init__(self,error)


def have(identifier):
    '''check if an identifier is defined equal to 1 in the dune-py config.h file.
       use this to check if for example #define HAVE_DUNE_COMMON 1 is
       provided the config file by calling
       have("HAVE_DUNE_COMMON")
    '''
    config = os.path.join(module.get_dune_py_dir(),"config.h")

    s = [ line for line in open(config) if identifier in line]
    if not s.__len__() == 1:
        logger.info("checkconfiguration.have("+identifier+") failed - identifier not found in " + config)
        raise ConfigurationError(identifier + " is not set in dune-py's config.h")
    slist = s[0].split()
    # note that the correct line should be #define identifier 1 anything
    # else will throw an exception
    if not slist.__len__() == 3 or not slist[0] == "#define" or not slist[2] == "1":
        logger.critical("checkconfiguration.have("+identifier+") failed - only found "+s[0]+" in "+config)
        raise ConfigurationError(identifier + " is not set in dune-py's config.h")

def preprocessorTest(tests):
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
        source = source + t[0]+"\n#else\nexit("+str(i+1)+");\n#endif\n"
        i += 1
    source = source + "exit(0);\n}\n"#

    returncode, stdout, stderr = builder.compile(source, "generated_test")
    if returncode > 0:
        logger.error(buffer_to_str(stderr))
        raise builder.CompileError(buffer_to_str(stderr))


    test_args = ["./generated_test"]
    test = subprocess.Popen(test_args, cwd=builder.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = test.communicate()
    logger.debug(buffer_to_str(stdout))
    returncode = test.returncode

    if returncode > 0:
        logger.debug("failed testing:\n"+source)
        logger.critical("checking "+tests[returncode-1][0]+" failed: "+tests[returncode-1][1])
        raise ConfigurationError(tests[returncode-1][1])
