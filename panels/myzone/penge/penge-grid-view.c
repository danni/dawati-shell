/*
 * Copyright (C) 2008 - 2009 Intel Corporation.
 *
 * Author: Rob Bradford <rob@linux.intel.com>
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

#include <moblin-panel/mpl-panel-client.h>

#include "penge-grid-view.h"

#include "penge-calendar-pane.h"
#include "penge-everything-pane.h"
#include "penge-apps-pane.h"

#include "penge-view-background.h"

G_DEFINE_TYPE (PengeGridView, penge_grid_view, MX_TYPE_TABLE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), PENGE_TYPE_GRID_VIEW, PengeGridViewPrivate))

#define V_DIV_LINE THEMEDIR "/v-div-line.png"

typedef struct _PengeGridViewPrivate PengeGridViewPrivate;

struct _PengeGridViewPrivate {
  ClutterActor *calendar_pane;
  ClutterActor *favourite_apps_pane;
  ClutterActor *everything_pane;
  ClutterActor *background;
  MplPanelClient *panel_client;
};

enum
{
  ACTIVATED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum
{
  PROP_0,
  PROP_PANEL_CLIENT
};

static void
penge_grid_view_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PANEL_CLIENT:
      g_value_set_object (value, priv->panel_client);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
penge_grid_view_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PANEL_CLIENT:
      priv->panel_client = g_value_dup_object (value);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
penge_grid_view_dispose (GObject *object)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (object);

  if (priv->panel_client)
  {
    g_object_unref (priv->panel_client);
    priv->panel_client = NULL;
  }

  G_OBJECT_CLASS (penge_grid_view_parent_class)->dispose (object);
}

static void
penge_grid_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (penge_grid_view_parent_class)->finalize (object);
}

static void
penge_grid_view_paint (ClutterActor *actor)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (actor);

  /* Paint the background */
  clutter_actor_paint (priv->background);

  CLUTTER_ACTOR_CLASS (penge_grid_view_parent_class)->paint (actor);
}

static void
penge_grid_view_map (ClutterActor *actor)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (actor);

  CLUTTER_ACTOR_CLASS (penge_grid_view_parent_class)->map (actor);

  clutter_actor_map (priv->background);
}

static void
penge_grid_view_unmap (ClutterActor *actor)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (actor);

  CLUTTER_ACTOR_CLASS (penge_grid_view_parent_class)->unmap (actor);

  clutter_actor_unmap (priv->background);
}

static void
penge_grid_view_allocate (ClutterActor          *actor,
                          const ClutterActorBox *box,
                          ClutterAllocationFlags flags)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (actor);
  ClutterActorBox child_box;

  /* Allocate the background to be the same area as the grid view */
  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y2 = box->y2 - box->y1;
  clutter_actor_allocate (priv->background, &child_box, flags);

  CLUTTER_ACTOR_CLASS (penge_grid_view_parent_class)->allocate (actor,
                                                                box,
                                                                flags);
}

static void
penge_grid_view_class_init (PengeGridViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (PengeGridViewPrivate));

  object_class->get_property = penge_grid_view_get_property;
  object_class->set_property = penge_grid_view_set_property;
  object_class->dispose = penge_grid_view_dispose;
  object_class->finalize = penge_grid_view_finalize;

  actor_class->paint = penge_grid_view_paint;
  actor_class->allocate = penge_grid_view_allocate;
  actor_class->map = penge_grid_view_map;
  actor_class->unmap = penge_grid_view_unmap;

  signals[ACTIVATED_SIGNAL] =
    g_signal_new ("activated",
                  PENGE_TYPE_GRID_VIEW,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  pspec = g_param_spec_object ("panel-client",
                               "Panel client",
                               "The panel client",
                               MPL_TYPE_PANEL_CLIENT,
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PANEL_CLIENT, pspec);
}

static void
penge_grid_view_init (PengeGridView *self)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (self);
  ClutterActor *div_tex;
  GError *error = NULL;

#if 0
  priv->calendar_pane = g_object_new (PENGE_TYPE_CALENDAR_PANE,
                                      NULL);
  clutter_actor_set_width (priv->calendar_pane, 280);

  mx_table_add_actor (MX_TABLE (self),
                      priv->calendar_pane,
                      0,
                      0);
  clutter_container_child_set (CLUTTER_CONTAINER (self),
                               priv->calendar_pane,
                               "y-expand", TRUE,
                               "y-fill", FALSE,
                               "y-align", 0.0,
                               "x-expand", FALSE,
                               "x-fill", FALSE,
                               NULL);
#endif

  priv->favourite_apps_pane = g_object_new (PENGE_TYPE_APPS_PANE,
                                            NULL);

  mx_table_add_actor (MX_TABLE (self),
                      priv->favourite_apps_pane,
                      0,
                      0);
  clutter_container_child_set (CLUTTER_CONTAINER (self),
                               priv->favourite_apps_pane,
                               "y-expand", TRUE,
                               "y-fill", TRUE,
                               "x-expand", FALSE,
                               "x-fill", FALSE,
                               NULL);


  div_tex = clutter_texture_new_from_file (V_DIV_LINE, &error);

  if (!div_tex)
  {
    g_warning (G_STRLOC ": Error loading vertical divider: %s",
               error->message);
    g_clear_error (&error);
  } else {
    mx_table_add_actor (MX_TABLE (self),
                        div_tex,
                        0,
                        1);
    clutter_container_child_set (CLUTTER_CONTAINER (self),
                                 div_tex,
                                 "x-expand",
                                 FALSE,
                                 "x-fill",
                                 FALSE,
                                 NULL);
  }

  priv->everything_pane = g_object_new (PENGE_TYPE_EVERYTHING_PANE,
                                        NULL);

  mx_table_add_actor_with_properties (MX_TABLE (self),
                                      priv->everything_pane,
                                      0, 2,
                                      "x-expand", TRUE,
                                      "y-expand", TRUE,
                                      "x-fill", TRUE,
                                      "y-fill", TRUE,
                                      NULL);

  mx_table_set_row_spacing (MX_TABLE (self), 6);
  mx_table_set_col_spacing (MX_TABLE (self), 6);

  /* 
   * Create a background and parent it to the grid. We paint and allocate this
   * in the overridden vfuncs
   */
  priv->background = g_object_new (PENGE_TYPE_VIEW_BACKGROUND, NULL);
  clutter_actor_set_parent (priv->background, (ClutterActor *)self);
  clutter_actor_show (priv->background);
}

