# [DirectShow Metadata Injector Filter](https://github.com/defl/directshow_metadata_injector_filter)

This is a DirectShow filter that can inject metadata related to high dynamic range (HDR) and wide color gamut (WCG) 
into a directshow graph.

This is usable for sources which do not properly relay this information like consumer video capture cards 
[AVerMedia Live Gamer 4K - GC573](https://www.avermedia.com/us/product-detail/GC573) who technically do HDR 
data but refuse to forward the correct HDR metdata through their DirectShow source filters. 
This leads to downstream renderers making a mess of the picture as they don't know how to interpret it.
Example [before](images/without.png) and [after](images/with.png) screenshots probably speak for themselves.

With this and a select few other bits and bobs you can build something like a poor mans' [madVR labs envy](https://madvrenvy.com/) if you're willing to forego the updates, ease of use and potential HDCP issues. If you like this, and have the means, please buy a real envy to support further madVR development.

## Features

* Supported metadata
    * HDR primaries & whitepoint as xy coordinates
	* HDR min/max mastering luminance
	* HDR content light levels (MaxCLL, MaxFALL)
	* HDR Transfer Matrix (BT.2020 etc)
	* HDR Primaries (DCI-P3, BT.2020 etc)
	* HDR Video transfer function (SMPTE ST 2084 (PQ), HLG etc)
* Configuration through an ini file (default in same directory)
* Tested players
    * Potplayer
* Tested DirectShow sources
    * AVerMedia Live Gamer 4K - GC573
    * Blackmagic Design DeckLink Mini Recorder 4K
* Tested DirectShow renderers
    * madVR

## Install

* Install VS2019 x64 runtime
* Download directshow_metadata_injector_filter.ax to a location you will not delete (and which is not admin only under windows like program files)
* Copy the example_config.ini as directshow_metadata_injector_filter.ini to the same dir

Potplayer

* Open potplayer
* Open preferences (F5)
* In the tree menu go to: Filter Control -> Filter Priority (Overall) ([screenshot](images/potplayer_settings.png))
* Click Add external filter
* Go to the directory where you downloaded the directshow_metadata_injector_filter.ax, select the file and press open
* There should be a new filter in the Custom Filter Manager called "Metadata Injector"
* Select Metadata Injector
* Bottom right of screen set priority to "Prefer"
*(If you're running a AVerMedia Live Gamer 4K - GC573 don't forget to set it's output to P010 for 10bit, you can do this in preferences under device->webcam->format, see [here](images/potplayer_avermedia_settings.png))*

optional: hdfury_virtex2_ini_generator.py

* Install Python
* Install python/requirements.txt modules
* Run hdfury_virtex2_ini_generator.py <ip of vertex2> <directshow_metadata_injector_filter.ini file from above>

## Configuration

Configuration is via a .ini file which contains all the settings. There is an example_config.ini._

There is a small Python program included in the python/ subdir which can automatically generate this file for a range of devices. For this to work you need
to place the device before the input to the capture server (you want to do this anyway because higher end audio is notriously poor though such setups). This 
way the device will see the same HDR meta data as the capture card and the filter can inject the correct metadata in the directshow stream. The config is 
re-generated every few seconds and a quick stop-start will reload everything and get you the right parameters (after a menu-switch or a new movie starts for 
example).

Supported devices:

 * HDFury Vertex2


## Build

**C++**: Open with Visual Studio 2019 VC++, should just work out of the box.

**Python**: Just have a python 3.x interpretor and install the packages mentioned in requirements.txt

## FAQ

Is this plug and play? No, because this filter does not offer anything new to get an video stream decoded it will not automatically be added to a DirectShow graph. It needs to be forced to be included, see install for an example.

## Credits

 * Thanks to WP for the initial introduction to DirectShow 
 * Thanks to Hendrik Leppkes' for his excellent [LAV](https://github.com/Nevcairiel/LAVFilters/releases) filter.
 * Thanks to madshi for [madVR](http://madvr.com/).

No thanks to all capture card manufacturers who are willing to sell you half implemented software on their hardware.
