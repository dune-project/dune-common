# $Id$

# distribute these files:
EXTRA_DIST = Buildsystem CodingStyles Doxyfile Doxydep \
  doxy-footer.html doxy-header.html dune-doxy.css mainpage

all: doxygen-tag

doxygen-tag: Doxyfile
@AMDEP_TRUE@	$(srcdir)/Doxydep $< $(DEPDIR)/doxygen.Po $@
	$(DOXYGEN) > doxygen.log 2>&1
	touch doxygen-tag

WEBDIR = /var/www/dune/doxygen
WEBFILES = html/*.html html/*.css html/*.png doxygen.log

web-install: doxygen-tag
	if [ ! -d $(WEBDIR) ]; then \
	  install -d -g web -m 775 $(WEBDIR); \
	fi
	install -g web -m 664 $(WEBFILES) $(WEBDIR)

@AMDEP_TRUE@DEP_FILES = $(DEPDIR)/doxygen.Po
@AMDEP_TRUE@@am__include@ @am__quote@$(DEPDIR)/doxygen.Po@am__quote@

clean-local:
	rm -rf html latex doxygen-tag *~
