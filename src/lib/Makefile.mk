
lib_LTLIBRARIES = src/lib/libetch.la

installed_headersdir = $(pkgincludedir)-$(VMAJ)
dist_installed_headers_DATA = \
src/lib/Etch.h

src_lib_libetch_la_SOURCES = \
src/lib/etch.c \
src/lib/etch_animation.c \
src/lib/etch_interpolator_argb.c \
src/lib/etch_interpolator_string.c \
src/lib/etch_interpolator_uint32.c \
src/lib/etch_interpolator_int32.c \
src/lib/etch_interpolator_float.c \
src/lib/etch_interpolator_double.c \
src/lib/etch_private.h

src_lib_libetch_la_CPPFLAGS = \
-DETCH_BUILD \
@ETCH_CFLAGS@

src_lib_libetch_la_LIBADD = \
@ETCH_LIBS@ \
-lm

src_lib_libetch_la_LDFLAGS = -no-undefined -version-info @version_info@
