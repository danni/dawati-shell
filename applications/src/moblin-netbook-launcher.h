/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2008 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *         Thomas Wood <thomas@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef MOBLIN_NETBOOK_LAUNCHER_H
#define MOBLIN_NETBOOK_LAUNCHER_H

#include <glib.h>
#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MNB_TYPE_LAUNCHER mnb_launcher_get_type()

#define MNB_LAUNCHER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MNB_TYPE_LAUNCHER, MnbLauncher))

#define MNB_LAUNCHER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MNB_TYPE_LAUNCHER, MnbLauncherClass))

#define MNB_IS_LAUNCHER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MNB_TYPE_LAUNCHER))

#define MNB_IS_LAUNCHER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MNB_TYPE_LAUNCHER))

#define MNB_LAUNCHER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MNB_TYPE_LAUNCHER, MnbLauncherClass))

typedef struct MnbLauncher_ MnbLauncher;
typedef struct MnbLauncherClass_ MnbLauncherClass;
typedef struct MnbLauncherPrivate_ MnbLauncherPrivate;

struct MnbLauncher_ {
  NbtkBin              parent;
  MnbLauncherPrivate  *priv;
};

struct MnbLauncherClass_ {
  NbtkBinClass parent;

  /* Signals. */
  void (* launcher_activated) (MnbLauncher  *self,
                               const gchar  *desktop_file);
};

GType mnb_launcher_get_type (void) G_GNUC_CONST;


ClutterActor * mnb_launcher_new (gint width,
                                 gint height);

G_END_DECLS

#endif
