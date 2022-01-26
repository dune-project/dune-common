import glob, os, sys, re, fileinput, shutil
import dune.common.module
dune_py_dir = dune.common.module.getDunePyDir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

def removeGenerated(modules = []):
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

    bases = []
    if 'all' in modules:
        bases += [os.path.join(generated_dir, '*.so')]
    else:
        for m in modules:
            bases += [os.path.join(generated_dir, m+'*.so')]

    for base in bases:
        for filename in glob.iglob( base ):
            rmJit(filename)

    for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt'), inplace = True):
        if not any( [m in line for m in moduleFiles] ):
             print(line, end="")
