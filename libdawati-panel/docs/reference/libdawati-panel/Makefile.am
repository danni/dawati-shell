## Process this file with automake to produce Makefile.in

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE=dawati-panel

# Uncomment for versioned docs and specify the version of the module, e.g. '2'.
#DOC_MODULE_VERSION=$(PLUGIN_VERSION)


# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

# Directories containing the source code, relative to $(srcdir).
DOC_SOURCE_DIR=../../../../libdawati-panel

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED"
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
# e.g. MKDB_OPTIONS=--sgml-mode --output-format=xml
MKDB_OPTIONS=--sgml-mode --output-format=xml --name-space=mpl

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-mkhtml
MKHTML_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS= \
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/glib \
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/gobject \
	--extra-dir=$(CLUTTER_PREFIX)/share/gtk-doc/html/clutter \
	--extra-dir=$(MX_PREFIX)/share/gtk-doc/html/mx


# Used for dependencies. The docs will be rebuilt if any of these change.
# e.g. HFILE_GLOB=$(top_srcdir)/gtk/*.h
# e.g. CFILE_GLOB=$(top_srcdir)/gtk/*.c
HFILE_GLOB= \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-version.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-content-pane.h         \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-entry.h		      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-icon-theme.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-client.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-clutter.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-common.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-gtk.h	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-windowless.h     \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-app-bookmark-manager.h \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-utils.h

CFILE_GLOB= \
	$(top_srcdir)/libdawati-panel/dawati-panel/mnb-enum-types.c	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-content-pane.c         \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-entry.c		      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-icon-theme.c	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-client.c	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-clutter.c	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-gtk.c	      \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-panel-windowless.c     \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-app-bookmark-manager.c \
	$(top_srcdir)/libdawati-panel/dawati-panel/mpl-utils.c

# Extra header to include when scanning, which are not under DOC_SOURCE_DIR
# e.g. EXTRA_HFILES=$(top_srcdir}/contrib/extra.h
EXTRA_HFILES=

# Header files to ignore when scanning. Use base file name, no paths
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h
IGNORE_HFILES= \
	marshal.h \
	mnb-enum-types.h \
	mnb-panel-dbus-bindings.h \
	mnb-panel-dbus-glue.h \
	mnb-toolbar-dbus-bindings.h \
	mnb-toolbar-dbus-glue.h \
	mpl-panel-background.h


# Images to copy into HTML directory.
# e.g. HTML_IMAGES=$(top_srcdir)/gtk/stock-icons/stock_about_24.png
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=		\
	cssfile.xml	\
	desktopfile.xml	\
	overview.xml

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=	\
	cssfile.xml	\
	desktopfile.xml	\
	overview.xml

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
# e.g. GTKDOC_CFLAGS=-I$(top_srcdir) -I$(top_builddir) $(GTK_DEBUG_FLAGS)
# e.g. GTKDOC_LIBS=$(top_builddir)/gtk/$(gtktargetlib)
INCLUDES=-I$(top_srcdir)/libdawati-panel $(LIBMPL_CFLAGS)
GTKDOC_LIBS=$(top_builddir)/libdawati-panel/dawati-panel/libdawati-panel.la $(LIBMPL_LIBS)

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST +=

# Files not to distribute
# for --rebuild-types in $(SCAN_OPTIONS), e.g. $(DOC_MODULE).types
# for --rebuild-sections in $(SCAN_OPTIONS) e.g. $(DOC_MODULE)-sections.txt
#DISTCLEANFILES +=

# Comment this out if you want your docs-status tested during 'make check'
if BUILD_GTK_DOC
#TESTS_ENVIRONMENT = cd $(srcsrc) &&
#TESTS = $(GTKDOC_CHECK)
endif
