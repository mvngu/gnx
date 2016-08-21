lib_LTLIBRARIES = libgnx.la

MAIN_HEADERFILE = gnx.h

GNX_HEADERFILES =                                                          \
	array.h                                                            \
	base.h                                                             \
	convert.h                                                          \
	dict.h                                                             \
	heap.h                                                             \
	io.h                                                               \
	query.h                                                            \
	queue.h                                                            \
	random.h                                                           \
	set.h                                                              \
	stack.h                                                            \
	type.h                                                             \
	util.h                                                             \
	visit.h
### GNX_HOOK:ALLOC_H

noinst_HEADERS =                                                           \
	sanity.h
### GNX_HOOK:NO_INSTALL_ALLOC_H
noinst_HEADERS += alloc.h

GNX_SRC =                                                                  \
	array.c                                                            \
	base.c                                                             \
	convert.c                                                          \
	dict.c                                                             \
	heap.c                                                             \
	io.c                                                               \
	query.c                                                            \
	queue.c                                                            \
	random.c                                                           \
	sanity.c                                                           \
	set.c                                                              \
	stack.c                                                            \
	util.c                                                             \
	visit.c
### GNX_HOOK:ALLOC_C

libgnx_la_SOURCES =                                                        \
	$(GNX_SRC)                                                         \
	$(MAIN_HEADERFILE)                                                 \
	$(GNX_HEADERFILES)

libgnx_la_CPPFLAGS =                                                       \
	-DG_LOG_DOMAIN=\"gnx\"                                             \
	-DGNX_COMPILATION                                                  \
	-I$(srcdir)
### GNX_HOOK:D_ALLOC_TEST

libgnx_la_CFLAGS = $(GLIB_CFLAGS)
### GNX_HOOK:LD_FLAGS

libgnx_la_LIBADD = $(GLIB_LIBS)

headerfilesdir = $(includedir)/gnx
headerfiles_HEADERS = $(MAIN_HEADERFILE)
gnxheaderfilesdir = $(headerfilesdir)
gnxheaderfiles_HEADERS = $(GNX_HEADERFILES)
