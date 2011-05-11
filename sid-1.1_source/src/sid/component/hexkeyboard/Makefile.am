## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

pkglib_LTLIBRARIES = libhexkeyboard.la

pkgdata_DATA = hw-hexkeyboard-1.txt

INCLUDES = -I. -I../../include -I$(srcdir)/../../include
libhexkeyboard_la_SOURCES = hexkeyboard.cxx
libhexkeyboard_la_LDFLAGS = -module -no-undefined

DEJAGNUTESTS = hexkeyboard.exp hexkeyboarddocs.exp hexmonkey.exp
check-local: all
	(cd ../testsuite ; $(MAKE) check RUNTESTFLAGS="$(RUNTESTFLAGS) $(DEJAGNUTESTS)" )

html_stylesheet=$(srcdir)/../component_html.xsl
include $(srcdir)/../../config/Makefile.docs
