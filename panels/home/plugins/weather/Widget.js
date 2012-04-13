const Lang = imports.lang;
const MainLoop = imports.mainloop;

imports.gi.versions.Mx = '2.0';

const Mx = imports.gi.Mx;
const GObject = imports.gi.GObject;

const Pager = imports.Pager;

const DEGREES = "\u00B0";

const Button = new Lang.Class({
  Name: 'Button',
  Extends: Mx.Button,
  Properties: {
    'id': GObject.ParamSpec.string('id', '', '',
        GObject.ParamFlags.READABLE | GObject.ParamFlags.WRITABLE,
        ''),
  },

  _init : function (params)
  {
    this.parent(params);
    this._id = '';
  },

  get id ()
  {
    return this._id;
  },

  set id (val)
  {
    this._id = val;
  },
});

const ButtonFactory = new Lang.Class({
  Name: 'ButtonFactory',
  Extends: GObject.Object,
  Implements: [ Mx.ItemFactory ],

  _init : function (callback)
  {
    this.parent();
    this._callback = callback;
  },

  vfunc_create : function ()
  {
    let button = new Button({
        'style-class': 'CompletionListButton',
      });

    button.connect('clicked', Lang.bind(this, function ()
      {
        this._callback(button);
      }));

    return button;
  },
});

const Widget = new Lang.Class({
  Name: 'Widget',

  _init : function (proxy)
  {
    this.parent();
    this._proxy = proxy;

    this._metric = true;
    this._id = '';

    this._block_text_changed = 0;

    this._entry_timeout = 0;
    this._weather_timeout = 0;
  },

  update_weather : function ()
  {
    if (this._id.length == 0)
      return;

    if (this._weather_timeout > 0)
        MainLoop.source_remove(this._weather_timeout);

    this._weather_timeout = MainLoop.timeout_add_seconds(60 * 60,
        Lang.bind(this, function () { this._update_weather() } ));

    this._update_weather();
  },

  _update_weather : function ()
  {
    let table = this._widget;

    /* remove the current children */
    table.foreach(function (actor)
      {
        table.remove_actor(actor);
      });

    let title = new Mx.Label({
        'style-class': 'WeatherTileTitle',
        'text': "Loading",
      });
    table.insert_actor(title, 0, 0);

    let spinner = new Mx.Spinner();
    table.insert_actor(spinner, 0, 1);

    this._proxy.request_location_async(this._id, this._metric,
        Lang.bind(this, function (weather)
      {
        spinner.destroy();
        weather.dump();

        /* FIXME: translation */
        title.set_text(weather.city + " Weather");

        let pager = new Pager.Pager();
        table.insert_actor(pager, 1, 0);

        pager.add_page(this._build_today_page(weather));

        for (let i = 1; i < weather.forecast.length; i++)
          pager.add_page(this._build_forecast_page(weather.forecast[i]));
      }));

    this._weather_timeout = 0;
    return true;
  },

  _build_today_page : function (weather)
  {
    let forecast = weather.forecast[0];
    let table = new Mx.Table();

    let label = new Mx.Label({
        'style-class': 'WeatherTileDay',
        'text': "Today",
      });
    table.insert_actor(label, 0, 0);

    let image = new Mx.Image();
    image.set_from_file('images/' + weather.current.iconnum + '.GIF');
    table.insert_actor(image, 1, 0);

    let subtable = new Mx.Table({
        'style-class': 'WeatherObservations',
      });
    table.insert_actor(subtable, 2, 0);

    let label = new Mx.Label({
        'style-class': 'CurrentTemperature',
        'text': weather.current.temp + DEGREES + weather.units.temp,
      });
    subtable.insert_actor(label, 0, 0);
    subtable.child_set_row_span(label, 2);

    let label = new Mx.Label({
        'text': weather.current.text
      });
    subtable.insert_actor(label, 0, 1);
    subtable.child_set_column_span(label, 2);

    subtable.insert_actor(new Mx.Label({
        'text': "Hi: " + forecast.hightemp + DEGREES + forecast.units.temp,
      }), 1, 1);
    subtable.insert_actor(new Mx.Label({
        'text': "Lo: " + forecast.lowtemp + DEGREES + forecast.units.temp,
      }), 1, 2);

    return table;
  },

  _build_forecast_page : function (forecast)
  {
    let table = new Mx.Table();

    table.insert_actor(new Mx.Label({
        'text': forecast.dayname,
        'style-class': 'WeatherTileDay',
      }), 0, 0);

    // FIXME: we don't know the icon for forecasts
    // let image = new Mx.Image();
    // image.set_from_file('images/02.GIF');
    // table.insert_actor(image, 1, 0);

    let subtable = new Mx.Table({
        'style-class': 'WeatherObservations',
      });
    table.insert_actor(subtable, 2, 0);

    subtable.insert_actor(new Mx.Label({
        'text': "Hi: " + forecast.hightemp + DEGREES + forecast.units.temp,
      }), 3, 0);
    subtable.insert_actor(new Mx.Label({
        'text': "Lo: " + forecast.lowtemp + DEGREES + forecast.units.temp,
      }), 4, 0);

    return table;
  },

  _construct_completion : function (entry)
  {
    if (this._completion)
      return;

    const transparent = new Clutter.Color({
        'red': 0x0,
        'blue': 0x0,
        'green': 0x0,
        'alpha': 0x0,
      });

    let stage = entry.get_stage();
    let eventbox = new Clutter.Rectangle({
        'color': transparent,
        'width': stage.get_width(),
        'height': stage.get_height(),
        'reactive': true,
      });

    stage.add_actor(eventbox);

    let frame = new Mx.Frame({
        'name': 'CompletionFrame',
        'x-fill': true,
        'y-fill': true,
      });

    let scroll = new Mx.ScrollView({
        'name': 'CompletionScrollView',
        'clip-to-allocation': true,
        'height': 300,
      });

    frame.set_child(scroll);
    stage.add_actor(frame);

    frame.raise_top();
    frame.set_position(entry.get_x(), entry.get_y() + entry.get_height());
    frame.hide();

    let model = Clutter.ListModel.newv(
        [ GObject.TYPE_STRING, GObject.TYPE_STRING ],
        [ 'City', 'Id' ]);

    let factory = new ButtonFactory(Lang.bind(this, function (button)
      {
        this._block_text_changed++;

        entry.set_text(button.get_label());
        this._id = button.id;

        this.update_weather();
        frame.hide();

        this._block_text_changed--;
      }));

    let view = new Mx.ListView({
        'name': 'CompletionListView',
        'model': model,
        'factory': factory,
      });

    view.add_attribute('label', 0);
    view.add_attribute('id', 1);
    scroll.add_actor(view);

    eventbox.connect('button-press-event', function ()
      {
        frame.hide();
        eventbox.hide()

        return true;
      });

    frame.connect('show', function ()
      {
        eventbox.show()
        eventbox.lower(frame);
      });
    frame.connect('hide', function ()
      {
        eventbox.hide();
      });

    this._completion = frame;
    this._model = model;
  },

  get_configuration : function ()
  {
    let table = new Mx.Table();

    let label = new Mx.Label({ 'text': "Temperature" });
    table.insert_actor(label, 0, 0);
    table.child_set_column_span(label, 2);

    /* FIXME: how do I make this say F/C ? */
    let tempswitch = new Mx.Toggle();
    table.insert_actor(tempswitch, 1, 0);
    table.child_set_column_span(tempswitch, 2);

    tempswitch.connect('notify::active', Lang.bind(this, function ()
      {
        this._metric = !tempswitch.get_active();
        this.update_weather();
      }));

    let label = new Mx.Label({ 'text': "Location" });
    table.insert_actor(label, 2, 0);
    table.child_set_column_span(label, 2);

    let entry = new Mx.Entry({
        'hint-text': "Search for a location",
      });
    table.insert_actor(entry, 3, 0);

    let spinner = new Mx.Spinner({ 'visible': false });
    table.insert_actor(spinner, 3, 1);

    entry.connect('notify::text', Lang.bind(this, function ()
      {
        if (this._block_text_changed > 0)
          return;

        if (this._entry_timeout > 0)
          MainLoop.source_remove(this._entry_timeout);

        this._entry_timeout = MainLoop.timeout_add(300,
            Lang.bind(this, function ()
          {
            this._construct_completion(entry);

            let text = entry.get_text();

            if (text.length >= 3)
              {
                spinner.show();

                this._proxy.search_locations_async(entry.text,
                  Lang.bind(this, function (locations)
                    {
                      spinner.hide();

                      /* clear model */
                      while (this._model.get_n_rows() > 0)
                        this._model.remove(0);

                      /* rebuild the model */
                      for (let i = 0; i < locations.length; i++)
                        {
                          print(locations[i]);
                          this._model.appendv([ 0, 1 ],
                              [ locations[i].get_printable_name(),
                                locations[i].get_id() ]);
                        }

                      if (locations.length > 0)
                        {
                          this._completion.show();
                          this._completion.raise_top();
                        }
                      else
                        {
                          this._completion.hide();
                        }
                    }));
              }
            else
              {
                this._completion.hide();
              }

            this._entry_timeout = 0;
            return false;
          }));
      }));

    return table;
  },

  get_widget : function ()
  {
    this._widget = new Mx.Table();

    return this._widget;
  },
});
