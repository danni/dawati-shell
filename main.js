const MainLoop = imports.mainloop;
const Clutter = imports.gi.Clutter;

const Weather = imports.Weather;
const Widget = imports.Widget;

function main()
{
  Clutter.init(null, null);

  let stage = Clutter.Stage.get_default();
  let proxy = new Weather.AccuweatherProxy();
  let widget = new Widget.Widget(proxy);

  stage.add_actor(widget.get_actor());
  stage.show_all();

  MainLoop.run('main');
}

main();
