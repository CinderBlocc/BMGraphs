#include "GraphingPluginExamples.h"
#include "BMGraphsMacrosStructsEnums.h"
#include <fstream>
#include <filesystem>

#define nl(x) setFile << x << '\n'
#define blank setFile << '\n'
#define cv(x) std::string(x)

void GraphingPluginExamples::GenerateSettingsFile()
{
    //Remove the old example settings file to avoid conflicts
    if(std::filesystem::exists("./bakkesmod/plugins/settings/GraphingPluginExamples.set"))
    {
        std::filesystem::remove("./bakkesmod/plugins/settings/GraphingPluginExamples.set");
    }

    std::ofstream setFile("./bakkesmod/plugins/settings/Y_GraphingPluginExamples_Y.set");

    nl("Graphing Examples");
    blank;
    nl("0|Line|"     + cv(NOTIFIER_START_TEST) + " 0; " + cv(CVAR_MAX_VALUE) + " 6000; "  + cv(CVAR_REFERENCE_STEPS) + " 3; " + cv(CVAR_POSITION_TOP) + " 75; " + cv(CVAR_POSITION_LEFT) + " 120");
    nl("7|");
    nl("9|Car and Ball Location Over Time");
    blank;
    blank;
    nl("0|Bar|"      + cv(NOTIFIER_START_TEST) + " 1; " + cv(CVAR_MAX_VALUE) + " 10000; " + cv(CVAR_REFERENCE_STEPS) + " 3; " + cv(CVAR_POSITION_TOP) + " 75; " + cv(CVAR_POSITION_LEFT) + " 50");
    nl("7|");
    nl("9|Distance Between Car and Ball");
    blank;
    blank;
    nl("0|Multibar|" + cv(NOTIFIER_START_TEST) + " 2; " + cv(CVAR_MAX_VALUE) + " 200; "   + cv(CVAR_REFERENCE_STEPS) + " 1; " + cv(CVAR_POSITION_TOP) + " 75; " + cv(CVAR_POSITION_LEFT) + " 300");
    nl("7|");
    nl("9|Wheel Info");
    blank;
    blank;
    nl("0|Splitbar|" + cv(NOTIFIER_START_TEST) + " 3; " + cv(CVAR_MAX_VALUE) + " 4500; "  + cv(CVAR_REFERENCE_STEPS) + " 3; " + cv(CVAR_POSITION_TOP) + " 75; " + cv(CVAR_POSITION_LEFT) + " 50");
    nl("7|");
    nl("9|Car and Ball Velocity");
    blank;
    blank;
    nl("9|");
    nl("0|STOP TEST|" + cv(NOTIFIER_END_TEST));

    setFile.close();

    cvarManager->executeCommand("cl_settings_refreshplugins");
}
