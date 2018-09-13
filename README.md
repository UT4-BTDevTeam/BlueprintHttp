# BlueprintHttp
UT4/UE4.15 plugin exposing nonintrusive blueprint nodes for http (json)

[img: http get node]()

[img: http post node]()

- [Installing plugin](#install)
- [Building complex json objects](#buildjson)
- [Receiving complex json objects](#receivejson)
- [Sending custom headers](#headers)
- [Error handling](#errors)
- [Important notes about Latent actions](#latent)
- [About "non-intrusive" blueprint nodes](#nonintrusive)

<a name="install"></a>
## Installing plugin
Download repository ZIP, extract, and paste the entire folder into the `Engine/Plugins/Runtime/` folder of your UT build.

Make sure the result path looks like this :
```bash
# editor
UTEditor/Engine/Plugins/Runtime/BlueprintHttp-master/Binaries/Win64/UE4Editor-BlueprintHttp.dll
# linux server
UTServer/Engine/Plugins/Runtime/BlueprintHttp-master/Binaries/Linux/libUE4Server-BlueprintHttp-Linux-Shipping.so
```

<a name="buildjson"></a>
## Building complex JSON
The library provides two straightforward for building simple to complex JSON in blueprint, taking care of formatting and escaping problems.

[img: make json object node]()

***Make Json Object*** generates a properly formatted JSON object as string, ready to be used as a POST Body, or as a "value" of type `object` when constructing a json object, or when constructing a json array of objects.

[img: make json array node]()

***Make Json Array*** generates a properly formatted JSON array as string, ready to be used as a "value" of type `array` when constructing a json object, or when constructing a json array of arrays.

In either case, the ***Types*** array must specify, for each value, the desired type of the value : `string` , `number` , `boolean` , `object` , or `array`. This tells the plugin how to format each value properly, rather than treating them all as strings.
- For `number` and `boolean`, simply add your int/float/bool variable into ***Values*** and let editor cast them to strings. Don't forget to specify the proper type.
- For `string`, the plugin will take care of wrapping in quotes and escaping bad characters.
- For `object` and `array`, the corresponding input value *should* be the result of a prior **Make Json Object** or **Make Json Array** node.

[img: complex json graph]()

<a name="receivejson"></a>
## Receiving complex JSON
Retrieving response data is done through the ***Keys*** and ***Values*** output pins.
As their names imply, **Keys** contains an array of keys, and **Values** contains an array of values.
- When the response object is *simple*, keys and values are trivially pushed to their respective array.
- When the response object is *complex*, the same method is used after a ***flattening*** process.

Nothing better than an example to illustrate, consider the following *complex* json object :
```json
{
  "name": "Brice",
  "age": 42,
  "activities": [ "Surfer", "Winner" ],
  "city": {
    "name": "Nice",
    "coords": { "lat": 43.710339, "lng": 7.261742 }
  },
  "matches": [
    { "kills": 10, "deaths": 9, "win": true },
    { "kills": 4, "deaths": 12, "win": false }
  ]
}
```
In order to give all necessary information to blueprints, it will be **flattened** and pushed into the **Keys** & **Values** like this :
```json
"name"              : "Brice",
"age"               : "42",
"activities.length" : "2",
"activities.0"      : "Surfer",
"activities.1"      : "Winner",
"city.name"         : "Nice",
"city.coords.lat"   : "43.710339",
"city.coords.lng"   : "7.261742",
"matches.length"    : "2",
"matches.0.kills"   : "10",
"matches.0.deaths"  : "9",
"matches.0.win"     : "true",
"matches.1.kills"   : "4",
"matches.1.deaths"  : "12",
"matches.1.win"     : "false",
```

<a name="headers"></a>
## Sending custom headers
By default the plugin sets the following headers :
```
Content-Type: application/json
Accept: application/json
User-Agent: X-UnrealEngine-Agent
```
Adding or overriding headers is pretty straightforward, using the ***Header Keys*** and ***Header Values*** input pins. Similarly to JSON objects, one array for the keys and one array for the values.

[img: example]()

<a name="errors"></a>
## Error handling
The ***success*** pin is currently directly bound to the internal result of the HTTP Request, which can be occasionally be a bit obscure. Basically, some HTTP codes are considered success from the request point of view, despite being error status codes.

Therefore, while it is certain the request failed when `success=false`, having `success=true` does not guarantee actual success. It is suggested to do the following :

[img: error handling]()

<a name="latent"></a>
## Important notes about Latent actions
Todo.

<a name="nonintrusive"></a>
## About "non-intrusive" blueprint nodes
When a blueprint uses native nodes from a plugin, it creates a ***dependency*** between the PAK and the plugin.
This dependency can be ***weaker*** or ***stronger*** depending on cases.
As everybody should know, it is not possible to distribute native plugins through the auto-download system in UT4.
When a server uses custom content that relies on plugin nodes, clients will only receive the PAK but not the plugin.
Therefore depending on the strength of that dependency stated above, this can be fine or ***critical***.

Through extensive testing I figured the following :
- Exposing new ***native nodes*** is fine. If your mutator graph has native nodes in them, it will not prevent clients from loading the mutator.
Upon loading, clients will log an error about failing to load some plugin/script, but it will work.
Execution flows can even go through the native nodes, and continue as if nothing. Output pins will remain at default values.
- Exposing new ***native objects*** is dangerous. Whether it is a native Class, Struct, or Enum, even if it is just exposed through an output pin.
When client tries to load an asset (graph) with native object references that don't resolve, game crashes.
This may happen as soon as client opens the "start match" screen, as the game tries to load all mutators and gamemodes.
And it will keep happening until client deletes offending PAK.
You can use custom objects within your plugin, but they must not be exposed to the blueprint graph.

All this does not necessarily matter if, for example, you make a server-side-only mutator, and do not want to distribute the mutator without the plugin.
That is why I came up with this naming for "instrusive" and "non-intrusive" nodes.

Non-intrusive nodes can be used for assets that are going to be sent to clients, knowing those clients will not have the plugin.
