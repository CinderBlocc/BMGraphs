#pragma once
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include "BMGraphsMacrosStructsEnums.h"

/*
*    WARNING!!!!!!
*    Make a backup of this file if you have made any changes.
*    This file will be automatically overwritten if there are any updates.
*/

class BMGraphs
{
public:
    BMGraphs(std::shared_ptr<CVarManagerWrapper> InCvarManager, std::shared_ptr<GameWrapper> InGameWrapper);

    //Start and Stop graph rendering
    void BeginRender(const GraphInitData& InInitData);
    void EndRender();

    //Data input functions
    void InputData(const std::vector<LineGraphDataSingle>& InData);
    void InputData(const std::vector<BarGraphData>& InData);
    void InputData(const std::vector<MultibarGraphData>& InData);
    void InputData(const std::vector<SplitbarGraphData>& InData);


private:
    BMGraphs() = default;
    std::shared_ptr<CVarManagerWrapper> cvarManager;
    std::shared_ptr<GameWrapper> gameWrapper;

    //Labels, colors, etc
    GraphInitData InitData;

    //Data for each graph type
    std::vector<LineGraphData>     LineData;
    std::vector<BarGraphData>      BarData;
    std::vector<MultibarGraphData> MultibarData;
    std::vector<SplitbarGraphData> SplitbarData;

    //Appearance variables
    int     PositionTop = 0;
    int     PositionLeft = 0;
    int     Width = 0;
    int     Height = 0;
    int     Steps = 0;
    int     MinMaxPrecision = 2;
    int     ReferenceSteps = 0;
    float   BarThickness = .67f;
    float   MinValue = 0.f;
    float   MaxValue = 0.f;
    float   RangeShift = 0.f;
    float   Opacity = 1.f;
    float   BackgroundOpacity = .4f;
    float   LabelSize = 1.f;
    bool    bDrawVertices = false;
    bool    bSliderLockInverted = false;
    bool    bWrapText = false;
    bool    bTextShadow = false;
    bool    bShouldRender = true;
    bool    bShouldAnimate = true;

    //Rendering functions
    void Render(CanvasWrapper canvas);
    void DrawGraphBase(CanvasWrapper canvas)     const;
    void DrawReferences(CanvasWrapper canvas)    const;
    void DrawGraphOutline(CanvasWrapper canvas)  const;
    void DrawLineGraph(CanvasWrapper canvas)     const;
    void DrawBarGraph(CanvasWrapper canvas)      const;
    void DrawMultibarGraph(CanvasWrapper canvas) const;
    void DrawSplitbarGraph(CanvasWrapper canvas) const;

    //Utility functions
    LinearColor GetOpacityColor(const LinearColor& InColor)      const;
    int ClampToGraph(const float InValue)                        const;
    int GraphFindY(const float InDataPoint)                      const;
    Vector2 GraphPlotPoint(int StepNumber, float InDataPoint)    const;
    void DrawVertex(CanvasWrapper canvas, Vector2 Location)      const;
    std::string PrecisionString(float InValue, int Precision)    const;
    float GetReferenceLineStart(float InMinValue, float RefStep) const;

    //Validation functions
    bool ShouldAnimate();
    bool CheckIfValidCvars(const std::vector<std::string>& CvarList) const;
    bool UpdateValuesFromCvars();
};
