#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "BMGraphs.h"

#define NOTIFIER_START_TEST "GRAPHTEST"
#define NOTIFIER_END_TEST "GRAPHTESTEND"

class GraphingPluginExamples : public BakkesMod::Plugin::BakkesModPlugin
{
private:
    std::shared_ptr<BMGraphs> graphs;

public:
    void onLoad() override;
    void onUnload() override;
    bool ShouldRun();

    void GenerateSettingsFile();
    
    void GraphTestBegin(std::vector<std::string> params);
    void GraphTestEnd();

    void InputData(EGraphType GraphType);
    void InputLineGraphData();
    void InputBarGraphData();
    void InputMultibarGraphData();
    void InputSplitBarGraphData();
};
