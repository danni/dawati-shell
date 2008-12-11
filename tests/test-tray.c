/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2008 Intel Corp.
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

/*
 * Simple system tray test.
 */

#include "../src/moblin-netbook-system-tray.h"

static void
activate_cb (GtkStatusIcon *status_icon, gpointer data)
{
  g_message ("Received activate signal.");
}

static void
popup_menu_cb (GtkStatusIcon *icon, guint button, guint atime, gpointer data)
{
  g_message ("Received pop-up menu signal with coords %d, %d.",
	     mnbk_event_x, mnbk_event_y);
}

int
main (int argc, char *argv[])
{
  GtkStatusIcon *icon;

  gtk_init (&argc, &argv);

  icon = gtk_status_icon_new_from_stock (GTK_STOCK_INFO);

  g_signal_connect (icon, "activate", G_CALLBACK (activate_cb), NULL);
  g_signal_connect (icon, "popup-menu", G_CALLBACK (popup_menu_cb), NULL);

  gdk_add_client_message_filter (gdk_atom_intern (MOBLIN_SYSTEM_TRAY_EVENT,
						  FALSE),
				 mnbtk_client_message_handler, icon);

  gtk_status_icon_set_visible (icon, TRUE);

  gtk_main ();

  return 0;
}
