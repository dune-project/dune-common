# $Id$

# use several sub-libraries to build a big one
# the sub-libs are not installed later, they only exist for the build-process
SUBDIRS = io

# the main library
lib_LTLIBRARIES = libdune.la

# common code
libdune_la_SOURCES = example.cc gridwriter.cc amirameshwriter.cc

# sub-libraries
libdune_la_LIBADD = io/libduneio.la
