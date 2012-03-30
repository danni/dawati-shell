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

  let stage = Clutter.Stage.get_default();
  let proxy = new Weather.AccuweatherProxy();
  let widget = new Widget.Widget(proxy);

  stage.add_actor(widget.get_actor());
  stage.show_all();

  MainLoop.run('main');
}

main();
