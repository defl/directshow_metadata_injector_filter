# [DirectShow Metadata Injector Filter](https://github.com/defl/directshow_metadata_injector_filter)

This is a DirectShow filter that can inject metadata related to high dynamic range (HDR) and wide color gamut (WCG) the graph so it video can be correctly shown by capable renderers like [madVR](http://madvr.com/).

It is usable for sources which do not properly relay this information, like consumer video capture cards such as the [AVerMedia Live Gamer 4K - GC573](https://www.avermedia.com/us/product-detail/GC573), which technically do HDR data but refuse to forward the correct HDR metadata through their DirectShow source filters. This leads to downstream renderers making a mess of the picture as they don't know how to interpret it.

Example [before](images/without.png) and [after](images/with.png) screenshots probably speak for themselves.

This filter has a big bother called [VideoProcessor](http://www.videoprocessor.org/) (code at [defl/videoprocessor](https://github.com/defl/videoprocessor)) which is a Windows application that integrates capture, metadata generation and rendering into one application for total ease of use, full control and highest possible quality.



# Features

* Supported metadata
    * HDR primaries & whitepoint as CIE1931 xy coordinates
	* HDR min/max mastering luminance
	* HDR content light levels (MaxCLL, MaxFALL)
	* HDR Transfer Matrix (BT.2020 etc)
	* HDR Primaries (DCI-P3, BT.2020 etc)
	* HDR Video transfer function (SMPTE ST 2084 (PQ), HLG etc)
* Configuration through an .ini file (by default in same directory)
* Tested players
    * Potplayer
* Tested DirectShow sources
    * AVerMedia Live Gamer 4K - GC573
* Tested DirectShow renderers
    * madVR up to beta 131
    
    

# Install

* Install VS2019 x64 runtime

* Download directshow_metadata_injector_filter.ax to a location you will not delete (and which is not admin only under windows like program files)

* Copy the example_config.ini as directshow_metadata_injector_filter.ini to the same dir

  

**Potplayer**

* Open potplayer
* Open preferences (F5)
* In the tree menu go to: Filter Control -> Filter Priority (Overall) ([screenshot](images/potplayer_settings.png))
* Click Add external filter
* Go to the directory where you downloaded the directshow_metadata_injector_filter.ax, select the file and press open
* There should be a new filter in the Custom Filter Manager called "Metadata Injector"
* Select Metadata Injector
* Bottom right of screen set priority to "Prefer"
*(If you're running a AVerMedia Live Gamer 4K - GC573 don't forget to set it's output to P010 for 10bit, you can do this in preferences under device->webcam->format, see [here](images/potplayer_avermedia_settings.png))*



# Configuration

Configuration is via a .ini file which contains all the settings. There is an example_config.ini in the project's code repo. Copy that to the same location as where you downloaded the .ax file and rename to directshow_metadata_injector_filter.ini. 

The settings are well documented and speak for themselves.



# HDFury Virtex2 auto config generator

If you have a HDFury Virtex2 you can automate the metadata generation. For this to work you need
to place the Vertex2 before the input to the capture server (you want to do this anyway because higher end audio is notoriously poor though such setups). This way the device will see the same HDR meta data as the capture card and the filter can inject the correct metadata in the DirectShow stream. 

The config is re-generated every few seconds and a quick stop-start will reload everything and get you the right parameters (after a menu-switch or a new movie starts for example).

Installation and use:

* Install Python
* Install python/requirements.txt modules
* Run hdfury_virtex2_ini_generator.py <ip of vertex2> <directshow_metadata_injector_filter.ini file from above>




# Build

**C++**: Open with Visual Studio 2019 VC++, should just work out of the box.

**Python**: Just have a python 3.x interpretor and install the packages mentioned in requirements.txt



# FAQ

**Is this plug and play?** 
No, because this filter does not offer anything new to get an video stream decoded it will not automatically be added to a DirectShow graph. It needs to be forced to be included, see install for an example.

**After starting the video the screen remains black**
This happens if there is another filter in between this filter and madVR. At that point the filter can't get memory for sending the metadata and will not copy the image data as well. Fix your graph/player.



# Credits

 * Thanks to WP for the initial introduction to DirectShow 
 * Thanks to Hendrik Leppkes' for his excellent [LAV](https://github.com/Nevcairiel/LAVFilters/releases) filter.
 * Thanks to madshi for [madVR](http://madvr.com/).

No thanks to all capture card manufacturers who are willing to sell you half implemented software on their hardware.



# License & legal

This application is released under the MIT license see LICENSE. 

I'm not affiliated or connected with any of the firms mentioned above. 



------

 Copyright 2021 [Dennis Fleurbaaij](mailto:mail@dennisfleurbaaij.com)

