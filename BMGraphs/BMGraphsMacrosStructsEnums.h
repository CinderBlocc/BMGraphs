#pragma once



/*
*    WARNING!!!!!!
*    Make a backup of this file if you have made any changes.
*    This file will be automatically overwritten if there are any updates.
*/



// CVAR MACROS //

#define CVAR_POSITION_TOP        "Graph_Top"
#define CVAR_POSITION_LEFT       "Graph_Left"
#define CVAR_WIDTH               "Graph_Width"
#define CVAR_HEIGHT              "Graph_Height"
#define CVAR_MIN_VALUE           "Graph_MinVal"
#define CVAR_MAX_VALUE           "Graph_MaxVal"
#define CVAR_RANGE_SHIFT         "Graph_Adjust_Range"
#define CVAR_SLIDER_LINK_INVERT  "GraphToggleSliderInvert"
#define CVAR_STEPS               "Graph_MaxSteps"
#define CVAR_BAR_THICKNESS       "Graph_Bar_Thickness"
#define CVAR_OPACITY             "Graph_Opacity"
#define CVAR_BACKGROUND_OPACITY  "Graph_Background_Opacity"
#define CVAR_LABEL_SIZE          "Graph_Label_Size"
#define CVAR_SHOW_VERTICES       "GraphToggleVertices"
#define CVAR_SHOULD_RENDER       "GraphToggleRender"
#define CVAR_SHOULD_ANIMATE      "GraphTogglePlayback"
#define CVAR_WRAP_TEXT           "Graph_Wrap_Text"
#define CVAR_TEXT_SHADOW         "Graph_Text_Shadow"
#define CVAR_REFERENCE_STEPS     "Graph_Reference_Steps"
#define CVAR_MINMAX_PRECISION    "Graph_MinMax_Precision"



// ENUMS //

enum class EGraphType
{
    GRAPH_None = 0,
    GRAPH_Line,
    GRAPH_Bar,
    GRAPH_Multibar,
    GRAPH_Splitbar
};



// STRUCTS //

//Init
struct LabelInfo
{
    std::string Text;
    LinearColor Color;
};
struct GraphInitData
{
    //All graph types
    EGraphType Type;
    std::string Title;
    std::vector<LabelInfo> Labels;

    //Multibar
    std::vector<std::string> MultibarValueLabels;

    //Splitbar
    std::string SplitbarAName;
    std::string SplitbarBName;
};

//Line
struct LineGraphDataSingle
{
    std::string Label;
    float Value;
};
struct LineGraphData
{
    std::vector<LineGraphDataSingle> Lines;
};

//Bar
struct BarGraphData
{
    std::string Label;
    float Value;
};

//Multibar
struct MultibarGraphDataSingle
{
    std::string Label;
    float Value;
};
struct MultibarGraphData
{
    std::string ValueLabel;
    std::vector<MultibarGraphDataSingle> ValueGroup;
};

//Splitbar
struct SplitbarGraphData
{
    std::string Label;
    float ValueA;
    float ValueB;
};
