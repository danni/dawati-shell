/*
 * Copyright (c) 2012 Intel Corp.
 *
 * Author: Danielle Madeley <danielle.madeley@collabora.co.uk>
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

imports.gi.versions.Mx = '2.0';

const Clutter = imports.gi.Clutter;
const ClutterGst = imports.gi.ClutterGst;
const Mx = imports.gi.Mx;
const Gio = imports.gi.Gio;
const GObject = imports.gi.GObject;

function app_plugin () {}

app_plugin.prototype = {
  init: function() {
    ClutterGst.init(null, null);

    // this._settings = new Gio.Settings({
    //     schema: 'org.dawati.shell.home.plugin.video',
    //     path: this.settings_path,
    //   });
  },

  get_widget: function() {
    let stack = new Mx.Stack();

    let video = new ClutterGst.VideoTexture();
    video.set_filename('/home/danni/Videos/JJ Penguin.m4v');
    stack.add_actor(video);

    let table = new Mx.Table();
    stack.add_actor(table);

    let playpause = new Mx.Button();
    table.add_actor(playpause, 0, 0);
    table.child_set_x_fill(playpause, false);
    table.child_set_x_expand(playpause, false);
    table.child_set_y_fill(playpause, false);
    table.child_set_y_expand(playpause, false);

    playpause.set_label('>');
    playpause.playing = false;
    playpause.connect('clicked', function() {
      video.set_playing(!video.get_playing());
    });
    video.connect('notify::playing', function() {
      if (video.get_playing())
        playpause.set_label('||');
      else
        playpause.set_label('>');
    });

    let scrubber = new Mx.Slider();
    table.add_actor(scrubber, 0, 1);

    video.bind_property('progress', scrubber, 'value',
        GObject.BindingFlags.BIDIRECTIONAL | GObject.BindingFlags.SYNC_CREATE);
    video.bind_property('buffer-fill', scrubber, 'buffer-value',
        GObject.BindingFlags.SYNC_CREATE);

    stack.show_all();

    return stack;
  },

  get_configuration: function() {
  },

  deinit: function() {
  },
}

var extensions = {
  'DawatiHomePluginsApp': app_plugin,
};
