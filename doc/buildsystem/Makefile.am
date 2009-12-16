# $Id: Makefile.am 4499 2006-04-13 14:56:35Z christi $

if BUILD_DOCS
  DOCS=buildsystem.pdf buildsystem.ps
endif

# setting like in dune-web
CURDIR=doc/buildsystem
# position of the web base directory,
# relative to $(CURDIR)
BASEDIR=..
EXTRAINSTALL = buildsystem.pdf

EXTRA_DIST = $(DOCS)

# install the html pages
buildsystemdir=$(datadir)/doc/dune-common/buildsystem
buildsystem_DATA = $(DOCS)

# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

include $(top_srcdir)/am/latex

# remove html pages on ``make  clean''
CLEANFILES = auto *.aux *.dvi *.log *.out *.toc

# PDFs and PSs are only clean in svn version not in the tarballs
SVNCLEANFILES = *.pdf *.ps
clean-local:
	if test -e $(top_srcdir)/$(CURDIR)/doxygen/Doxydep; then rm -rf $(SVNCLEANFILES) ; fi

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
