#include "penge-grid-view.h"

#include "penge-calendar-pane.h"
#include "penge-recent-files-pane.h"
#include "penge-apps-pane.h"
#include "penge-people-pane.h"

#include "penge-view-background.h"

G_DEFINE_TYPE (PengeGridView, penge_grid_view, NBTK_TYPE_TABLE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), PENGE_TYPE_GRID_VIEW, PengeGridViewPrivate))

typedef struct _PengeGridViewPrivate PengeGridViewPrivate;

struct _PengeGridViewPrivate {
  ClutterActor *calendar_pane;
  ClutterActor *recent_files_pane;
  ClutterActor *favourite_apps_pane;
  ClutterActor *people_pane;

  ClutterActor *background;
};

enum
{
  ACTIVATED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
penge_grid_view_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
penge_grid_view_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
penge_grid_view_dispose (GObject *object)
{
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
penge_grid_view_allocate (ClutterActor          *actor,
                          const ClutterActorBox *box,
                          gboolean               absolute_origin_changed)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (actor);
  ClutterActorBox child_box;

  /* Allocate the background to be the same area as the grid view */
  child_box.x1 = 0;
  child_box.y1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y2 = box->y2 - box->y1;
  clutter_actor_allocate (priv->background, &child_box, absolute_origin_changed);

  CLUTTER_ACTOR_CLASS (penge_grid_view_parent_class)->allocate (actor,
                                                                box,
                                                                absolute_origin_changed);
}

static void
penge_grid_view_class_init (PengeGridViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (PengeGridViewPrivate));

  object_class->get_property = penge_grid_view_get_property;
  object_class->set_property = penge_grid_view_set_property;
  object_class->dispose = penge_grid_view_dispose;
  object_class->finalize = penge_grid_view_finalize;

  actor_class->paint = penge_grid_view_paint;
  actor_class->allocate = penge_grid_view_allocate;

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
}

static void
penge_grid_view_init (PengeGridView *self)
{
  PengeGridViewPrivate *priv = GET_PRIVATE (self);

  priv->calendar_pane = g_object_new (PENGE_TYPE_CALENDAR_PANE,
                                      NULL);

  nbtk_table_add_actor (NBTK_TABLE (self),
                        priv->calendar_pane,
                        0,
                        0);

  priv->favourite_apps_pane = g_object_new (PENGE_TYPE_APPS_PANE,
                                            NULL);

  nbtk_table_add_actor (NBTK_TABLE (self),
                        priv->favourite_apps_pane,
                        1,
                        0);


  priv->recent_files_pane = g_object_new (PENGE_TYPE_RECENT_FILES_PANE, 
                                          NULL);

  nbtk_table_add_actor (NBTK_TABLE (self),
                        priv->recent_files_pane,
                        0,
                        1);
  clutter_container_child_set (CLUTTER_CONTAINER (self),
                               priv->recent_files_pane,
                               "row-span",
                               2,
                               NULL);


  priv->people_pane = g_object_new (PENGE_TYPE_PEOPLE_PANE,
                                    NULL);

  nbtk_table_add_actor (NBTK_TABLE (self),
                        priv->people_pane,
                        0,
                        2);
  clutter_container_child_set (CLUTTER_CONTAINER (self),
                               priv->people_pane,
                               "row-span",
                               2,
                               NULL);


  nbtk_table_set_row_spacing (NBTK_TABLE (self), 8);
  nbtk_table_set_col_spacing (NBTK_TABLE (self), 8);

  /* 
   * Create a background and parent it to the grid. We paint and allocate this
   * in the overridden vfuncs
   */
  priv->background = g_object_new (PENGE_TYPE_VIEW_BACKGROUND, NULL);
  clutter_actor_set_parent (priv->background, (ClutterActor *)self);
  clutter_actor_show (priv->background);
}

