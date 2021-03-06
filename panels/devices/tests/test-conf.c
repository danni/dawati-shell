
/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Robert Staudinger <robert.staudinger@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <clutter/clutter.h>
#include "mpd-conf.h"

static void
print_brightness_enabled (MpdConf *conf)
{
  char const  *name = "brightness-enabled";
  bool         brightness_enabled;

  g_object_get (conf, name, &brightness_enabled, NULL);

  printf ("%s: %s\n", name, brightness_enabled ? "true" : "false");
}

static void
print_brightness_value (MpdConf *conf)
{
  char const  *name = "brightness-value";
  float        brightness_value;

  g_object_get (conf, name, &brightness_value, NULL);

  printf ("%s: %.2f\n", name, brightness_value);
}

static void
_conf_property_notify_cb (GObject     *object,
                          GParamSpec  *pspec,
                          MpdConf     *conf)
{
  char const *name;

  name = g_param_spec_get_name (pspec);
  if (0 == g_strcmp0 (name, "brightness-enabled"))
    print_brightness_enabled (conf);
  else if (0 == g_strcmp0 (name, "brightness-value"))
    print_brightness_value (conf);
  else
    g_warning ("%s : Unknown property \"%s\"", G_STRLOC, name);
}

int
main (int     argc,
      char  **argv)
{
  MpdConf *conf;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    {
      g_critical ("Could not initialize Clutter");
      return EXIT_FAILURE;
    }

  conf = mpd_conf_new ();

  print_brightness_enabled (conf);
  g_signal_connect (conf, "notify::brightness-enabled",
                    G_CALLBACK (_conf_property_notify_cb), conf);

  print_brightness_value (conf);
  g_signal_connect (conf, "notify::brightness-value",
                    G_CALLBACK (_conf_property_notify_cb), conf);

  clutter_main ();
  g_object_unref (conf);
  return EXIT_SUCCESS;
}

