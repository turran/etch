AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

### Library

include src/lib/Makefile.mk

### Test

include src/bin/Makefile.mk

### Documentation

include doc/Makefile.mk

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
etch.pc.in \
doc/Doxyfile.in \
m4/ens_doxygen.m4

sed_process = \
$(SED) \
-e 's,@VERSION\@,$(VERSION),g' \
-e 's,@VMAJ\@,$(VMAJ),g' \
-e 's,@prefix\@,$(prefix),g' \
-e 's,@exec_prefix\@,$(exec_prefix),g' \
-e 's,@libdir\@,$(libdir),g' \
-e 's,@includedir\@,$(includedir),g' \
-e 's,@requirements_pc\@,$(requirements_pc),g' \
< $< > $@ || rm $@

pc_verbose = $(pc_verbose_@AM_V@)
pc_verbose_ = $(pc_verbose_@AM_DEFAULT_V@)
pc_verbose_0 = @echo "  PC      " $@;

etch.pc: $(top_srcdir)/etch.pc.in Makefile
	@rm -f $@
	$(pc_verbose)$(sed_process)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = etch.pc

clean-local: doc-clean
	-rm -rf etch.pc
