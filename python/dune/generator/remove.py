import glob, os, sys, re, fileinput, shutil, datetime
import dune.common.module
dune_py_dir = dune.common.module.getDunePyDir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

def removeGenerated(modules = [], date=False):
    if len(modules) == 0:
        return

    moduleFiles = set()

    def rmJit(filename):
        fileBase = os.path.splitext(os.path.basename(filename))[0]
        print("Remove", fileBase)
        filePath, fileName = os.path.split(filename)
        os.remove( os.path.join(filePath,filename) )
        os.remove( os.path.join(filePath,fileBase+'.cc') )
        try:
            shutil.rmtree( os.path.join(filePath,"CMakeFiles",fileBase+".dir") )
        except:
            pass
        moduleFiles.update( [fileBase] )

    rmDate = None
    if not date:
        bases = []
        if 'all' in modules:
            bases += [os.path.join(generated_dir, '*.so')]
        else:
            for m in modules:
                bases += [os.path.join(generated_dir, m+'*.so')]
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
        # iterate over all files
        bases = [os.path.join(generated_dir, '*.so')]

    for base in bases:
        for filename in glob.iglob( base ):
            accessTime = datetime.datetime.fromtimestamp( os.path.getatime(filename) )
            if rmDate and rmDate < accessTime:
                continue
            rmJit(filename)

    for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt'), inplace = True):
        if not any( [m in line for m in moduleFiles] ):
             print(line, end="")
