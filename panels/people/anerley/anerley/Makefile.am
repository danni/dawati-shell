noinst_LTLIBRARIES = libanerley.la

%.c: %.list Makefile.am
	$(QUIET_GEN)glib-genmarshal --body --prefix=anerley_marshal $< > $@

%.h: %.list Makefile.am
	$(QUIET_GEN)glib-genmarshal --header --prefix=anerley_marshal $< > $@

EXTRA_DIST = anerley-marshals.list
BUILT_SOURCES = anerley-marshals.c anerley-marshals.h

libanerley_la_HEADERS = anerley-tp-feed.h \
			anerley-feed.h \
			anerley-item.h \
			anerley-tile.h \
			anerley-simple-grid-view.h \
			anerley-feed-model.h \
			anerley-tile-view.h \
			anerley-tp-monitor-feed.h \
			anerley-presence-chooser.h \
			anerley-compact-tile.h \
			anerley-compact-tile-view.h \
			anerley-tp-user-avatar.h

libanerley_la_SOURCES = anerley-tp-feed.c \
			anerley-feed.c \
			anerley-item.c \
			anerley-tile.c \
			anerley-simple-grid-view.c \
			anerley-feed-model.c \
			anerley-tile-view.c \
			anerley-tp-monitor-feed.c \
			anerley-presence-chooser.c \
			anerley-tp-user-avatar.c \
			anerley-main.c \
			anerley-compact-tile.c \
			anerley-compact-tile-view.c \
			$(libanerley_la_HEADERS) \
			$(BUILT_SOURCES) \
			penge-magic-texture.c \
			penge-magic-texture.h
libanerley_ladir = $(includedir)/anerley/anerley

libanerley_la_CFLAGS = -I$(top_srcdir)/panels/people/anerley \
		       $(PANEL_PEOPLE_CFLAGS) \
		       -DPKG_DATA_DIR=\"$(pkgdatadir)\" \
		       -D_GNU_SOURCE
libanerley_la_LIBADD = $(PANEL_PEOPLE_LIBS)
