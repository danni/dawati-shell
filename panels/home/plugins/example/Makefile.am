QUIET_GEN = $(Q:@=@echo '  GEN   '$@;)

PLUGIN = example

pluginsdir = $(datadir)/dawati-shell/plugins/$(PLUGIN)
plugins_DATA = \
        $(PLUGIN).js \
        $(PLUGIN).plugin \
        $(PLUGIN).png \
        $(NULL)

gsettings_SCHEMAS = \
        org.dawati.shell.home.plugin.$(PLUGIN).gschema.xml \
        $(NULL)

@GSETTINGS_RULES@
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
        $(plugins_DATA) \
        $(gsettings_SCHEMAS) \
        $(NULL)
