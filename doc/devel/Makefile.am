# $Id$

if BUILD_DOCS
# only build html when wml is available
if WML
PAGES = codingstyle.html portability.html \
  tests.html features.html indexidsets.html
endif
endif

all: $(PAGES)

# place in website
BASEDIR=../..
CURDIR=doc/devel

# install the html pages
develdir=$(datadir)/doc/dune-common/devel
if BUILD_DOCS
devel_DATA = $(PAGES)
EXTRA_DIST= $(PAGES)
endif

include $(top_srcdir)/am/webstuff

# remove html pages on ``make  clean'' if we are not a tarball
SVNCLEANFILES = $(PAGES)
clean-local:
	if test -e $(top_srcdir)/doc/doxygen/Doxydep; then rm -rf $(SVNCLEANFILES); fi

include $(top_srcdir)/am/global-rules
