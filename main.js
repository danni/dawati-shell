const MainLoop = imports.mainloop;
const Clutter = imports.gi.Clutter;

imports.gi.versions.Mx = '2.0';
const Mx = imports.gi.Mx;

const Weather = imports.Weather;
const Widget = imports.Widget;

function main()
{
  Clutter.init(null, null);

  /* FIXME: hacky */
  const PREFIX = '/home/danni/src/install';
  Mx.Style.get_default().load_from_file(
      PREFIX + '/share/dawati-shell/theme/datetime/date-panel.css');
  Mx.Style.get_default().load_from_file('weather.css');

  let stage = Clutter.Stage.get_default();
  let proxy = new Weather.AccuweatherProxy();
  let widget = new Widget.Widget(proxy);
  let table = new Mx.Table();

  table.add_actor(widget.get_config(), 0, 0);
  table.add_actor(widget.get_widget(), 1, 0);

  stage.add_actor(table);
  stage.show_all();

  MainLoop.run('main');
}

main();
