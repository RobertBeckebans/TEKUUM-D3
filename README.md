

This code is unsupported but it contains some goodies Doom 3 modders might want to cherry-pick:

* It has an older version of the RBDOOM-3-BFG renderer running on top of vanilla Doom 3 (good example how the API changes look like)
* There are several CMake configurations that allow to compile the engine for TC development, D3 compat mode with or without MFC tools
* It uses the BFG idlib
* It uses the BFG input system
* There are many changes to the Radiant to get it working with the BFG renderer
* It provides some changes to the Radiant to allow mapping in meters/centimeters instead of inches and Blender like shortcuts for viewport flipping. There are also some enhancements by Sikkpin
* It has an Android sytem layer. The idea was to compile the entire engine + game into a .so and let it load from a Android Java application through the Java native interface. I still have to lookup where the Java code is on my server. I stopped working on this in 2013.
* It contains somwhere a modified dmap which supports per vertex lighting baking like q3map1 which was supported by r_usePrecomputedLighting 1. The renderer also contains a Q3A style lightgrid in that mode. However full stencil shadow interactions worked fine on my mobile phone.
* It has an additional alternative UI code which lets you write GUIs in Lua instead of the .gui language. First I wanted to extend the .gui descripting language and then I figured how limited and unstable it was. Lua was a perfect fit so every windowDef can have its own Lua machine. It is very likely that I will port this code to RBDOOM-3-BFG because it is one of the best parts

There are Lua menu examples in base/guis/



