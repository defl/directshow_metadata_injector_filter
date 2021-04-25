# [DirectShow Metadata Injector Filter](https://github.com/defl/directshow_metadata_injector_filter)

**Warning: This is feature complete yet - there is no way to configure without recompiling**

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
* Tested players
    * Potplayer
* Tested DirectShow sources
	* AVerMedia Live Gamer 4K - GC573
* Tested DirectShow renderers
    * madVR

## Install

Potplayer

* Download directshow_metadata_injector_filter.ax to a location you will not delete
* Install VS2019 runtime
* Open potplayer
* Open preferences (F5)
* In the tree menu go to: Filter Control -> Filter Priority (Overall) ([screenshot](images/potplayer_settings.png))
* Click Add external filter
* Go to the directory where you downloaded the directshow_metadata_injector_filter.ax, select the file and press open
* There should be a new filter in the Custom Filter Manager called "Metadata Injector"
* Select Metadata Injector
* Bottom right of screen set priority to "Prefer"

*(If you're running a AVerMedia Live Gamer 4K - GC573 don't forget to set it's output to P010 for 10bit, you can do this in preferences under device->webcam->format, see [here](images/potplayer_avermedia_settings.png))*

## Build

Open with Visual Studio 2019 VC++, should just work out of the box.

## FAQ

Is this plug and play? No, because this filter does not offer anything new to get an video stream decoded it will not automatically be added to a DirectShow graph. It needs to be forced to be included, see install for an example.


## Credits

Thanks to WP for the initial introduction to DirectShow 

Thanks to Hendrik Leppkes' for his excellent [LAV](https://github.com/Nevcairiel/LAVFilters/releases) filter.

Thanks to madshi for [madVR](http://madvr.com/).

No thanks to all capture card manufacturers who are willing to sell you half implemented hardware.