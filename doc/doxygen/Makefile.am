# $Id$

# distribute these files:
EXTRA_DIST = Doxyfile Doxydep \
  doxy-footer.html doxy-header.html dune-doxy.css mainpage

all: doxygen-tag

WEBDIR = /var/www/dune/doxygen
WEBFILES = html/*.html html/*.css html/*.png doxygen.log

# what files does the doxygen-generated stuff depend on (find-syntax)
DEPENDON = -name \*.cc -o -name \*.hh -o -name \*.css -o -name \*.html -o -name \*.png

# check dependency ourself to be robust
doxygen-tag: FORCE
	set -e; \
	if test ! -e doxygen-tag || test x"`find $(top_srcdir) \( $(DEPENDON) \) -a -cnewer doxygen-tag -print`" != x ; then \
	  echo Running doxygen. This may take a while... ; \
	  $(DOXYGEN) > doxygen.log 2>&1 <&- ; \
	  touch doxygen-tag ; \
	fi

# nice trick from the GNU make infopage to force a rule to run
FORCE:

web-install: doxygen-tag
	if [ ! -d $(WEBDIR) ]; then \
	  install -d -g web -m 775 $(WEBDIR); \
	fi
	install -g web -m 664 $(WEBFILES) $(WEBDIR)

clean-local:
	rm -rf html latex doxygen-tag *~
