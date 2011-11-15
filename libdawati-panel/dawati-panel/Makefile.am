
AM_CPPFLAGS = \
	$(LIBMPL_CFLAGS) \
	-I$(top_srcdir)/libdawati-panel \
	-DMX_CACHE=\"$(MUTTER_DAWATI_THEME_DIR)/dawati-panel/mx.cache\" \
	-DDATADIR=\"$(datadir)\" \
	-DTHEMEDIR=\"$(MUTTER_DAWATI_THEME_DIR)/dawati-panel\" \
	-DDAWATI_APP_LAUNCHES_STORE=\"$(libexecdir)/dawati-app-launches-store\"

#
# Application launch utility
#
bin_PROGRAMS = dawati-launch

dawati_launch_LDADD = \
	$(LIBMPL_LIBS) \
	libdawati-panel.la

dawati_launch_SOURCES = \
	dawati-launch.c

#
# App launches store manipulation tool
#
libexec_PROGRAMS = dawati-app-launches-store

dawati_app_launches_store_LDADD = \
	$(LIBMPL_LIBS) \
	libdawati-panel.la

dawati_app_launches_store_SOURCES = \
	dawati-app-launches-store.c

#
# Dawati-panel library
#
lib_LTLIBRARIES = libdawati-panel.la

source_h = \
		$(srcdir)/mpl-app-launch-context.h \
		$(srcdir)/mpl-app-launches-query.h \
		$(srcdir)/mpl-app-launches-store.h\
		$(srcdir)/mpl-version.h			\
		$(srcdir)/mpl-content-pane.h \
		$(srcdir)/mpl-entry.h			\
		$(srcdir)/mpl-icon-theme.h		\
		$(srcdir)/mpl-panel-client.h		\
		$(srcdir)/mpl-panel-clutter.h		\
		$(srcdir)/mpl-panel-common.h		\
		$(srcdir)/mpl-panel-gtk.h		\
		$(srcdir)/mpl-panel-windowless.h	\
		$(srcdir)/mpl-app-bookmark-manager.h	\
		$(srcdir)/mpl-utils.h

private_h = \
		$(srcdir)/gdkapplaunchcontext-x11.h \
		$(srcdir)/mpl-app-launches-store-priv.h \
		$(srcdir)/mpl-panel-background.h

source_c = \
		$(srcdir)/gdkapplaunchcontext-x11.c \
		$(srcdir)/mpl-app-launch-context.c \
		$(srcdir)/mpl-app-launches-query.c \
		$(srcdir)/mpl-app-launches-store.c \
		$(srcdir)/mnb-enum-types.c		\
		$(srcdir)/mpl-content-pane.c \
		$(srcdir)/mpl-entry.c			\
		$(srcdir)/mpl-icon-theme.c		\
		$(srcdir)/mpl-panel-background.c	\
		$(srcdir)/mpl-panel-client.c		\
		$(srcdir)/mpl-panel-clutter.c		\
		$(srcdir)/mpl-panel-gtk.c		\
		$(srcdir)/mpl-panel-windowless.c	\
		$(srcdir)/mpl-app-bookmark-manager.c	\
		$(srcdir)/mpl-utils.c			\
		$(DBUS_GLUE)				\
		$(DBUS_BINDINGS)			\
		$(MARSHALS)

libdawati_panel_includedir = $(includedir)/libdawati-panel/dawati-panel
libdawati_panel_include_HEADERS = $(source_h)
libdawati_panel_la_SOURCES = $(source_c)

libdawati_panel_la_LDFLAGS = $(LIBMPL_LIBS)

DBUS_GLUE = $(srcdir)/mnb-panel-dbus-glue.h	\
	    $(srcdir)/mnb-toolbar-dbus-glue.h

DBUS_BINDINGS =	$(srcdir)/mnb-panel-dbus-bindings.h	\
		$(srcdir)/mnb-toolbar-dbus-bindings.h

%-glue.h: $(top_srcdir)/src/%.xml
	dbus-binding-tool --mode=glib-server --output=$@ --prefix=$(subst -,_,$*) $^

%-bindings.h: $(top_srcdir)/src/%.xml
	dbus-binding-tool --mode=glib-client --output=$@ --prefix=$(subst -,_,$*) $^

STAMPS = stamp-marshal.h stamp-mnb-enum-types.h

ENUMS = mnb-enum-types.h \
	mnb-enum-types.c

mnb-enum-types.h: stamp-mnb-enum-types.h
	@true
stamp-mnb-enum-types.h: $(source_h) $(private_h) Makefile
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
		--template $(top_srcdir)/src/mnb-enum-types.h.in \
	  $(source_h) $(private_h)) >> xgen-ceth && \
	(cmp -s xgen-ceth mnb-enum-types.h || cp xgen-ceth mnb-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

mnb-enum-types.c: mnb-enum-types.h
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
		--template $(top_srcdir)/src/mnb-enum-types.c.in \
	  $(source_h) $(private_h) ) >> xgen-cetc && \
	cp xgen-cetc mnb-enum-types.c && \
	rm -f xgen-cetc

MARSHALS = marshal.c marshal.h

marshal.h: stamp-marshal.h
	@true
stamp-marshal.h: $(top_srcdir)/src/marshal.list
	$(QUIET_GEN)$(GLIB_GENMARSHAL) \
		--prefix=dawati_netbook_marshal \
		--header \
	  $(top_srcdir)/src/marshal.list > xgen-cmh \
	&& (cmp -s xgen-cmh marshal.h || cp xgen-cmh marshal.h ) \
	&& rm -f xgen-cmh \
	&& echo timestamp > $(@F)

marshal.c: marshal.h Makefile
	$(QUIET_GEN)( echo "#include \"marshal.h\"" ; \
	  $(GLIB_GENMARSHAL) \
		--prefix=dawati_netbook_marshal \
		--body \
	  $(top_srcdir)/src/marshal.list --body ) >> xgen-cmc \
	&& cp xgen-cmc marshal.c \
	&& rm -f xgen-cmc

#%.c: $(top_srcdir)/src/%.list
#	echo '#include "marshal.h"' > $@ && \
#	$(GLIB_GENMARSHAL) --prefix=dawati_netbook_marshal $^ --body >> $@
#%.h: $(top_srcdir)/src/%.list
#	$(GLIB_GENMARSHAL) --prefix=dawati_netbook_marshal $^ --header > $@

BUILT_SOURCES = $(DBUS_GLUE) $(DBUS_BINDINGS) $(MARSHALS) $(ENUMS)

CLEANFILES = $(BUILT_SOURCES) $(STAMPS)

EXTRA_DIST= $(private_h)