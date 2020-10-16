# BMGraphs
Quick data visualization for plugin developers.

**NOTE: This code will not work if you do not have the GraphingPlugin installed: https://bakkesplugins.com/plugins/view/50**

This plugin takes away the headache of manually building graphs within BakkesMod. With this plugin and its source code, it is now very easy to display live data on screen. There are examples provided in the GraphingPluginExamples project that show how to set up the graphs.

You will need to include these 3 files in a plugin:
- BMGraphs.h
- BMGraphs.cpp
- BMGraphsMacrosStructsEnums.h

The "BMGraphs" class should be stored as a `shared_ptr` member variable in your plugin class, and should be constructed in the onLoad function.


