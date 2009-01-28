/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Moblin Netbook
 * Copyright © 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "mnb-switcher.h"
#include "moblin-netbook-ui.h"
#include "moblin-netbook.h"
#include <nbtk/nbtk-tooltip.h>

#define CHILD_DATA_KEY "MNB_SWITCHER_CHILD_DATA"
#define HOVER_TIMEOUT  800

static GQuark child_data_quark = 0;

G_DEFINE_TYPE (MnbSwitcher, mnb_switcher, MNB_TYPE_DROP_DOWN)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MNB_TYPE_SWITCHER, MnbSwitcherPrivate))

struct _MnbSwitcherPrivate {
  MutterPlugin *plugin;
  NbtkWidget   *table;

  gboolean      dnd_in_progress : 1;
};

struct input_data
{
  gint          index;
  MutterPlugin *plugin;
};

/*
 * Calback for clicks on a workspace in the switcher (switches to the
 * appropriate ws).
 */
static gboolean
workspace_input_cb (ClutterActor *clone, ClutterEvent *event, gpointer data)
{
  struct input_data *input_data = data;
  gint               indx       = input_data->index;
  MutterPlugin      *plugin     = input_data->plugin;
  MetaScreen        *screen     = mutter_plugin_get_screen (plugin);
  MetaWorkspace     *workspace;
  MoblinNetbookPluginPrivate *priv = MOBLIN_NETBOOK_PLUGIN (plugin)->priv;

  if (MNB_SWITCHER (priv->switcher)->priv->dnd_in_progress)
    return FALSE;

  workspace = meta_screen_get_workspace_by_index (screen, indx);

  if (!workspace)
    {
      g_warning ("No workspace specified, %s:%d\n", __FILE__, __LINE__);
      return FALSE;
    }

  clutter_actor_hide (priv->switcher);
  hide_panel (plugin);
  meta_workspace_activate (workspace, event->any.time);

  return FALSE;
}

struct child_data
{
  ClutterActor *self;
  MnbSwitcher  *switcher;
  MutterWindow *mw;
  guint         hover_timeout_id;
  ClutterActor *tooltip;
};

static struct child_data *
get_child_data (ClutterActor *child)
{
  struct child_data * child_data;


  child_data = g_object_get_qdata (G_OBJECT (child), child_data_quark);

  return child_data;
}

static struct child_data *
make_child_data (MnbSwitcher  *switcher,
		 MutterWindow *mw,
		 ClutterActor *actor,
		 const gchar  *text)
{
  struct child_data *child_data = g_new0 (struct child_data, 1);

  child_data->self = actor;
  child_data->switcher = switcher;
  child_data->mw = mw;
  child_data->tooltip = CLUTTER_ACTOR (nbtk_tooltip_new (actor, text));

  return child_data;
}

static void
free_child_data (struct child_data *child_data)
{
  g_object_set_qdata (G_OBJECT (child_data->self),
		      child_data_quark, NULL);

  if (child_data->hover_timeout_id)
    g_source_remove (child_data->hover_timeout_id);

  /*
   * Do not destroy the tooltip, this is happens automatically.
   */

  g_free (child_data);
}

static gboolean
workspace_switcher_clone_input_cb (ClutterActor *clone,
                                   ClutterEvent *event,
                                   gpointer      data)
{
  struct child_data          *child_data = data;
  MutterWindow               *mw = child_data->mw;
  MetaWindow                 *window;
  MetaWorkspace              *workspace;
  MetaWorkspace              *active_workspace;
  MetaScreen                 *screen;
  MnbSwitcher                *switcher = child_data->switcher;
  MutterPlugin               *plugin = switcher->priv->plugin;
  MoblinNetbookPluginPrivate *priv = MOBLIN_NETBOOK_PLUGIN (plugin)->priv;

  if (MNB_SWITCHER (priv->switcher)->priv->dnd_in_progress)
    return FALSE;

  window           = mutter_window_get_meta_window (mw);
  screen           = meta_window_get_screen (window);
  workspace        = meta_window_get_workspace (window);
  active_workspace = meta_screen_get_active_workspace (screen);

  clutter_actor_hide (CLUTTER_ACTOR (switcher));
  hide_panel (plugin);

  if (!active_workspace || (active_workspace == workspace))
    {
      meta_window_activate_with_workspace (window, event->any.time, workspace);
    }
  else
    {
      meta_workspace_activate_with_focus (workspace, window, event->any.time);
    }

  return FALSE;
}

static void
dnd_begin_cb (NbtkWidget   *table,
	      ClutterActor *dragged,
	      ClutterActor *icon,
	      gint          x,
	      gint          y,
	      gpointer      data)
{
  MnbSwitcherPrivate *priv = MNB_SWITCHER (data)->priv;
  struct child_data  *child_data = get_child_data (dragged);

  priv->dnd_in_progress = TRUE;

  if (child_data->hover_timeout_id)
    {
      g_source_remove (child_data->hover_timeout_id);
      child_data->hover_timeout_id = 0;
    }

  if (CLUTTER_ACTOR_IS_VISIBLE (child_data->tooltip))
    nbtk_tooltip_hide (NBTK_TOOLTIP (child_data->tooltip));

  clutter_actor_set_rotation (icon, CLUTTER_Y_AXIS, 60.0, 0, 0, 0);
  clutter_actor_set_opacity (dragged, 0x4f);
}

static void
dnd_end_cb (NbtkWidget   *table,
	    ClutterActor *dragged,
	    ClutterActor *icon,
	    gint          x,
	    gint          y,
	    gpointer      data)
{
  MnbSwitcherPrivate *priv = MNB_SWITCHER (data)->priv;

  priv->dnd_in_progress = FALSE;

  clutter_actor_set_rotation (icon, CLUTTER_Y_AXIS, 0.0, 0, 0, 0);
  clutter_actor_set_opacity (dragged, 0xff);
}

static void
dnd_dropped_cb (NbtkWidget   *table,
		ClutterActor *dragged,
		ClutterActor *icon,
		gint          x,
		gint          y,
		gpointer      data)
{
  ClutterChildMeta *meta;
  ClutterActor     *parent;
  ClutterActor     *table_actor = CLUTTER_ACTOR (table);
  MutterWindow     *mcw;
  MetaWindow       *mw;
  gint              col;

  if (!(mcw = get_child_data (dragged)->mw) ||
      !(mw = mutter_window_get_meta_window (mcw)))
    {
      g_warning ("No MutterWindow associated with this item.");
      return;
    }

  parent = clutter_actor_get_parent (table_actor);

  g_assert (NBTK_IS_TABLE (parent));

  meta = clutter_container_get_child_meta (CLUTTER_CONTAINER (parent),
					   table_actor);

  g_object_get (meta, "column", &col, NULL);

  /*
   * TODO -- perhaps we should expose the timestamp from the pointer event,
   * or event the entire Clutter event.
   */
  meta_window_change_workspace_by_index (mw, col, TRUE, CurrentTime);
}

static gboolean
clone_hover_timeout_cb (gpointer data)
{
  struct child_data  *child_data = data;
  MnbSwitcherPrivate *priv       = child_data->switcher->priv;

  if (!priv->dnd_in_progress)
    nbtk_tooltip_show (NBTK_TOOLTIP (child_data->tooltip));

  child_data->hover_timeout_id = 0;

  return FALSE;
}

static gboolean
clone_enter_event_cb (ClutterActor *actor,
		      ClutterCrossingEvent *event,
		      gpointer data)
{
  struct child_data  *child_data = data;
  MnbSwitcherPrivate *priv       = child_data->switcher->priv;

  if (!priv->dnd_in_progress)
    child_data->hover_timeout_id = g_timeout_add (HOVER_TIMEOUT,
						  clone_hover_timeout_cb,
						  data);

  return FALSE;
}

static gboolean
clone_leave_event_cb (ClutterActor *actor,
		      ClutterCrossingEvent *event,
		      gpointer data)
{
  struct child_data *child_data = data;

  if (child_data->hover_timeout_id)
    {
      g_source_remove (child_data->hover_timeout_id);
      child_data->hover_timeout_id = 0;
    }

  if (CLUTTER_ACTOR_IS_VISIBLE (child_data->tooltip))
    nbtk_tooltip_hide (NBTK_TOOLTIP (child_data->tooltip));

  return FALSE;
}

static void
mnb_switcher_show (ClutterActor *self)
{
  MnbSwitcherPrivate *priv = MNB_SWITCHER (self)->priv;
  MetaScreen   *screen = mutter_plugin_get_screen (priv->plugin);
  gint          ws_count, active_ws, ws_max_windows = 0;
  gint         *n_windows;
  gint          i, screen_width;
  NbtkWidget   *table;
  GList        *window_list, *l;
  NbtkWidget  **spaces;
  NbtkPadding   padding = MNB_PADDING (6, 6, 6, 6);

  /* create the contents */

  table = nbtk_table_new ();
  nbtk_table_set_row_spacing (NBTK_TABLE (table), 4);
  nbtk_table_set_col_spacing (NBTK_TABLE (table), 7);
  nbtk_widget_set_padding (table, &padding);

  clutter_actor_set_name (CLUTTER_ACTOR (table), "switcher-table");

  ws_count = meta_screen_get_n_workspaces (screen);
  active_ws = meta_screen_get_active_workspace_index (screen);

  mutter_plugin_query_screen_size (priv->plugin, &screen_width, &i);

  /* loop through all the workspaces, adding a label for each */
  for (i = 0; i < ws_count; i++)
    {
      NbtkWidget *ws_label;
      gchar *s;
      struct input_data *input_data = g_new (struct input_data, 1);
      input_data->index = i;
      input_data->plugin = priv->plugin;

      s = g_strdup_printf ("%d", i + 1);

      ws_label = nbtk_label_new (s);

      if (i == active_ws)
        clutter_actor_set_name (CLUTTER_ACTOR (ws_label), "workspace-title-active");

      nbtk_widget_set_style_class_name (ws_label, "workspace-title");

      clutter_actor_set_reactive (CLUTTER_ACTOR (ws_label), TRUE);

      g_signal_connect_data (ws_label, "button-press-event",
                             G_CALLBACK (workspace_input_cb), input_data,
                             (GClosureNotify) g_free, 0);

      nbtk_table_add_widget (NBTK_TABLE (table), ws_label, 0, i);
      clutter_container_child_set (CLUTTER_CONTAINER (table), CLUTTER_ACTOR (ws_label),
                                   "y-expand", FALSE, NULL);
    }

  /* iterate through the windows, adding them to the correct workspace */

  n_windows = g_new0 (gint, ws_count);
  spaces = g_new0 (NbtkWidget*, ws_count);
  window_list = mutter_plugin_get_windows (priv->plugin);
  for (l = window_list; l; l = g_list_next (l))
    {
      MutterWindow       *mw = l->data;
      ClutterActor       *texture, *clone;
      gint                ws_indx;
      MetaCompWindowType  type;
      gint                w, h;
      struct child_data  *child_data;
      MetaWindow         *meta_win = mutter_window_get_meta_window (mw);
      gchar              *title;

      ws_indx = mutter_window_get_workspace (mw);
      type = mutter_window_get_window_type (mw);
      /*
       * Only show regular windows that are not sticky (getting stacking order
       * right for sticky windows would be really hard, and since they appear
       * on each workspace, they do not help in identifying which workspace
       * it is).
       */
      if (ws_indx < 0                             ||
          mutter_window_is_override_redirect (mw) ||
          type != META_COMP_WINDOW_NORMAL)
        {
          continue;
        }

      /* create the table for this workspace if we don't already have one */
      if (!spaces[ws_indx])
        {
          struct input_data *input_data = g_new (struct input_data, 1);
          input_data->index = ws_indx;
          input_data->plugin = priv->plugin;

          spaces[ws_indx] = nbtk_table_new ();
          nbtk_table_set_row_spacing (NBTK_TABLE (spaces[ws_indx]), 6);
          nbtk_table_set_col_spacing (NBTK_TABLE (spaces[ws_indx]), 6);
          nbtk_widget_set_padding (spaces[ws_indx], &padding);
          nbtk_widget_set_style_class_name (NBTK_WIDGET (spaces[ws_indx]),
                                            "switcher-workspace");
          if (ws_indx == active_ws)
            clutter_actor_set_name (CLUTTER_ACTOR (spaces[ws_indx]),
                                    "switcher-workspace-active");

          nbtk_widget_set_dnd_threshold (spaces[ws_indx], 5);

          g_signal_connect (spaces[ws_indx], "dnd-begin",
                            G_CALLBACK (dnd_begin_cb), self);

          g_signal_connect (spaces[ws_indx], "dnd-end",
                            G_CALLBACK (dnd_end_cb), self);

          g_signal_connect (spaces[ws_indx], "dnd-dropped",
                            G_CALLBACK (dnd_dropped_cb), self);

          nbtk_table_add_widget (NBTK_TABLE (table), spaces[ws_indx], 1,
                                 ws_indx);

          /* switch workspace when the workspace is selected */
          g_signal_connect_data (spaces[ws_indx], "button-press-event",
                                 G_CALLBACK (workspace_input_cb), input_data,
                                 (GClosureNotify)g_free, 0);
        }

      texture = mutter_window_get_texture (mw);
      clone   = clutter_clone_texture_new (CLUTTER_TEXTURE (texture));

      clutter_actor_set_reactive (clone, TRUE);

      g_object_weak_ref (G_OBJECT (mw), switcher_origin_weak_notify, clone);
      g_object_weak_ref (G_OBJECT (clone), switcher_clone_weak_notify, mw);

      g_object_get (meta_win, "title", &title, NULL);
      child_data = make_child_data (MNB_SWITCHER (self), mw, clone, title);
      g_free (title);

      g_object_set_qdata (G_OBJECT (clone), child_data_quark, child_data);

      g_signal_connect_data (clone, "enter-event",
			     G_CALLBACK (clone_enter_event_cb), child_data,
			     (GClosureNotify)free_child_data, 0);
      g_signal_connect (clone, "leave-event",
                        G_CALLBACK (clone_leave_event_cb), child_data);

      n_windows[ws_indx]++;
      nbtk_table_add_actor (NBTK_TABLE (spaces[ws_indx]), clone,
                            n_windows[ws_indx], 0);
      clutter_container_child_set (CLUTTER_CONTAINER (spaces[ws_indx]), clone,
                                   "keep-aspect-ratio", TRUE, NULL);

      clutter_actor_get_size (clone, &h, &w);
      clutter_actor_set_size (clone, h/(gdouble)w * 80.0, 80);

      g_signal_connect (clone, "button-release-event",
                        G_CALLBACK (workspace_switcher_clone_input_cb),
			child_data);

      ws_max_windows = MAX (ws_max_windows, n_windows[ws_indx]);
    }

  /* add an "empty" message for empty workspaces */
  for (i = 0; i < ws_count; i++)
    {
      if (!spaces[i])
        {
          NbtkWidget *label;

          label = nbtk_label_new ("No applications on this space");

          nbtk_table_add_widget (NBTK_TABLE (table), label, 1, i);
        }
    }

  /*
   * Now create the new workspace column.
   */
  {
    NbtkWidget *new_ws = nbtk_table_new ();
    NbtkWidget *label;

    nbtk_table_set_row_spacing (NBTK_TABLE (new_ws), 6);
    nbtk_table_set_col_spacing (NBTK_TABLE (new_ws), 6);
    nbtk_widget_set_padding (new_ws, &padding);
    nbtk_widget_set_style_class_name (NBTK_WIDGET (new_ws),
                                      "switcher-workspace-new");

    label = nbtk_label_new ("+");
    nbtk_table_add_widget (NBTK_TABLE (new_ws), label, 0, 0);

    nbtk_table_add_widget (NBTK_TABLE (table), new_ws, 1, ws_count);
#if 0
    /*
     * FIXME -- inserting the ws into the 1st row and setting span to 2
     * rows screws up the layout.
     */
    nbtk_table_set_widget_rowspan (NBTK_TABLE (table), new_ws, 2);
#endif
  }

  g_free (spaces);
  g_free (n_windows);

  mnb_drop_down_set_child (MNB_DROP_DOWN (self),
                           CLUTTER_ACTOR (table));

  CLUTTER_ACTOR_CLASS (mnb_switcher_parent_class)->show (self);

  priv->table = table;

}

static void
mnb_switcher_hide (ClutterActor *self)
{
  MnbSwitcherPrivate *priv;

  g_return_if_fail (MNB_IS_SWITCHER (self));

  priv = MNB_SWITCHER (self)->priv;

  mnb_drop_down_set_child (MNB_DROP_DOWN (self),
                           NULL);
  CLUTTER_ACTOR_CLASS (mnb_switcher_parent_class)->hide (self);
}

static void
mnb_switcher_class_init (MnbSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  actor_class->show = mnb_switcher_show;
  actor_class->hide = mnb_switcher_hide;

  g_type_class_add_private (klass, sizeof (MnbSwitcherPrivate));

  child_data_quark = g_quark_from_static_string (CHILD_DATA_KEY);
}

static void
mnb_switcher_init (MnbSwitcher *self)
{
  self->priv = GET_PRIVATE (self);
}

NbtkWidget*
mnb_switcher_new (MutterPlugin *plugin)
{
  MnbSwitcher *switcher;

  g_return_val_if_fail (MUTTER_PLUGIN (plugin), NULL);

  switcher = g_object_new (MNB_TYPE_SWITCHER,
                           "show-on-set-parent", FALSE,
                           "reactive", TRUE, NULL);
  switcher->priv->plugin = plugin;

  return NBTK_WIDGET (switcher);
}


