// imports.gi.versions.Mx = '2.0';

// const Mx = imports.gi.Mx;

const MainLoop = imports.mainloop;

const Weather = imports.Weather;

function main()
{
  let proxy = new Weather.AccuweatherProxy();

  proxy.search_locations_async('Melbourne', function(locations)
    {
      print("Success 1");
      print(locations[0]);
      proxy.request_location_async(locations[0].get_id(), function(report)
        {
          print("Success A");

          report.dump();

          MainLoop.quit('main');
        });
    });

  MainLoop.run('main');
}

main();
