
bin_PROGRAMS = src/bin/etch_test

src_bin_etch_test_SOURCES = \
src/bin/etch_test.c

src_bin_etch_test_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
@ETCH_CFLAGS@

src_bin_etch_test_LDADD = \
$(top_builddir)/src/lib/libetch.la \
@ETCH_LIBS@
