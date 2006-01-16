# $Id$

# distribute these files:
EXTRA_DIST = Doxyfile Doxydep \
  doxy-footer.html doxy-header.html dune-doxy.css mainpage

if BUILD_DOCS
  WHAT = bestpractice.html doxy-header.html doxygen-tag
endif

all: $(WHAT)

# setting like in dune-web
BASEDIR=../..
CURDIR=doc/doxygen
EXTRAINSTALL = doxygen.log
# EXTRAINSTALL in the html subdirectory
DOXYGENINSTALL = *.html *.css *.png *.gif

# what files does the doxygen-generated stuff depend on (find-syntax)
DEPENDON = -name \*.cc -o -name \*.hh -o -name \*.png

# check dependency ourself to be robust
doxygen-tag: FORCE
	set -e; \
	if test ! -e doxygen-tag || test x"`find $(top_srcdir) \( $(DEPENDON) \) -a -cnewer doxygen-tag -print`" != x ; then \
	  echo Running doxygen. This may take a while... ; \
	  $(DOXYGEN) > doxygen.log 2>&1 <&- ; \
	  touch doxygen-tag ; \
	fi

# how to install the doxygen documentation
doxygen-install:
	if test -d $(DUNEWEBDIR) && test -r $(DUNEWEBDIR)/Make.global ; then \
	  pushd html; \
	    $(MAKE) -f $(DUNEWEBDIR)/Make.global \
          EXTRAINSTALL="$(DOXYGENINSTALL)" CURDIR="$(CURDIR)/html" install ; \
	  popd; \
	fi

web-install: doxygen-install

# nice trick from the GNU make infopage to force a rule to run
FORCE:

include $(top_srcdir)/am/webstuff

clean-local:
	rm -rf html latex *~

CLEANFILES = $(WHAT)

include $(top_srcdir)/am/global-rules
