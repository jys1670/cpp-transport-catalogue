Once again, "transport catalogue" is a simplistic public transport navigator.
In order to find optimal path between two objects some kind of map must be available.
Preferably it should be similar to common bus routes map, where each bus is described by a list of stops and arrival time.
Traffic jams (and other constantly changing things) are ignored.

Navigator expects data to be received from standard input in the following format:
```
{
  "base_requests": [ ... ],           // 1
  "render_settings": { ... },         // 2
  "routing_settings": { ... },        // 3
  "serialization_settings": { ... },  // 4
  "stat_requests": [ ... ]            // 5
}
```

1) The `base_requests` array contains two types of elements: _routes_ and _stops_, that will be used to construct map.
They can be listed in arbitrary order.

Stop example:
```json
{
  "type": "Stop",
  "name": "Times Sq",
  "latitude": 40.758896,
  "longitude": -73.985130,
  "road_distances": {
    "Street–Penn Station": 3000,
    "Union Sq": 4300
  }
} 
```
*
  * `type` — "Stop" string. Means that the dictionary describes the bus stop.
  * `name` — the name of the stop.
  * `latitude` and `longitude` — latitude and longitude of the stop — floating point numbers.
  * `road_distances` is a dictionary that specifies the road distance from this stop to the neighboring ones. 
  Each key in this dictionary is the name of a nearby stop, the value is an integer distance in meters.

Route example:
```json
{
"type": "Bus",
"name": "14",
"stops": [
"Union Sq",
"Times Sq",
"Street–Penn Station",
"Union Sq"
],
"is_roundtrip": true
}
```
*
  * `type` — string "Bus". Means that the dictionary describes the bus route.
  * `name` — the name of the route.
  * `stops` — an array with the names of stops that the route passes through. For a circular route, the name of the last stop duplicates the name of the first one. For example: ["stop1", "stop2", "stop3", "stop1"].
  * `is_roundtrip` is a bool type value, true if the route is circular.

2. `render_settings` controls map visualization
```json
{
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
} 
```
*
  * `line_width` — the thickness of the lines used to draw bus routes. A real number in the range from 0 to 100000.
  * `stop_radius`— the radius of the circles that indicate stops. A real number in the range from 0 to 100000.
  * `bus_label_font_size` — the size of the text used to write the names of bus routes. An integer in the range from 0 to 100000.
  * `bus_label_offset` — offset of the route name label relative to the coordinates of the final stop on the map. An array of two elements of the double type with values in range from -100000 to 100000. Sets the values of the dx and dy properties of the <text> SVG element.
  * `stop_label_font_size` — the size of the text that displays the names of stops. An integer in the range from 0 to 100000.
  * `stop_label_offset` — offset of the name of the stop relative to its coordinates on the map. An array of two elements of the double type with values in range from -100000 to 100000. Sets the values of the dx and dy properties of the <text> SVG element.
  * `underlayer_color` — the color of the background under the names of stops and routes. The color format is below.
  * `underlayer_width` — the thickness of the substrate under the names of stops and routes. Sets the value of the stroke-width attribute of the <text> element. Double type with value in the range from 0 to 100000.
  * `color_palette` — color palette. A non-empty array filled with elements in the form of:
    * Strings like "red", "black", etc
    * RGB arrays like [255, 16, 12]
    * RGBA arrays like [255, 200, 23, 0.85]

3. `routing_settings` dictionary adjusts parameters of the pathfinding algorithm
```json
{
  "bus_wait_time": 6,
  "bus_velocity": 40
} 
```
*
  * `bus_wait_time` — waiting time for the bus at the stop, in minutes. Whenever a person comes to a stop and whatever that stop is, he (or she) will wait for bus for exactly the specified number of minutes. The value is an integer from 1 to 1000.
  * `bus_velocity` — bus speed, in km/h. It is constant and exactly equal to the specified number. Stops parking time is not taken into account, acceleration and braking time too. The value is a real number from 1 to 1000.
4. `serialization_settings` — dictionary with a single `file` key and a string value — name of the file where centralized database (aka everything except `stat_requests`) will be saved.
5. `stat_requests` is an array of requests that produce some kind of output based on previously provided data. There are four types of requests available:
*
  * Query stop/route information:
  ```json
  {
    "id": 123456,
    "type": "Stop",
    "name": "Union Sq"
  }
  ```
  ```json
  {
    "id": 12345,
    "type": "Bus",
    "name": "14"
  } 
  ```
  * Generate map:
  ```json
  {
    "id": 1234,
    "type": "Map"
  }
  ```
  * Find the fastest path:
  ```json
  {
    "type": "Route",
    "from": "Union Sq",
    "to": "Street–Penn Station",
    "id": 123
  } 
  ```