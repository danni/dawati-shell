/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* mnb-toolbar-button.c */
/*
 * Copyright (c) 2009, 2010 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
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

#include "mnb-spinner.h"

G_DEFINE_TYPE (MnbSpinner, mnb_spinner, MX_TYPE_WIDGET);

#define MNB_SPINNER_GET_PRIVATE(obj)    \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MNB_TYPE_SPINNER, MnbSpinnerPrivate))

enum
{
  PROP_0 = 0,
};

struct _MnbSpinnerPrivate
{
  ClutterTimeline *timeline;

  ClutterColor   *bg_color;
  MxBorderImage  *mx_border_image;
  CoglHandle      border_image;

  guint            frame;
  guint            n_frames;

  gboolean         disposed : 1;
};

static CoglHandle template_material = NULL;

static void
mnb_spinner_dispose (GObject *object)
{
  MnbSpinnerPrivate *priv = MNB_SPINNER (object)->priv;

  if (priv->timeline)
    {
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }

  G_OBJECT_CLASS (mnb_spinner_parent_class)->dispose (object);
}

static void
mnb_spinner_finalize (GObject *object)
{
  MnbSpinnerPrivate *priv = MNB_SPINNER (object)->priv;

  if (priv->mx_border_image)
    g_boxed_free (MX_TYPE_BORDER_IMAGE, priv->mx_border_image);

  if (priv->border_image)
    cogl_handle_unref (priv->border_image);

  if (priv->bg_color)
    clutter_color_free (priv->bg_color);

  G_OBJECT_CLASS (mnb_spinner_parent_class)->finalize (object);
}

void
mnb_spinner_start (MnbSpinner *spinner)
{
  MnbSpinnerPrivate *priv = MNB_SPINNER (spinner)->priv;

  priv->frame = 0;

  if (priv->timeline)
    clutter_timeline_start (priv->timeline);
}

void
mnb_spinner_stop (MnbSpinner *spinner)
{
  MnbSpinnerPrivate *priv = MNB_SPINNER (spinner)->priv;

  if (priv->timeline)
    clutter_timeline_stop (priv->timeline);
}

static void
mnb_spinner_allocate (ClutterActor          *actor,
                      const ClutterActorBox *box,
                      ClutterAllocationFlags flags)
{
  /* MnbSpinnerPrivate *priv = MNB_SPINNER (actor)->priv; */

  CLUTTER_ACTOR_CLASS (
                       mnb_spinner_parent_class)->allocate (actor,
                                                            box,
                                                            flags);
}

static void
mnb_spinner_set_property (GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  /* MnbSpinnerPrivate *priv = MNB_SPINNER (gobject)->priv; */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mnb_spinner_get_property (GObject    *gobject,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  /* MnbSpinnerPrivate *priv = MNB_SPINNER (gobject)->priv; */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mnb_spinner_get_preferred_width (ClutterActor *self,
                                 gfloat        for_height,
                                 gfloat       *min_width_p,
                                 gfloat       *natural_width_p)
{
  MxWidget   *widget = MX_WIDGET (self);
  CoglHandle  background;

  if ((background = mx_widget_get_background_texture (widget)))
    {
      gint tx_h;

      /*
       * The background texture is a strip of squares making up the individual
       * frames in the animation, so the width matches the height of the
       * texture.
       */
      tx_h = cogl_texture_get_height (background);

      if (min_width_p)
        *min_width_p = tx_h;

      if (natural_width_p)
        *natural_width_p = tx_h;

      return;
    }

  if (min_width_p)
    *min_width_p = 0.0;

  if (natural_width_p)
    *natural_width_p = 0.0;
}

static void
mnb_spinner_get_preferred_height (ClutterActor *self,
                                  gfloat        for_width,
                                  gfloat       *min_height_p,
                                  gfloat       *natural_height_p)
{
  MxWidget   *widget = MX_WIDGET (self);
  CoglHandle  background;

  if ((background = mx_widget_get_background_texture (widget)))
    {
      gint tx_h;

      /*
       * The background texture is a strip of squares making up the individual
       * frames in the animation, so the width matches the height of the
       * texture.
       */
      tx_h = cogl_texture_get_height (background);

      if (min_height_p)
        *min_height_p = tx_h;

      if (natural_height_p)
        *natural_height_p = tx_h;

      return;
    }

  if (min_height_p)
    *min_height_p = 0.0;

  if (natural_height_p)
    *natural_height_p = 0.0;
}

static void
mnb_spinner_pick (ClutterActor *self, const ClutterColor *color)
{
  /* We never pick this actor */
}

static void
mnb_spinner_paint (ClutterActor *self)
{
  MnbSpinnerPrivate *priv   = MNB_SPINNER (self)->priv;
  MxWidget          *widget = MX_WIDGET (self);
  CoglHandle         background;
  gfloat             width, height;
  ClutterActorBox    allocation;
  guint              alpha = clutter_actor_get_paint_opacity (self);

  clutter_actor_get_allocation_box (self, &allocation);

  width = allocation.x2 - allocation.x1;
  height = allocation.y2 - allocation.y1;

  /* paint the background color first */
  if (priv->bg_color && priv->bg_color->alpha != 0)
    {
      guint tmp_alpha = alpha * priv->bg_color->alpha / 255;

      cogl_set_source_color4ub (priv->bg_color->red,
                                priv->bg_color->green,
                                priv->bg_color->blue,
                                tmp_alpha);
      cogl_rectangle (0, 0, width, height);
    }

  /*
   * This paints border-image.
   */
  if (priv->border_image)
    mx_texture_frame_paint_texture (priv->border_image,
                                    alpha,
                                    priv->mx_border_image->top,
                                    priv->mx_border_image->right,
                                    priv->mx_border_image->bottom,
                                    priv->mx_border_image->left,
                                    width, height);

  if ((background = mx_widget_get_background_texture (widget)))
    {
      gfloat          tf_x, tf_y, tf_w, tf_h;
      guint8          opacity;
      ClutterActorBox box = { 0, };
      CoglHandle      material;

      /* setup the template material */
      if (!template_material)
        template_material = cogl_material_new ();

      /* create the material and apply opacity */
      material = cogl_material_copy (template_material);

      opacity = clutter_actor_get_paint_opacity (self);

      if (opacity == 0)
        return;

      cogl_material_set_color4ub (material, opacity, opacity, opacity, opacity);

      /* add the texture */
      cogl_material_set_layer (material, 0, background);

      clutter_actor_get_allocation_box (self, &box);

      tf_x = (gfloat)priv->frame / (gfloat) priv->n_frames;
      tf_y = 0.0;
      tf_w = tf_x + 1.0 / priv->n_frames;
      tf_h = 1.0;

      /* Paint will have translated us */
      cogl_set_source (material);
      cogl_rectangle_with_texture_coords (0.0, 0.0,
                                          box.x2 - box.x1,
                                          box.y2 - box.y1,
                                          tf_x, tf_y, tf_w, tf_h);

      cogl_handle_unref (material);
    }
}
static void
mnb_spinner_marker_reached_cb (ClutterTimeline *timeline,
                               const gchar     *marker,
                               gint             time,
                               MnbSpinner      *spinner)
{
  MnbSpinnerPrivate *priv = spinner->priv;

  priv->frame++;

  if (priv->frame >= priv->n_frames)
    priv->frame = 0;

  clutter_actor_queue_redraw ((ClutterActor *) spinner);
}

static void
_mnb_spinner_style_changed (MnbSpinner *self, gpointer data)
{
  MnbSpinnerPrivate *priv = self->priv;
  MxTextureCache *texture_cache = mx_texture_cache_get_default ();

  if (priv->mx_border_image)
    {
      g_boxed_free (MX_TYPE_BORDER_IMAGE, priv->mx_border_image);
      priv->mx_border_image = NULL;
    }
  if (priv->border_image)
    {
      cogl_handle_unref (priv->border_image);
      priv->border_image = NULL;
    }
  if (priv->bg_color)
    {
      clutter_color_free (priv->bg_color);
      priv->bg_color = NULL;
    }

  mx_stylable_get (MX_STYLABLE (self),
                   "border-image", &priv->mx_border_image,
                   "background-color", &priv->bg_color,
                   NULL);
  if (priv->mx_border_image)
    priv->border_image = mx_texture_cache_get_cogl_texture (texture_cache,
                                                            priv->mx_border_image->uri);
}

static void
mnb_spinner_constructed (GObject *self)
{
  MnbSpinnerPrivate *priv = MNB_SPINNER (self)->priv;
  MxWidget          *widget = MX_WIDGET (self);
  CoglHandle         background;
  ClutterTimeline   *timeline;

  /*
   * Mx does not seem to load the style info until the first show, but we want
   * to get hold of the background asset here to work out the frame count, so
   * we need to force the style loading.
   *
   * NB: mx_widget_ensure_style() does not work here, because the MxWidget
   *     is_style_dirty flag is cleared at this point.
   */
  mx_stylable_style_changed (MX_STYLABLE (widget), MX_STYLE_CHANGED_FORCE);

  if ((background = mx_widget_get_background_texture (widget)))
    {
      gint  tx_w, tx_h;
      guint duration;

      /*
       * The background texture is a strip of squares making up the individual
       * frames in the animation, so the width matches the height of the
       * texture.
       */
      tx_w = cogl_texture_get_width (background);
      tx_h = cogl_texture_get_height (background);

      priv->n_frames = tx_w / tx_h;

      if (tx_w % tx_h)
        g_warning (G_STRLOC ": Expected texture size %d x %d, got %d x %d",
                   tx_h * priv->n_frames, tx_h, tx_w, tx_h);

      /*
       * Setup a looped timeline with a marker that fires everytime we should
       * advance to a new frame.
       *
       * Assume the whole animation is to last 1s.
       */
      duration = 1000/ priv->n_frames;

      timeline = priv->timeline = clutter_timeline_new (duration);

      clutter_timeline_set_loop (timeline, TRUE);
      clutter_timeline_add_marker_at_time (timeline, "next", duration);
      clutter_timeline_stop (timeline);

      g_signal_connect (timeline, "marker-reached",
                        G_CALLBACK (mnb_spinner_marker_reached_cb),
                        self);
    }
  else
    g_warning ("%s did not have background-image set in style !!!",
               G_OBJECT_TYPE_NAME (self));


  g_signal_connect (self, "style-changed",
                    G_CALLBACK (_mnb_spinner_style_changed),
                    NULL);
}

static void
mnb_spinner_class_init (MnbSpinnerClass *klass)
{
  ClutterActorClass *actor_class  = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MnbSpinnerPrivate));

  actor_class->allocate             = mnb_spinner_allocate;
  actor_class->get_preferred_width  = mnb_spinner_get_preferred_width;
  actor_class->get_preferred_height = mnb_spinner_get_preferred_height;
  actor_class->pick                 = mnb_spinner_pick;
  actor_class->paint                = mnb_spinner_paint;

  object_class->constructed         = mnb_spinner_constructed;
  object_class->dispose             = mnb_spinner_dispose;
  object_class->finalize            = mnb_spinner_finalize;
  object_class->set_property        = mnb_spinner_set_property;
  object_class->get_property        = mnb_spinner_get_property;
}

static void
mnb_spinner_init (MnbSpinner *self)
{
  self->priv = MNB_SPINNER_GET_PRIVATE (self);
}

ClutterActor*
mnb_spinner_new (void)
{
  return g_object_new (MNB_TYPE_SPINNER, NULL);
}
