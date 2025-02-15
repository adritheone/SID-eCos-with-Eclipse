# Minimal top-level Makefile.  Just pass everything to the $(CONFIGDIR)
# subdir.
# Tom Tromey <tromey@cygnus.com>

CONFIGDIR=unix


SHELL = /bin/sh
srcdir = .



all install install-binaries install-libraries install-minimal:
	@cd $(CONFIGDIR) && $(MAKE) $@

# Nothing for these yet.
installcheck install-info info:

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@cd $(CONFIGDIR) && $(MAKE) `echo $@ | sed 's/-recursive//'`

# Don't depend on configure.in, because we can't ensure that the user
# has autoconf.
configure:
	cd $(srcdir) ; autoconf

mostlyclean: mostlyclean-recursive

clean: clean-recursive

distclean-local:
	rm -f Makefile config.status config.cache config.log

distclean: distclean-recursive distclean-local

maintainer-clean: distclean-local maintainer-clean-recursive

Makefile: Makefile.in config.status
	CONFIG_FILES=Makefile CONFIG_HEADERS= $(SHELL) ./config.status

config.status: configure
	$(SHELL) config.status --recheck

#----------------------------------------------------------------
# These let the DejaGnu test suite run when DejaGnu isn't 
# installed yet, so run it from the srcdir and objdir.
#----------------------------------------------------------------
EXPECT = ` \
  if [ -f $${rootme}/../expect/expect ] ; then \
    echo $${rootme}/../expect/expect ; \
  else echo expect ; fi`

RUNTESTFLAGS =
RUNTEST = ` \
  if [ -f $(srcdir)/../dejagnu/runtest ] ; then \
    echo $(srcdir)/../dejagnu/runtest ; \
  else echo runtest ;  fi`

check:
	cd $(CONFIGDIR) && $(MAKE) tktest
	rootme=`pwd`; export rootme; \
	srcdir=${srcdir}; export srcdir ; \
	EXPECT=${EXPECT} ; export EXPECT ; \
	if [ -f $${rootme}/../expect/expect ] ; then  \
	   TCL_LIBRARY=`cd $${srcdir}/../tcl/library && pwd`; \
	   export TCL_LIBRARY; \
	   TK_LIBRARY=`cd $${srcdir}/library && pwd`; \
	   export TK_LIBRARY; \
	fi ; \
	$(RUNTEST) $(RUNTESTFLAGS) --tool tk --srcdir $(srcdir)/testsuite
