# BlueprintHttp
UT4/UE4.15 plugin exposing nonintrusive blueprint nodes for http (json)

- [Http GET](#httpget)
- [Http POST with a JSON body](#httppost)
- [Building complex json objects](#buildjson)
- [Receiving complex json objects](#receivejson)
- [Sending custom headers](#headers)
- [Decent error handling](#errors)
- [About "non-intrusive" blueprint nodes](#nonintrusive)

<a name="httpget"></a>
## Http GET

<a name="httppost"></a>
## Http POST

<a name="buildjson"></a>
## Building complex JSON

<a name="receivejson"></a>
## Receiving complex JSON

<a name="headers"></a>
## Sending custom headers

<a name="errors"></a>
## Error handling

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
You can have custom objects in your plugin, but they must not be exposed to the blueprint graph.
When client tries to load an asset with native object references that don't resolve, game crashes.
This may happen as soon as client opens the "start match" screen, as the game tries to load all mutators and gamemodes.
And it will keep happening until client deletes offending PAK.

All this does not necessarily matter if, for example, you make a server-side-only mutator, and do not want to distribute the mutator without the plugin.
That is why I came up with this naming for "instrusive" and "non-intrusive" nodes.

Non-intrusive nodes can be used for assets that are going to be sent to clients, knowing those clients will not have the plugin.
