const Lang = imports.lang;
const MainLoop = imports.mainloop;

imports.gi.versions.Mx = '2.0';

const Mx = imports.gi.Mx;
const GObject = imports.gi.GObject;

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
    let button = new Button();

    button.set_style_class('CompletionListButton');
    button.connect('clicked', Lang.bind(this, function ()
      {
        this._callback(button);
      }));

    return button;
  },
});

const Widget = new Lang.Class({
  Name: 'Widget',
  /* FIXME: it would be nice to make this inherit from Mx.Table, but it screws
   * up the widget allocation. IDK why */

  _init : function (proxy)
  {
    this.parent();
    this._proxy = proxy;
    this._block_text_changed = 0;
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
    frame.set_position(14, 167); // huh?
    frame.hide();

    let model = Clutter.ListModel.newv(
        [ GObject.TYPE_STRING, GObject.TYPE_STRING ],
        [ 'City', 'Id' ]);

    let factory = new ButtonFactory(Lang.bind(this, function (button)
      {
        this._block_text_changed++;

        print(button.id);
        entry.set_text(button.get_label());
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

  get_actor : function ()
  {
    let table = new Mx.Table();
    let entry = new Mx.Entry({
        'hint-text': "Search for a location",
      });
    table.add_actor(entry, 0, 0);

    let spinner = new Mx.Spinner({ 'visible': false });
    table.add_actor(spinner, 0, 1);

    this._entry_timeout = 0;
    entry.clutter_text.connect('text-changed', Lang.bind(this, function ()
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
});
