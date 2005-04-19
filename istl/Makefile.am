# $Id$

SUBDIRS = . doc tutorial test paamg

SOURCES = allocator.hh basearray.hh bcrsmatrix.hh bvector.hh gsetc.hh io.hh istlexception.hh istl.hh vbcrsmatrix.hh vbvector.hh ilu.hh operators.hh preconditioners.hh solvers.hh indexset.hh communicator.hh remoteindices.hh mpitraits.hh interface.hh

istldir = $(includedir)/dune/istl
istl_HEADERS = $(SOURCES)

## --- istl-only tarball stuff

# which documentation files should be put into special istl-only-tarball
DISTDOCS = doc/istl.tex doc/istl.bib doc/istl.ps doc/istl.pdf

# same for tutorial
DISTTUT = tutorial/example.cc

# parts from dune needed for istl (only common, please!)
DUNESTUFF = common/exceptions.hh common/fvector.hh common/fmatrix.hh \
	common/precision.hh common/timer.hh common/arraylist.hh \
	common/smartpointer.hh common/fixedarray.hh \
	common/poolallocator.hh common/iteratorfacades.hh common/sllist.hh \
	common/enumset.hh common/typetraits.hh common/logictraits.hh \
	common/alignment.hh

# these files are copied into the distribution but with .istl-dist stripped
RENAMESTUFF = README.istl-dist tutorial/Makefile.istl-dist

# more or less stolen from automake (a lot simpler though...)
istl-dist:
	(cd doc && $(MAKE))
	if test -e istl ; then echo Please remove `pwd`/istl!; exit 1; fi
	mkdir -p istl
	mkdir -p istl/dune
	cp --parents $(SOURCES) $(DISTDOCS) $(DISTTUT) istl
	(cd $(top_srcdir) && cp --parents $(DUNESTUFF) istl/istl/dune )
	for NAME in $(RENAMESTUFF) ; do \
	  cp $$NAME istl/`dirname $$NAME`/`basename $$NAME .istl-dist`; \
	done
	$(AMTAR) chf - istl --posix | \
	  GZIP=$(GZIP_ENV) gzip -c > istl.tar.gz
	rm -r istl

CLEANFILES = istl.tar.gz

.PHONY: istl-dist
