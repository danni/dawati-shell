const Lang = imports.lang;

imports.gi.versions.Mx = '2.0';

const Mx = imports.gi.Mx;
const Clutter = imports.gi.Clutter;

const PAGER_WIDTH = 20.;

const Pager = new Lang.Class({
  Name: 'Pager',
  Extends: Mx.Stack,
  Implements: [ Clutter.Container ],

  _init : function (params)
  {
    this.parent(params);
    this._pages = [];
    this._current_page = -1;

    this.set_clip_to_allocation(true);

    let prevbox = new Mx.Frame({
        'width': PAGER_WIDTH,
        'reactive': true,
        'style-class': 'PagerClicky',
      });

    this.add_actor(prevbox);
    this.child_set_x_fill(prevbox, false);
    this.child_set_x_align(prevbox, Mx.Align.START);

    prevbox.connect('button-press-event', Lang.bind(this, function ()
      {
        let page = Math.max(this._current_page - 1, 0);

        this._update_pager(page, true);
        return true;
      }));

    let nextbox = new Mx.Frame({
        'width': PAGER_WIDTH,
        'reactive': true,
        'style-class': 'PagerClicky',
      });

    this.add_actor(nextbox);
    this.child_set_x_fill(nextbox, false);
    this.child_set_x_align(nextbox, Mx.Align.END);

    nextbox.connect('button-press-event', Lang.bind(this, function ()
      {
        let page = Math.min(this._current_page + 1, this._pages.length - 1);

        this._update_pager(page, true);
        return true;
      }));
  },

  _update_pager : function (new_page, animate)
  {
    const ANIM_MODE = Clutter.AnimationMode.LINEAR;
    const ANIM_DURATION = 250;

    let oldpage = this._pages[this._current_page];
    let newpage = this._pages[new_page];

    let direction = new_page > this._current_page ? 1 : -1;

    if (this._current_page == new_page)
      return;

    if (this._current_page != -1)
      {
        if (animate)
          {
            let animation = oldpage.animatev(ANIM_MODE, ANIM_DURATION,
                [ 'anchor-x' ],
                [ direction * this.get_width() ]);

            animation.connect('completed', function ()
              {
                /* we make the actor invisible instead of hiding it, so
                 * that we don't change the size of the Mx.Stack */
                oldpage.set_opacity(0x0);
                oldpage.set_anchor_point(0., 0.);
              });
          }
        else
          {
            oldpage.set_opacity(0x0);
          }

        newpage.raise(oldpage);
      }

    if (animate)
      {
        newpage.set_anchor_point(direction * -this.get_width(), 0.);
        newpage.animatev(ANIM_MODE, ANIM_DURATION,
            [ 'anchor-x' ],
            [ 0 ]);
      }

    newpage.set_opacity(0xff);
    newpage.show();

    this._current_page = new_page;
  },

  add_page : function (actor)
  {
    this._pages.push(actor);
    this.add_actor(actor);
    this.child_set_fit(actor, true);
    actor.set_opacity(0x0)

    if (this._current_page == -1)
      {
        this._update_pager(0, false);
      }
  },

  remove_page : function (actor)
  {
    let i = this._pages.indexOf(actor);

    if (i == -1)
      return;

    this._pages.splice(i, 1);
    this.remove_actor(actor);

    if (i == this._current_page)
      {
        let page = (this._current_page + 1) % this._pages.length;

        this._update_pager(page, false);
      }

  },
});
