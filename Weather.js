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

const Lang = imports.lang;

const Rest = imports.gi.Rest;

/* This is not strictly a Restful API, but we can deal */
const URL_FORMAT = 'http://inteltizen.accu-weather.com/widget/inteltizen/';
const FIND_CITY_FUNC = 'city-find.asp';
const WEATHER_DATA_FUNC = 'weather-data.asp';

const Location = new Lang.Class({
  Name: 'Location',

  _init : function (node)
  {
    this.city = node.get_attr('city');
    this.country = node.get_attr('country');
    this.municipality = node.get_attr('adminArea');
    this.city_id = node.get_attr('location');
  },

  get_printable_name : function ()
  {
    return this.city + ', ' + this.municipality + ', ' + this.country;
  },

  get_id : function ()
  {
    return this.city_id;
  },

  toString : function ()
  {
    return '[Location(' + this.city_id + '-' + this.get_printable_name() + ')]';
  },
});

const Units = new Lang.Class({
  Name: 'Units',

  _init : function (node)
  {
    this.temp = node.find('temp').content;
    this.dist = node.find('dist').content;
    this.speed = node.find('speed').content;
    this.pres = node.find('pres').content;
    this.prec = node.find('prec').content;
  },
});

const CurrentConditions = new Lang.Class({
  Name: 'CurrentConditions',

  _init : function (units, node)
  {
    this.units = units;

    this.temp = node.find('temperature').content;
    this.humidity = node.find('humidity').content;
    this.text = node.find('weathertext').content; /* FIXME: i18n? */
  },

  dump : function ()
  {
    print ("Now: " + this.temp + this.units.temp + " " +
        "Humidity: " + this.humidity);
  },
});

const ForecastDay = new Lang.Class({
  Name: 'ForecastDay',

  _init : function (units, node)
  {
    this.units = units;

    this.dayname = node.find('daycode').content;
    this.hightemp = node.find('hightemperature').content;
    this.lowtemp = node.find('lowtemperature').content;
  },

  dump : function ()
  {
    print (this.dayname + ": " +
        "High: " + this.hightemp + this.units.temp + " " +
        "Low: " + this.lowtemp + this.units.temp);
  },
});

const Weather = new Lang.Class({
  Name: 'Weather',

  _init : function (dom)
  {
    this.units = new Units(dom.find('units'));
    this.current = new CurrentConditions(this.units,
                                         dom.find('currentconditions'));
    this.forecast = new Array();

    for (let d = dom.find('day'); d != null; d = d.next)
      {
        this.forecast.push(new ForecastDay(this.units, d));
      }
  },

  dump : function()
  {
    this.current.dump();

    for (let i = 0; i < this.forecast.length; i++)
     this.forecast[i].dump();
  },
});

const AccuweatherProxy = new Lang.Class({
  Name: 'AccuweatherProxy',
  Extends: Rest.Proxy,

  _init : function ()
  {
    this.parent({
        'url-format': URL_FORMAT,
      });
  },

  search_locations_async : function (city, callback)
  {
    let call = new Rest.ProxyCall({
        'proxy': this,
      });

    call.set_function(FIND_CITY_FUNC);
    call.add_param('location', city);

    call.invoke_async(null, function (call, result)
      {
        call.invoke_finish(result);

        let parser = new Rest.XmlParser();
        let dom = parser.parse_from_data(call.get_payload(),
                                         call.get_payload_length());

        let locations = new Array();

        for (let l = dom.find('location'); l != null; l = l.next)
          {
            if (l.name != 'location')
              continue;

            locations.push(new Location(l));
          }

        callback(locations);
      });
  },

  request_location_async : function (cityid, metric, callback)
  {
    let call = new Rest.ProxyCall({
        'proxy': this,
      });

    /* request the data for a city */
    call.set_function(WEATHER_DATA_FUNC);
    call.add_param('location', cityid);
    if (metric)
      call.add_param('metric', '1');

    call.invoke_async(null, function (call, result)
      {
        call.invoke_finish(result);

        let parser = new Rest.XmlParser();
        let dom = parser.parse_from_data(call.get_payload(),
                                         call.get_payload_length());

        callback(new Weather(dom));
      });
  },
});
