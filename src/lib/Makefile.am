MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@ETCH_BUILD@ \
@ETCH_CFLAGS@

lib_LTLIBRARIES = libetch.la

libetch_la_SOURCES = \
etch.c \
etch_animation.c \
etch_interpolator_argb.c \
etch_interpolator_string.c \
etch_interpolator_uint32.c \
etch_interpolator_int32.c \
etch_interpolator_float.c \
etch_interpolator_double.c

libetch_la_LIBADD = @ETCH_LIBS@ -lm

libetch_la_LDFLAGS = -no-undefined -version-info @version_info@
