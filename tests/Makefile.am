include $(top_srcdir)/build/glib-tap.mk

AM_CFLAGS =                                                                \
	-g                                                                 \
	$(GLIB_CFLAGS)                                                     \
	$(MAINTAINER_CFLAGS)
### GNX_HOOK:LD_FLAGS

LDADD =                                                                    \
	$(top_builddir)/src/libgnx.la                                      \
	$(GLIB_LIBS)

AM_CPPFLAGS =                                                              \
	$(GNX_DEBUG_CFLAGS)                                                \
	-DG_LOG_DOMAIN=\"gnx\"                                             \
	-I$(top_srcdir)                                                    \
	-I$(top_builddir)                                                  \
	-I$(top_srcdir)/src                                                \
	-I$(top_builddir)/src                                              \
	$(NULL)
### GNX_HOOK:D_ALLOC_TEST

test_programs =                                                            \
	array                                                              \
	base                                                               \
	dict                                                               \
	heap                                                               \
	queue                                                              \
	util
