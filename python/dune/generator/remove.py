# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import glob, os, sys, re, fileinput, shutil, datetime
import logging
import dune.common.module

logger = logging.getLogger(__name__)

dune_py_dir = dune.common.module.getDunePyDir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

def removeGenerated(modules = [], date=False):
    if len(modules) == 0:
        return

    moduleFiles = set()

    def rmJit(fileBase):
        # do not remove the entry belonging to the dune-py template
        if fileBase == "extractCompiler":
            return
        removed = False

        try:
            os.remove( os.path.join(generated_dir, fileBase+'.so') )
            removed = True
        except:
            pass

        try:
            os.remove( os.path.join(generated_dir, fileBase+'.cc') )
            removed = True
        except:
            pass

        try:
            shutil.rmtree( os.path.join(generated_dir, "CMakeFiles", fileBase+".dir") )
            removed = True
        except:
            pass

        if removed:
          logger.debug(f"Removed {fileBase}")

        moduleFiles.update( [fileBase] )

    bases = set()
    rmDate = None
    if not date:
        if 'all' in modules:
            modules = ['']
        for m in modules:
            files = []
            for ext in ('.so', '.cc'):
                pattern = os.path.join(generated_dir, m+'*'+ext)
                files += glob.glob(pattern)
            if len(files) == 0:
              bases.add(m)
            else:
              bases.update( [os.path.splitext(os.path.basename(f))[0] for f in files] )
    else:
        if not len(modules) == 1:
            raise ValueError("when removing modules by date only provide the date as argument and not a list of modules")
        # try to convert the provided date (modules[0]) into a date.
        # Possible formats
        formatExamples = ["19-Mar-2019", "19-03-2019", "2019-03-19", "19-Mar-19",
                          "older than number of days from today (int)"]
        formats        = ["%d-%b-%Y",    "%d-%m-%Y",   "%Y-%m-%d",   "%d-%b-%y"]
        for f in formats:
            try:
                rmDate = datetime.datetime.strptime(modules[0],f)
                break
            except ValueError:
                pass
        if not rmDate:
            try:
                rmDate = datetime.datetime.today() - datetime.timedelta(days=int(modules[0]))
            except:
                raise ValueError("could not read provided date - possible formats: "+
                                 ", ".join(formatExamples)) from None

        for filename in glob.iglob( os.path.join(generated_dir, '*.so') ):
            accessTime = datetime.datetime.fromtimestamp( os.path.getatime(filename) )
            if rmDate > accessTime:
                base = os.path.splitext(os.path.basename(filename))[0]
                bases.add( base )

    for base in bases:
        rmJit(base)

    # not sure what this was for but in the new builder version
    # this is not reasonable anymore.
    # Replace with something else?
    # for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt'), inplace = True):
    #     if not any( [m in line for m in moduleFiles] ):
    #          print(line, end="")
