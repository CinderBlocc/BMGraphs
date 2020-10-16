#include "GraphingPlugin.h"
#include "BMGraphsMacrosStructsEnums.h"

BAKKESMOD_PLUGIN(GraphingPlugin, "Graphing Plugin", "2.0", PLUGINTYPE_FREEPLAY)

void GraphingPlugin::onLoad()
{
    //Toggle rendering and playback
    cvarManager->registerCvar(CVAR_SHOULD_RENDER,      "1",      "Toggle graph rendering", true, true, 0, true, 1);
    cvarManager->registerCvar(CVAR_SHOULD_ANIMATE,     "1",      "Toggle graph animation", true, true, 0, true, 1);

    //Position, scale, and range
    cvarManager->registerCvar(CVAR_POSITION_TOP,       "100",    "Graph offset from top",                true, true, 0,      true, 1500);
    cvarManager->registerCvar(CVAR_POSITION_LEFT,      "100",    "Graph offset from left",               true, true, 0,      true, 2000);
    cvarManager->registerCvar(CVAR_WIDTH,              "700",    "Graph width",                          true, true, 0,      true, 2000);
    cvarManager->registerCvar(CVAR_HEIGHT,             "350",    "Graph height",                         true, true, 0,      true, 1500);
    cvarManager->registerCvar(CVAR_MIN_VALUE,          "-6000",  "Min graph value floor",                true, true, -40000, true, 40000);
    cvarManager->registerCvar(CVAR_MAX_VALUE,          "6000",   "Max graph value ceiling",              true, true, -40000, true, 40000);
    cvarManager->registerCvar(CVAR_RANGE_SHIFT,        "0",      "Shift the graph range up or down",     true, true, -40000, true, 40000);
    cvarManager->registerCvar(CVAR_SLIDER_LINK_INVERT, "1",      "Toggle min/max slider inversion lock", true, true, 0,      true, 1);
    
    //Resolution: number of data points
    cvarManager->registerCvar(CVAR_STEPS,              "500",    "Number of steps to display in the graph", true, true, 0, true, 1000);
    
    //Appearance
    cvarManager->registerCvar(CVAR_REFERENCE_STEPS,    "3",      "Gap between each reference value", true, true, -1, true, 4);
    cvarManager->registerCvar(CVAR_MINMAX_PRECISION,   "1",      "Precision of min/max display",     true, true,  0, true, 4);
    cvarManager->registerCvar(CVAR_BAR_THICKNESS,      "0.67",   "Graph bar thickness",              true, true,  0, true, 1);
    cvarManager->registerCvar(CVAR_OPACITY,            "1",      "Graph opacity",                    true, true,  0, true, 1);
    cvarManager->registerCvar(CVAR_BACKGROUND_OPACITY, "0.4",    "Background opacity",               true, true,  0, true, 1);
    cvarManager->registerCvar(CVAR_LABEL_SIZE,         "2",      "Size of labels",                   true, true,  0, true, 4);
    cvarManager->registerCvar(CVAR_SHOW_VERTICES,      "0",      "Toggle graph vertices",            true, true,  0, true, 1);
    cvarManager->registerCvar(CVAR_WRAP_TEXT,          "0",      "Wrap text at edge of screen",      true, true,  0, true, 1);
    cvarManager->registerCvar(CVAR_TEXT_SHADOW,        "1",      "Draw dropshadow under text",       true, true,  0, true, 1);

    //Handle changes to slider link
    cvarManager->getCvar(CVAR_SLIDER_LINK_INVERT).addOnValueChanged(std::bind(&GraphingPlugin::OnLinkChanged, this));
    cvarManager->getCvar(CVAR_MAX_VALUE).addOnValueChanged(std::bind(&GraphingPlugin::OnLinkChanged, this));

    GenerateSettingsFile();
}
void GraphingPlugin::onUnload(){}

void GraphingPlugin::OnLinkChanged()
{
    if(cvarManager->getCvar(CVAR_SLIDER_LINK_INVERT).getBoolValue())
    {
        cvarManager->getCvar(CVAR_MIN_VALUE).setValue(-cvarManager->getCvar(CVAR_MAX_VALUE).getFloatValue());
    }
}
