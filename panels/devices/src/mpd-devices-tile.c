
/*
 * Copyright © 2010 Intel Corp.
 *
 * Authors: Rob Staudinger <robert.staudinger@intel.com>
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

#include <stdbool.h>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "mpd-default-device-tile.h"
#include "mpd-devices-tile.h"
#include "mpd-gobject.h"
#include "mpd-shell-defines.h"
#include "mpd-storage-device-tile.h"
#include "config.h"

G_DEFINE_TYPE (MpdDevicesTile, mpd_devices_tile, MX_TYPE_SCROLL_VIEW)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MPD_TYPE_DEVICES_TILE, MpdDevicesTilePrivate))

enum
{
  REQUEST_HIDE,
  REQUEST_SHOW,

  LAST_SIGNAL
};

typedef struct
{
  ClutterActor    *vbox;

  GVolumeMonitor  *monitor;
  GHashTable      *tiles;      /* key=GMount, value=MpdStorageTile */
} MpdDevicesTilePrivate;

static unsigned int _signals[LAST_SIGNAL] = { 0, };

static void
_eject_ready_cb (GDrive         *drive,
                 GAsyncResult   *result,
                 MpdDevicesTile *self)
{
  GError *error = NULL;

  g_debug ("%s()", __FUNCTION__);

  g_drive_eject_with_operation_finish (drive, result, &error);
  if (error)
  {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
  }
}

static int
_find_mount_cb (GMount      *mount,
                char const  *path)
{
  GFile *file;
  char  *mount_path;
  int    ret;

  file = g_mount_get_root (mount);
  mount_path = g_file_get_path (file);

  g_debug ("%s() %s", __FUNCTION__, mount_path);
  ret = g_strcmp0 (mount_path, path);

  g_free (mount_path);
  g_object_unref (file);

  return ret;
}

static void
_tile_eject_cb (MpdStorageDeviceTile  *tile,
                MpdDevicesTile        *self)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  char const  *path;
  GList       *mounts;
  GList       *iter;
  GDrive      *drive = NULL;

  path = mpd_storage_device_tile_get_mount_point (tile);
  g_debug ("%s() %s", __FUNCTION__, path);

  mounts = g_volume_monitor_get_mounts (priv->monitor);
  iter = g_list_find_custom (mounts, path, (GCompareFunc) _find_mount_cb);
  if (iter)
  {
    GMount *mount = G_MOUNT (iter->data);
    drive = g_mount_get_drive (mount);
  }

  g_list_foreach (mounts, (GFunc) g_object_unref, NULL);
  g_list_free (mounts);

  if (drive)
  {
    g_drive_eject_with_operation (drive,
                                  G_MOUNT_UNMOUNT_NONE,
                                  NULL,
                                  NULL,
                                  (GAsyncReadyCallback) _eject_ready_cb,
                                  tile);
    g_object_unref (drive);
  }
}

static void
_device_tile_request_hide_cb (ClutterActor    *tile,
                              MpdDevicesTile  *self)
{
  g_signal_emit_by_name (self, "request-hide");
}

static void
_device_tile_request_show_cb (ClutterActor    *tile,
                              MpdDevicesTile  *self)
{
  g_signal_emit_by_name (self, "request-show");
}

/* return TRUE for all the mounts we want to show */
static gboolean
_mount_is_wanted_device (GMount *mount)
{
  GDrive *drive;
  gboolean removable = TRUE;

  /* shadowed mounts are not supposed to be shown to user */
  if (g_mount_is_shadowed (mount)) {
    return FALSE;
  }

  /* we only want to show removable drives */
  drive = g_mount_get_drive (mount);
  if (drive) {
    removable = g_drive_is_media_removable (drive);
    g_object_unref (drive);
  }
  return removable;
}

static void
add_tile_from_mount (MpdDevicesTile *self,
                     GMount         *mount)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  GFile         *file;
  char          *path;
  GVolume       *volume;
  char          *name = NULL;
  char         **mime_types;
  GIcon         *icon;
  GtkIconInfo   *icon_info;
  char const    *icon_file;
  ClutterActor  *tile;
  GError        *error = NULL;

  volume = g_mount_get_volume (mount);
  if (volume) {
    name = g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_LABEL);
    g_object_unref (volume);
  }

  if (!name) {
    name = g_mount_get_name (mount);
  }

  /* Mount point */
  file = g_mount_get_root (mount);
  path = g_file_get_path (file);

  mime_types = g_mount_guess_content_type_sync (mount, false, NULL, &error);
  for (int i = 0; mime_types[i]; i++)
  {
    g_debug ("%s", mime_types[i]);
  }
  if (error)
  {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
  }

  /* Icon */
  icon = g_mount_get_icon (mount);
  icon_info = gtk_icon_theme_lookup_by_gicon (gtk_icon_theme_get_default (),
                                              icon,
                                              MPD_STORAGE_DEVICE_TILE_ICON_SIZE,
                                              GTK_ICON_LOOKUP_NO_SVG);
  icon_file = gtk_icon_info_get_filename (icon_info);
  g_debug ("%s() %s", __FUNCTION__, icon_file);

  tile = mpd_storage_device_tile_new (name, path, mime_types[0], icon_file);
  g_signal_connect (tile, "eject",
                    G_CALLBACK (_tile_eject_cb), self);
  g_signal_connect (tile, "request-hide",
                    G_CALLBACK (_device_tile_request_hide_cb), self);
  g_signal_connect (tile, "request-show",
                    G_CALLBACK (_device_tile_request_show_cb), self);
  mx_box_layout_add_actor (MX_BOX_LAYOUT (priv->vbox), tile, 0);

  g_hash_table_insert (priv->tiles, mount, tile);

  gtk_icon_info_free (icon_info);
  g_object_unref (icon);
  g_strfreev (mime_types);
  g_free (path);
  g_object_unref (file);
}

typedef struct
{
  char          *path;
  ClutterActor  *tile;
} find_tile_t;

static void
_find_tile_cb (ClutterActor *tile,
               find_tile_t  *find_tile)
{
  if (MPD_IS_STORAGE_DEVICE_TILE (tile))
  {
    char const *path = mpd_storage_device_tile_get_mount_point (
                        MPD_STORAGE_DEVICE_TILE (tile));
    g_debug ("%s() %s", __FUNCTION__, path);

    if (0 == g_strcmp0 (path, find_tile->path))
      find_tile->tile = CLUTTER_ACTOR (tile);
  }
}


static void
_add_mount_cb (GMount         *mount,
               MpdDevicesTile *self)
{
  if (_mount_is_wanted_device (mount)) {
    add_tile_from_mount (self, mount);
  }
  g_object_unref (mount);
}


static void
_monitor_mount_added_cb (GVolumeMonitor  *monitor,
                         GMount          *mount,
                         MpdDevicesTile  *self)
{
  if (_mount_is_wanted_device (mount)) {
    add_tile_from_mount (self, mount);
  }
}

static void
_monitor_mount_changed_cb (GVolumeMonitor *monitor,
                           GMount         *mount,
                           MpdDevicesTile *self)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  MpdStorageDeviceTile *tile;

  tile = g_hash_table_lookup (priv->tiles, mount);
  if (tile) {
    if (!_mount_is_wanted_device (mount)) {
      g_hash_table_remove (priv->tiles, mount);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->vbox),
                                      CLUTTER_ACTOR (tile));
    }
  } else {
    if (_mount_is_wanted_device (mount)) {
      add_tile_from_mount (self, mount);
    }
  }
}

static void
_monitor_mount_removed_cb (GVolumeMonitor  *monitor,
                           GMount          *mount,
                           MpdDevicesTile  *self)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  MpdStorageDeviceTile *tile;

  tile = g_hash_table_lookup (priv->tiles, mount);
  if (tile) {
    g_hash_table_remove (priv->tiles, mount);
    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->vbox),
                                    CLUTTER_ACTOR (tile));
  }
}


static void
_dispose (GObject *object)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (object);

  g_hash_table_destroy (priv->tiles);

  mpd_gobject_detach (object, (GObject **) &priv->monitor);

  G_OBJECT_CLASS (mpd_devices_tile_parent_class)->dispose (object);
}

static void
mpd_devices_tile_class_init (MpdDevicesTileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MpdDevicesTilePrivate));

  object_class->dispose = _dispose;

  /* Signals */

  _signals[REQUEST_HIDE] = g_signal_new ("request-hide",
                                         G_TYPE_FROM_CLASS (klass),
                                         G_SIGNAL_RUN_LAST,
                                         0, NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);

  _signals[REQUEST_SHOW] = g_signal_new ("request-show",
                                         G_TYPE_FROM_CLASS (klass),
                                         G_SIGNAL_RUN_LAST,
                                         0, NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);
}

static void
mpd_devices_tile_init (MpdDevicesTile *self)
{
  MpdDevicesTilePrivate *priv = GET_PRIVATE (self);
  ClutterActor  *tile;
  GList *mounts;

  priv->tiles = g_hash_table_new (g_direct_hash, g_direct_equal);

  priv->vbox = mx_box_layout_new ();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT (priv->vbox),
                                 MX_ORIENTATION_VERTICAL);
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->vbox);

  tile = mpd_default_device_tile_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->vbox), tile);

  priv->monitor = g_volume_monitor_get ();
  g_signal_connect (priv->monitor, "mount-added",
                    G_CALLBACK (_monitor_mount_added_cb), self);
  g_signal_connect (priv->monitor, "mount-changed",
                    G_CALLBACK (_monitor_mount_changed_cb), self);
  g_signal_connect (priv->monitor, "mount-removed",
                    G_CALLBACK (_monitor_mount_removed_cb), self);

  mounts = g_volume_monitor_get_mounts (priv->monitor);
  g_list_foreach (mounts, (GFunc) _add_mount_cb, self);
  g_list_free (mounts);
}

ClutterActor *
mpd_devices_tile_new (void)
{
  return g_object_new (MPD_TYPE_DEVICES_TILE, NULL);
}

