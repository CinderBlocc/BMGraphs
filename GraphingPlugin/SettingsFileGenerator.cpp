#include "GraphingPlugin.h"
#include "BMGraphsMacrosStructsEnums.h"
#include <fstream>

#define nl(x) setFile << x << '\n'
#define blank setFile << '\n'
#define cv(x) std::string(x)

void GraphingPlugin::GenerateSettingsFile()
{
    std::ofstream setFile("./bakkesmod/plugins/settings/Z_GraphingPlugin_Z.set");

    nl("GRAPHS");
    nl("1|Render|" + cv(CVAR_SHOULD_RENDER));
    nl("7|");
    nl("1|Animate|" + cv(CVAR_SHOULD_ANIMATE));
    blank;
    blank;
    nl("8|");
    blank;
    blank;
    nl("9|POSITION / SCALE / RANGE");
    nl("9|ctrl + left click to input a specific value");
    nl("5|Position Top|" + cv(CVAR_POSITION_TOP) + "|0|1500");
    nl("5|Position Left|" + cv(CVAR_POSITION_LEFT) + "|0|2000");
    nl("5|Width|" + cv(CVAR_WIDTH) + "|0|2000");
    nl("5|Height|" + cv(CVAR_HEIGHT) + "|0|1500");
    nl("1|Lock sliders inverted|" + cv(CVAR_SLIDER_LINK_INVERT));
    nl("10|!" + cv(CVAR_SLIDER_LINK_INVERT));
    nl("4|Min Value|" + cv(CVAR_MIN_VALUE) + "|-40000|40000");
    nl("11|");
    nl("4|Max Value|" + cv(CVAR_MAX_VALUE) + "|-40000|40000");
    nl("4|Range Shift|" + cv(CVAR_RANGE_SHIFT) + "|-40000|40000");
    blank;
    blank;
    nl("8|");
    blank;
    blank;
    nl("1|Draw vertices (line graph)|" + cv(CVAR_SHOW_VERTICES));
    nl("7|");
    nl("1|Wrap text|" + cv(CVAR_WRAP_TEXT));
    nl("7|");
    nl("1|Text shadow|" + cv(CVAR_TEXT_SHADOW));
    nl("5|Steps (line graph)|" + cv(CVAR_STEPS) + "|0|1000");
    nl("5|Min/Max Display Precision|" + cv(CVAR_MINMAX_PRECISION) + "|0|4");
    nl("5|Reference line steps|" + cv(CVAR_REFERENCE_STEPS) + "|-1|4");
    nl("4|Bar Thickness (bar graphs)|" + cv(CVAR_BAR_THICKNESS) + "|0|1");
    nl("4|Opacity|" + cv(CVAR_OPACITY) + "|0|1");
    nl("4|Background Opacity|" + cv(CVAR_BACKGROUND_OPACITY) + "|0|1");
    nl("4|Label Size|" + cv(CVAR_LABEL_SIZE) + "|0|4");

    setFile.close();

    cvarManager->executeCommand("cl_settings_refreshplugins");
}
