# $Id$

SUBDIRS = doc tutorial

SOURCES = allocator.hh basearray.hh bcrsmatrix.hh bvector.hh fmatrix.hh fvector.hh gsetc.hh io.hh istlexception.hh istl.hh precision.hh vbcrsmatrix.hh vbvector.hh

istldir = $(includedir)/dune/istl
istl_HEADERS = $(SOURCES)

## --- istl-only tarball stuff

# which documentation files should be put into special istl-only-tarball
DISTDOCS = doc/istl.tex doc/istl.bib doc/istl.ps doc/istl.pdf

# same for tutorial
DISTTUT = tutorial/example.cc

# more or less stolen from automake (a lot simpler though...)
istl-dist:
	(cd doc && $(MAKE))
	if test -e istl ; then echo Please remove `pwd`/istl!; exit 1; fi
	$(mkdir_p) istl
	cp --parents $(SOURCES) $(DISTDOCS) $(DISTTUT) istl
	$(AMTAR) chf - istl --format=posix | \
	  GZIP=$(GZIP_ENV) gzip -c > istl.tar.gz
	rm -r istl

CLEANFILES = istl.tar.gz

PHONY = istl-dist
