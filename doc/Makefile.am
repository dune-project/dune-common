# $Id$

all: doxygen-tag

doxygen_DEPS = $(shell: cat $(DEPDIR)/doxygen.Po)
doxygen_FILES = Doxyfile doxy-header.html doxy-footer.html dune-doxy.css

$(DEPDIR)/doxygen.Po: Doxyfile
	./Doxydep $< $@

doxygen-tag: $(doxygen_FILES) $(doxygen_DEPS)
	doxygen >& doxygen.log
	touch doxygen-tag

WEBDIR = /var/www/dune/doxygen
WEBFILES = html/*.html html/*.css html/*.png doxygen.log

web-install: doxygen-tag
	if [ ! -d $(WEBDIR) ]; then \
	  install -d -g web -m 775 $(WEBDIR); \
	fi
	install -g web -m 664 $(WEBFILES) $(WEBDIR)

include $(DEPDIR)/doxygen.Po

clean-local:
	rm -rf html latex $(DEPDIR) doxygen-tag	*~
