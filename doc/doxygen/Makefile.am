# $Id$

# distribute these files:
#DOXY_DIST = Doxyfile Doxydep \
#  doxy-footer.html doxy-header.html dune-doxy.css mainpage

if BUILD_DOCS
  DOXYGENTAG = doxygen-tag
  WHAT = bestpractice.html
endif

DISTCLEANFILES = doxygen.log doxygen-tag

all: $(WHAT) html

# setting like in dune-web
BASEDIR=../..
CURDIR=doc/doxygen
EXTRAINSTALL = doxygen.log
# EXTRAINSTALL in the html subdirectory
DOXYGENINSTALL = *.html *.css *.png *.gif

if WML
# themeing doxygen
DOXYGENHEADER = doxy-header.html
DOXYGENFOOTER = doxy-footer.html
DOXYGENSTYLESHEET = dune-doxy.css
DOYXFILE_OVERWRITE = echo "HTML_HEADER = $(DOXYGENHEADER)"; \
  echo "HTML_FOOTER = $(DOXYGENFOOTER)"; \
  echo "HTML_STYLESHEET = $(DOXYGENSTYLESHEET)";
endif


#we need this in the distribution
EXTRA_DIST = $(WHAT) html $(DOXYGENHEADER)

doxygendir = $(datadir)/doc/dune-common/doxygen
doxygen_DATA = $(WHAT)

install-data-local: html
	$(mkinstalldirs) $(DESTDIR)/$(doxygendir); \
	list="$(srcdir)/html/*.html $(srcdir)/html/*.css $(srcdir)/html/*.png $(srcdir)/html/*.gif"; \
	for p in $$list; do \
		$(install_sh_DATA) $$p $(DESTDIR)/$(doxygendir); \
	done

uninstall-local:
	rm -f $(DESTDIR)/$(doxygendir)/*.html $(DESTDIR)/$(doxygendir)/*.css \
	$(DESTDIR)/$(doxygendir)/*.png $(DESTDIR)/$(doxygendir)/*.gif

# what files does the doxygen-generated stuff depend on (find-syntax)
DEPENDON = -name \*.cc -o -name \*.hh -o -name \*.png

# as all doxygen files fill be installed in the html subdirectory
# the BASEDIR has to be adjusted
doxy-header.html:BASEDIR=../../..

# check dependency ourself to be robust
doxygen-tag: FORCE $(DOXYGENHEADER)
	set -e; \
	if test -e Doxyfile; then \
	if test ! -e doxygen-tag || test x"`find $(top_srcdir) \( $(DEPENDON) \) -a -cnewer doxygen-tag -print`" != x ; then \
	  echo Running doxygen. This may take a while... ; \
	  ( cat Doxyfile; $(DOYXFILE_OVERWRITE) ) > Doxyfile.tmp; \
	  $(DOXYGEN) Doxyfile.tmp > doxygen.log 2>&1 <&- ; \
	  rm -f Doxyfile.tmp; \
	  touch doxygen-tag ; \
	fi; \
	fi

html: $(DOXYGENTAG)

# how to install the doxygen documentation
doxygen-install:
	if test -d $(DUNEWEBDIR) && test -r $(DUNEWEBDIR)/Make.global ; then \
	  pushd html; \
	    $(MAKE) -f $(DUNEWEBDIR)/Make.global \
          EXTRAINSTALL="$(DOXYGENINSTALL)" CURDIR="$(CURDIR)/dune-common-html" install ; \
	  popd; \
	fi

web-install: doxygen-install

# nice trick from the GNU make infopage to force a rule to run
FORCE:

include $(top_srcdir)/am/webstuff

clean-local:
	rm -rf html latex *~

CLEANFILES = $(WHAT) $(DOXYGENHEADER)

include $(top_srcdir)/am/global-rules
