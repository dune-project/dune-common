# SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

from io import StringIO
import os,sys
os.environ['DUNE_LOG_LEVEL'] = 'debug'
os.environ['DUNE_SAVE_BUILD'] = 'console'
from dune.common import FieldVector,TupleVector

tuple1 = (17.0, FieldVector([2,2]), 3.0, FieldVector([1, 2, 3]))

mtbv= TupleVector(tuple1)

for v in range(len(mtbv)-1): # block vector has no correct comparison by itself
    assert(mtbv[v] == tuple1[v])

mtbv[1]= FieldVector([2,5]) # check assignment of field vector to multi type block vector entry

# check that is really the same data address
assert(sys.getsizeof(mtbv)==7*8)

mtbv2= TupleVector(18.0, mtbv)# try nested TupleVectors
assert(mtbv2[0] == 18)
for v in range(len(mtbv2[1])-1): # block vector has no correct comparison by itself
    if v == 1:
        assert(mtbv2[1][v] == FieldVector([2,5])) # check if field vector was correctly assigned
    else:
        assert(mtbv2[1][v] == tuple1[v])


tuple3 = (34.0, FieldVector([4,10]), 6.0, FieldVector([2, 4, 6]))



for v in range(len(mtbv)-1):
    mtbv[v]+=mtbv[v]

for v in range(len(mtbv)-1): # block vector has no correct comparison by itself
    assert(mtbv[v] == tuple3[v])


mtbv3 = TupleVector(tuple1,allowByReference=True)

tuple1[1][0]= 5

assert(mtbv3[1][0] == 5) # check if field vector was correctly assigned by reference

try: #assignment of scalar to field vector should throw
    mtbv[3]= 2.0
except RuntimeError:
    pass


tuple4 = ( FieldVector([1,2,3]), FieldVector([1, 2]))

mtbv4 = TupleVector(tuple4,allowByReference=False)
mtbv5 = TupleVector(tuple4,allowByReference=True)


runCode="""
#include <dune/common/tuplevector.hh>
#include <dune/common/fvector.hh>
#include <iostream>
#include <dune/python/pybind11/cast.h>

double run(const Dune::TupleVector<Dune::FieldVector<double,3>,Dune::FieldVector<double,2>> &t) {
double res=0;
for (int i=0;i<3;i++) res+=std::get<0>(t)[i];
for (int i=0;i<2;i++) res+=std::get<1>(t)[i];
return res;
}

void run2( Dune::TupleVector<Dune::FieldVector<double,3>,Dune::FieldVector<double,2>> &t) {
std::get<0>(t)[0]=13;
}
"""
from dune.generator.algorithm import run
tup = (FieldVector([1, 2,3]), FieldVector([1, 2]))
assert (run("run",StringIO(runCode),tup)==9)

assert (run("run",StringIO(runCode),mtbv4)==9)
run("run2",StringIO(runCode),mtbv4)

assert (mtbv4[0][0]==13)

from dune.generator.exceptions import CompileError
try:
    run("run2",StringIO(runCode),tup)
except CompileError:
    pass
