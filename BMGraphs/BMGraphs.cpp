#include "BMGraphs.h"
#include <sstream>
#include <iomanip>

/*
*    WARNING!!!!!!
*    Make a backup of this file if you have made any changes.
*    This file will be automatically overwritten if there are any updates.
*/

BMGraphs::BMGraphs(std::shared_ptr<CVarManagerWrapper> InCvarManager, std::shared_ptr<GameWrapper> InGameWrapper)
{
    cvarManager = InCvarManager;
    gameWrapper = InGameWrapper;

    cvarManager->executeCommand("plugin load graphingplugin; sleep 10; cl_settings_refreshplugins");
}



// START AND STOP GRAPH RENDERING //
void BMGraphs::BeginRender(const GraphInitData& InInitData)
{
    //End other drawings before starting this new one
    EndRender();

    InitData = InInitData;

    //gameWrapper->RegisterDrawable(bind(&BMGraphs::Render, this, std::placeholders::_1));
}

void BMGraphs::EndRender()
{
    //gameWrapper->UnregisterDrawables();

    LineData.clear();
    BarData.clear();
    MultibarData.clear();
    SplitbarData.clear();
}



// DATA INPUT FUNCTIONS //
void BMGraphs::InputData(const std::vector<LineGraphDataSingle>& InData)
{
    if(ShouldAnimate())
    {
        LineData.push_back(LineGraphData{InData});
        while(LineData.size() > Steps)
        {
            LineData.erase(LineData.begin());
        }
    }
}

void BMGraphs::InputData(const std::vector<BarGraphData>& InData)
{
    if(ShouldAnimate())
    {
        BarData = InData;
    }
}

void BMGraphs::InputData(const std::vector<MultibarGraphData>& InData)
{
    if(ShouldAnimate())
    {
        MultibarData = InData;
    }
}

void BMGraphs::InputData(const std::vector<SplitbarGraphData>& InData)
{
    if(ShouldAnimate())
    {
        SplitbarData = InData;
    }
}



// RENDERING FUNCTIONS //
void BMGraphs::Render(CanvasWrapper canvas)
{
    //Get data from GraphingPlugin cvars
    if(!UpdateValuesFromCvars()) { return; }

    //Check if graph should be rendered
    if(!bShouldRender) { return; }

    //Draw title and background of graph
    DrawGraphBase(canvas);

    //Draw all the reference lines and labels
    DrawReferences(canvas);

    //Choose graphing function
    switch(InitData.Type)
    {
        case EGraphType::GRAPH_Line:
            DrawLineGraph(canvas);
            break;
        case EGraphType::GRAPH_Bar:
            DrawBarGraph(canvas);
            break;
        case EGraphType::GRAPH_Multibar:
            DrawMultibarGraph(canvas);
            break;
        case EGraphType::GRAPH_Splitbar:
            DrawSplitbarGraph(canvas);
            break;
        default:
            break;
    }

    //Draw outline to close off graph borders
    DrawGraphOutline(canvas);
}

void BMGraphs::DrawGraphBase(CanvasWrapper canvas) const
{
    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    //Draw title centered above graph
    Vector2F TitleSizeF = canvas.GetStringSize(InitData.Title, LabelSize, LabelSize);
    Vector2 TitleSize = Vector2{static_cast<int>(TitleSizeF.X), static_cast<int>(TitleSizeF.Y)};
    Vector2 TitlePosition = TopLeft + Vector2{Width / 2, 0} - Vector2{TitleSize.X / 2, TitleSize.Y};
    canvas.SetColor(LinearColor{255, 255, 255, 255});
    canvas.SetPosition(TitlePosition);
    canvas.DrawString(InitData.Title, LabelSize, LabelSize, bTextShadow, bWrapText);

    //Draw background box
    canvas.SetColor(LinearColor{100, 100, 100, 255 * BackgroundOpacity * Opacity});
    canvas.SetPosition(TopLeft);
    canvas.FillBox(GraphSize);
}

void BMGraphs::DrawGraphOutline(CanvasWrapper canvas) const
{
    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    canvas.SetColor(LinearColor{255, 255, 255, 255 * Opacity});
    canvas.SetPosition(TopLeft - Vector2{2, 2});
    canvas.DrawBox(GraphSize + Vector2{4, 4});
}

void BMGraphs::DrawReferences(CanvasWrapper canvas) const
{
    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    //For line graph, use true minimum value. For other graphs (bars), use 0 as minimum
    float TempMinValue = InitData.Type == EGraphType::GRAPH_Line ? MinValue : 0;

    //Draw reference lines and labels
    LinearColor RefLabelColor = {255, 255, 255, 255 * (Opacity * .75f)};
    LinearColor RefLineColor = {255, 255, 255, 255 * (Opacity * .25f)};
    float RefStep = powf(10.f, static_cast<float>(ReferenceSteps));
    int ReferenceLabelPrecision = ReferenceSteps < 0 ? abs(ReferenceSteps) : 0;
    float CurrentRefLine = GetReferenceLineStart(MinValue, RefStep);
    int NumRefLinesDrawn = 0;
    bool bDrawnTooManyRefs = false;
    while(CurrentRefLine < MaxValue)
    {
        if(CurrentRefLine <= TempMinValue)
        {
            CurrentRefLine += RefStep;
            continue;
        }

        std::string RefLabel = PrecisionString(CurrentRefLine, ReferenceLabelPrecision);
        Vector2F RefLabelSize = canvas.GetStringSize(RefLabel);

        Vector2 RefLineStart = TopLeft;
        Vector2 RefLineEnd = TopLeft;
        Vector2 RefLabelPosition = TopLeft;
        switch(InitData.Type)
        {
            case EGraphType::GRAPH_Line:
            {
                RefLineStart = GraphPlotPoint(0, CurrentRefLine);
                RefLineEnd = GraphPlotPoint(Steps - 1, CurrentRefLine);
                RefLabelPosition = RefLineStart - Vector2{static_cast<int>(RefLabelSize.X) + 5, static_cast<int>(RefLabelSize.Y / 2)};

                break;
            }

            case EGraphType::GRAPH_Splitbar:
            {
                int HorizontalPosition = static_cast<int>((CurrentRefLine / MaxValue) * (Width / 2));
                Vector2 Midline = TopLeft + Vector2{Width / 2, 0};
                Vector2 LineStartOffset = Vector2{HorizontalPosition, Height};
                Vector2 LineEndOffset = Vector2{HorizontalPosition, 0};

                //Right reference lines
                RefLineStart = Midline + LineStartOffset;
                RefLineEnd = Midline + LineEndOffset;
                RefLabelPosition = RefLineStart - Vector2{static_cast<int>(RefLabelSize.X / 2), 0};

                //Left reference lines
                canvas.SetColor(RefLabelColor);
                canvas.SetPosition(Midline + Vector2{-HorizontalPosition, Height} - Vector2{static_cast<int>(RefLabelSize.X / 2), 0});
                canvas.DrawString(RefLabel);
                canvas.SetColor(RefLineColor);
                canvas.DrawLine(Midline + Vector2{-HorizontalPosition, Height}, Midline + Vector2{-HorizontalPosition, 0});
                ++NumRefLinesDrawn;

                break;
            }

            case EGraphType::GRAPH_Bar: //Intentional fallthrough
            case EGraphType::GRAPH_Multibar:
            {
                //Use max bar width value
                int HorizontalPosition = static_cast<int>((CurrentRefLine / MaxValue) * Width);
                RefLineStart = TopLeft + Vector2{HorizontalPosition, Height};
                RefLineEnd = TopLeft + Vector2{HorizontalPosition, 0};
                RefLabelPosition = RefLineStart - Vector2{static_cast<int>(RefLabelSize.X / 2), 0};

                break;
            }

            default:
            {
                break;
            }
        }
        canvas.SetColor(RefLabelColor);
        canvas.SetPosition(RefLabelPosition);
        canvas.DrawString(RefLabel);
        canvas.SetColor(RefLineColor);
        canvas.DrawLine(RefLineStart, RefLineEnd);

        CurrentRefLine += RefStep;

        ++NumRefLinesDrawn;
        if(NumRefLinesDrawn > 150)
        {
            bDrawnTooManyRefs = true;
            break;
        }
    }

    //Draw Min and Max value labels
    canvas.SetColor(LinearColor{255, 255, 255, 255 * Opacity});
    std::string MinValueString = PrecisionString(TempMinValue, MinMaxPrecision);
    std::string MaxValueString = PrecisionString(MaxValue, MinMaxPrecision);
    Vector2F MinValueStringSize = canvas.GetStringSize(MinValueString, LabelSize, LabelSize);
    Vector2F MaxValueStringSize = canvas.GetStringSize(MaxValueString, LabelSize, LabelSize);
    Vector2 MinValueLabelPosition = TopLeft;
    Vector2 MaxValueLabelPosition = TopLeft;
    switch(InitData.Type)
    {
        case EGraphType::GRAPH_Line:
        {
            //Max at top left, Min at bottom left
            MaxValueLabelPosition = TopLeft - Vector2{static_cast<int>(MaxValueStringSize.X) + 5, static_cast<int>(MaxValueStringSize.Y)};
            MinValueLabelPosition = TopLeft - Vector2{static_cast<int>(MinValueStringSize.X) + 5, -Height};
            break;
        }

        case EGraphType::GRAPH_Splitbar:
        {
            //Max at bottom left, Min at bottom center, Max2 at bottom right
            Vector2 MaxValueLabelPosition2;
            MaxValueLabelPosition  = TopLeft + Vector2{0,         Height + 10} - Vector2{static_cast<int>(MaxValueStringSize.X / 2), 0};
            MinValueLabelPosition  = TopLeft + Vector2{Width / 2, Height + 10} - Vector2{static_cast<int>(MinValueStringSize.X / 2), 0};
            MaxValueLabelPosition2 = TopLeft + Vector2{Width,     Height + 10} - Vector2{static_cast<int>(MaxValueStringSize.X / 2), 0};
            canvas.SetPosition(MaxValueLabelPosition2);
            canvas.DrawString(MaxValueString, LabelSize, LabelSize, bTextShadow, bWrapText);
            break;
        }

        case EGraphType::GRAPH_Bar: //Intentional fallthrough
        case EGraphType::GRAPH_Multibar:
        {
            //Min at bottom left, Max at bottom right
            MinValueLabelPosition = TopLeft + Vector2{0,     Height + 10} - Vector2{static_cast<int>(MinValueStringSize.X / 2), 0};
            MaxValueLabelPosition = TopLeft + Vector2{Width, Height + 10} - Vector2{static_cast<int>(MaxValueStringSize.X / 2), 0};
            break;
        }

        default:
        {
            break;
        }
    }
    canvas.SetPosition(MinValueLabelPosition);
    canvas.DrawString(MinValueString, LabelSize, LabelSize, bTextShadow, bWrapText);
    canvas.SetPosition(MaxValueLabelPosition);
    canvas.DrawString(MaxValueString, LabelSize, LabelSize, bTextShadow, bWrapText);

    //If there were too many reference lines, draw a warning
    //Draw after Min/Max value labels so it's guaranteed to show on top
    if(bDrawnTooManyRefs)
    {
        static const std::string ReferenceWarningMessage = "TOO MANY REFERENCE LINES! Reduce range, or increase reference lines steps value.";
        static const Vector2F WarningMessageSize = canvas.GetStringSize(ReferenceWarningMessage);
        
        Vector2 WarningMessagePosition = TopLeft + Vector2{Width / 2, Height} - Vector2{static_cast<int>(WarningMessageSize.X / 2), 0};
        if(InitData.Type != EGraphType::GRAPH_Line)
        {
            //Move warning below labels
            WarningMessagePosition.Y += 25;
        }

        canvas.SetColor(LinearColor{255, 0, 0, 255 * Opacity});
        canvas.SetPosition(WarningMessagePosition);
        canvas.DrawString(ReferenceWarningMessage, 1, 1, bTextShadow, bWrapText);
    }
}

void BMGraphs::DrawLineGraph(CanvasWrapper canvas) const
{
    if(LineData.empty()) { return; }

    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    //Draw horizontal line at 0
    canvas.SetColor(LinearColor{255, 255, 255, 255 * Opacity});
    canvas.DrawLine(GraphPlotPoint(0, 0), GraphPlotPoint(Steps - 1, 0), 2);

    //Draw the data lines
    //NOTE: This does not use range-based-for because it needs to plot both this step and the next step
    for(size_t Step = 0; Step < LineData.size(); ++Step)
    {
        const LineGraphData StepData = LineData[Step];
        if(StepData.Lines.empty()) { continue; }

        //Iterate through each line at the current time step
        for(size_t LineIndex = 0; LineIndex < StepData.Lines.size(); ++LineIndex)
        {
            const LineGraphDataSingle ThisDataPoint = StepData.Lines[LineIndex];

            //Check if this line's label matches a graph label
            //If it matches a label, draw a line from this point to the next point
            for(const auto& Label : InitData.Labels)
            {
                if(ThisDataPoint.Label == Label.Text)
                {
                    canvas.SetColor(GetOpacityColor(Label.Color));
                    Vector2 ThisPlottedPoint = GraphPlotPoint(static_cast<int>(Step), ThisDataPoint.Value);
                    Vector2 NextPlottedPoint = ThisPlottedPoint;

                    if(Step < LineData.size() - 1)
                    {
                        NextPlottedPoint = GraphPlotPoint(static_cast<int>(Step + 1), LineData[Step + 1].Lines[LineIndex].Value);
                    }

                    canvas.DrawLine(ThisPlottedPoint, NextPlottedPoint);

                    if(bDrawVertices)
                    {
                        DrawVertex(canvas, NextPlottedPoint);
                    }

                    break;
                }
            }
        }
    }

    //Draw labels
    Vector2 LabelPosition = TopLeft + Vector2{Width + 10, 0};
    for(const auto& Label : InitData.Labels)
    {
        canvas.SetColor(GetOpacityColor(Label.Color));
        canvas.SetPosition(LabelPosition);
        canvas.DrawString(Label.Text, LabelSize, LabelSize, bTextShadow, bWrapText);

        Vector2F LabelTextSize = canvas.GetStringSize(Label.Text, LabelSize, LabelSize);

        LabelPosition.Y += static_cast<int>(LabelTextSize.Y);
    }
}

void BMGraphs::DrawBarGraph(CanvasWrapper canvas) const
{
    if(BarData.empty()) { return; }
    
    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    //The number of spaces between the bars
    int NumSpaces = static_cast<int>(BarData.size());
    int HeightPerBar = Height / NumSpaces;
    
    //Iterate through each bar
    int BarIndex = 0;
    for(const auto& Bar : BarData)
    {
        //Check if this bar's label matches a graph label
        for(const auto& Label : InitData.Labels)
        {
            if(Bar.Label == Label.Text)
            {
                //Clamp bar within graph limits
                float Value = min(Bar.Value, MaxValue);

                //Positioning magic
                int ThisBarMidline = PositionTop + (HeightPerBar * (BarIndex + 1)) - (Height / (NumSpaces * 2));
                int ThisBarThickness = static_cast<int>((Height / (NumSpaces * 2.f)) * BarThickness);

                //Draw bar
                canvas.SetColor(GetOpacityColor(Label.Color));
                canvas.SetPosition(Vector2{PositionLeft, ThisBarMidline - ThisBarThickness});
                canvas.FillBox(Vector2{static_cast<int>((Value / MaxValue) * Width), ThisBarThickness * 2});

                //Draw label
                Vector2F LabelTextSize = canvas.GetStringSize(Label.Text, LabelSize, LabelSize);
                canvas.SetPosition(Vector2{PositionLeft + Width + 10, ThisBarMidline - static_cast<int>(LabelTextSize.Y / 2)});
                canvas.DrawString(Label.Text, LabelSize, LabelSize, bTextShadow, bWrapText);

                break;
            }
        }
        ++BarIndex;
    }
}

void BMGraphs::DrawMultibarGraph(CanvasWrapper canvas) const
{
    if(MultibarData.empty()) { return; }
    if(MultibarData[0].ValueGroup.empty()) { return; }

    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};

    int NumObjects = static_cast<int>(MultibarData[0].ValueGroup.size());
    int NumSpaces = static_cast<int>(MultibarData.size());
    int HeightPerGroup = Height / NumSpaces;

    //Iterate through each group of bars
    int GroupIndex = 0;
    for(const auto& BarGroup : MultibarData)
    {
        int ThisGroupMidline = PositionTop + (HeightPerGroup * (GroupIndex + 1)) - (Height / (NumSpaces * 2));
        int ThisGroupThickness = static_cast<int>((Height / (NumSpaces * 2.f)) * BarThickness);
        int IndividualBarThickness = (ThisGroupThickness * 2) / NumObjects;

        //Iterate through each bar in this group
        int BarIndex = 0;
        for(const auto& Bar : BarGroup.ValueGroup)
        {
            //Check if this bar's label matches a graph label
            for(const auto& Label : InitData.Labels)
            {
                if(Bar.Label == Label.Text)
                {
                    float Value = min(Bar.Value, MaxValue);

                    canvas.SetColor(GetOpacityColor(Label.Color));
                    canvas.SetPosition(Vector2{PositionLeft, ThisGroupMidline - ThisGroupThickness + (IndividualBarThickness * BarIndex)});
                    canvas.FillBox(Vector2{static_cast<int>((Value / MaxValue) * Width), IndividualBarThickness});

                    break;
                }
            }

            ++BarIndex;
        }

        //Draw value label
        canvas.SetColor(LinearColor{255, 255, 255, 255 * Opacity});
        Vector2F LabelTextSize = canvas.GetStringSize(BarGroup.ValueLabel, LabelSize, LabelSize);
        canvas.SetPosition(Vector2{PositionLeft - static_cast<int>(LabelTextSize.X) - 10, ThisGroupMidline - static_cast<int>(LabelTextSize.Y / 2)});
        canvas.DrawString(BarGroup.ValueLabel, LabelSize, LabelSize, bTextShadow, bWrapText);
        

        ++GroupIndex;
    }

    //Draw object labels
    Vector2 LabelPosition = TopLeft + Vector2{Width + 10, 0};
    for(const auto& Label : InitData.Labels)
    {
        canvas.SetColor(GetOpacityColor(Label.Color));
        canvas.SetPosition(LabelPosition);
        canvas.DrawString(Label.Text, LabelSize, LabelSize, bTextShadow, bWrapText);

        Vector2F LabelTextSize = canvas.GetStringSize(Label.Text, LabelSize, LabelSize);

        LabelPosition.Y += static_cast<int>(LabelTextSize.Y);
    }
}

void BMGraphs::DrawSplitbarGraph(CanvasWrapper canvas) const
{
    if(SplitbarData.empty()) { return; }

    Vector2 TopLeft = {PositionLeft, PositionTop};
    Vector2 GraphSize = {Width, Height};
    
    int Midline = PositionLeft + (Width / 2);
    int MaxBarLength = Midline - PositionLeft;
    int NumSpaces = static_cast<int>(SplitbarData.size());
    int HeightPerBar = Height / NumSpaces;
        
    //Iterate through each bar
    int BarIndex = 0;
    for(const auto& Bar : SplitbarData)
    {
        //Check if this bar's label matches a graph label
        for(const auto& Label : InitData.Labels)
        {
            if(Bar.Label == Label.Text)
            {
                //Clamp bars within graph limits
                float ValueA = min(Bar.ValueA, MaxValue);
                float ValueB = min(Bar.ValueB, MaxValue);

                //Positioning magic
                int ThisBarMidline = PositionTop + (HeightPerBar * (BarIndex + 1)) - (Height / (NumSpaces * 2));
                int ThisBarThickness = static_cast<int>((Height / (NumSpaces * 2.f)) * BarThickness);

                canvas.SetColor(GetOpacityColor(Label.Color));

                //Draw left bar
                int BarALength = static_cast<int>((ValueA / MaxValue) * MaxBarLength);
                canvas.SetPosition(Vector2{Midline - BarALength, ThisBarMidline - ThisBarThickness});
                canvas.FillBox(Vector2{BarALength, ThisBarThickness * 2});

                //Draw right bar
                int BarBLength = static_cast<int>((ValueB / MaxValue) * MaxBarLength);
                canvas.SetPosition(Vector2{Midline, ThisBarMidline - ThisBarThickness});
                canvas.FillBox(Vector2{BarBLength, ThisBarThickness * 2});

                //Draw label
                Vector2F LabelTextSize = canvas.GetStringSize(Label.Text, LabelSize, LabelSize);
                canvas.SetPosition(Vector2{PositionLeft + Width + 10, ThisBarMidline - static_cast<int>(LabelTextSize.Y / 2)});
                canvas.DrawString(Label.Text, LabelSize, LabelSize, bTextShadow, bWrapText);

                break;
            }
        }
        ++BarIndex;
    }

    canvas.SetColor(LinearColor{255, 255, 255, 255 * Opacity});
    
    //Draw midline
    canvas.DrawLine(Vector2{Midline, PositionTop}, Vector2{Midline, PositionTop + Height}, 2);

    //Draw the A and B labels
    Vector2F LabelALength = canvas.GetStringSize(InitData.SplitbarAName, LabelSize, LabelSize);
    Vector2 LabelAPosition = TopLeft + Vector2{0, 5} + Vector2{Width / 4, 0} - Vector2{static_cast<int>(LabelALength.X / 2), 0};
    canvas.SetPosition(LabelAPosition);
    canvas.DrawString(InitData.SplitbarAName, LabelSize, LabelSize, bTextShadow, bWrapText);

    Vector2F LabelBLength = canvas.GetStringSize(InitData.SplitbarBName, LabelSize, LabelSize);
    Vector2 LabelBPosition = TopLeft + Vector2{0, 5} + Vector2{(Width / 4) * 3, 0} - Vector2{static_cast<int>(LabelBLength.X / 2), 0};
    canvas.SetPosition(LabelBPosition);
    canvas.DrawString(InitData.SplitbarBName, LabelSize, LabelSize, bTextShadow, bWrapText);
}



// UTILITY FUNCTIONS //
LinearColor BMGraphs::GetOpacityColor(const LinearColor& InColor) const
{
    LinearColor OutColor = InColor;
    OutColor.A *= Opacity;

    return OutColor;
}

int BMGraphs::ClampToGraph(const float InValue) const
{
    // If InValue is above or below the graph range, clamp it back inside

    int OutValue = static_cast<int>(InValue);
    OutValue = max(OutValue, PositionTop);
    OutValue = min(OutValue, PositionTop + Height);

    return OutValue;
}

int BMGraphs::GraphFindY(const float InDataPoint) const
{
    // Find vertical position of data point on graph

    float ValuesVerticalRange = static_cast<float>(MaxValue - MinValue);
    float ScreenVerticalRange = static_cast<float>(Height) / ValuesVerticalRange;
    int GraphBottom = PositionTop + Height;

    return ClampToGraph(GraphBottom - ((InDataPoint - MinValue) * ScreenVerticalRange));
}

Vector2 BMGraphs::GraphPlotPoint(int StepNumber, float InDataPoint) const
{
    // Find X and Y screen position

    float HorizontalRange = static_cast<float>(Width) / (Steps - 1);
    float Xvalue = PositionLeft + HorizontalRange * StepNumber;

    return Vector2{static_cast<int>(Xvalue), GraphFindY(InDataPoint)};
}

void BMGraphs::DrawVertex(CanvasWrapper canvas, Vector2 Location) const
{
    //3 DrawLine calls is pretty much the most efficient way of doing this
    canvas.DrawLine(Location + Vector2{-1, -2}, Location + Vector2{-1, 1});
    canvas.DrawLine(Location + Vector2{ 0, -2}, Location + Vector2{ 0, 1});
    canvas.DrawLine(Location + Vector2{ 1, -2}, Location + Vector2{ 1, 1});
}

std::string BMGraphs::PrecisionString(float InValue, int Precision) const
{
    std::stringstream OutputStream;
    OutputStream << std::fixed << std::setprecision(Precision) << InValue;
    
    return OutputStream.str();
}

float BMGraphs::GetReferenceLineStart(float InMinValue, float RefStep) const
{
    float FirstReferenceValue = InMinValue;

    //Start at min and find the first refstep above that value
    if(ReferenceSteps >= 0)
    {
        //Handle as integer
        int MinValueInt = static_cast<int>(InMinValue);
        FirstReferenceValue = static_cast<float>(MinValueInt - (MinValueInt % static_cast<int>(RefStep)));
    }
    else
    {
        //Shift decimal to handle floats as integers
        int MinValueInt = static_cast<int>(InMinValue / RefStep);
        FirstReferenceValue = static_cast<float>(MinValueInt - (MinValueInt % static_cast<int>(1.f / RefStep)));
    }

    //Sometimes the first value is somehow less than the minimum
    //At least the starting increment is correct, so just move up within the range
    while(FirstReferenceValue < InMinValue)
    {
        FirstReferenceValue += RefStep;
    }

    return FirstReferenceValue;
}



// VALIDATION FUNCTIONS //
bool BMGraphs::ShouldAnimate()
{
    //This is a separate function because only this cvar matters for the InputData functions
    static const std::vector<std::string> Cvars =
    {
        CVAR_SHOULD_ANIMATE
    };

    if(!CheckIfValidCvars(Cvars)) { return false; }

    bShouldAnimate = cvarManager->getCvar(CVAR_SHOULD_ANIMATE).getBoolValue();

    return bShouldAnimate;
}

bool BMGraphs::CheckIfValidCvars(const std::vector<std::string>& CvarList) const
{
    bool bAllCvarsAreValid = true;
    for(const auto& Cvar : CvarList)
    {
        CVarWrapper CvarWrap = cvarManager->getCvar(Cvar);
        if(CvarWrap.IsNull())
        {
            //Don't return false immediately so that this can log all the null cvars instead of just the first one
            bAllCvarsAreValid = false;
            cvarManager->log(Cvar + " is a null cvar");
        }
    }

    return bAllCvarsAreValid;
}

bool BMGraphs::UpdateValuesFromCvars()
{
    static const std::vector<std::string> Cvars =
    {
        CVAR_POSITION_TOP,
        CVAR_POSITION_LEFT,
        CVAR_WIDTH,
        CVAR_HEIGHT,
        CVAR_STEPS,
        CVAR_MINMAX_PRECISION,
        CVAR_REFERENCE_STEPS,
        CVAR_BAR_THICKNESS,
        CVAR_MIN_VALUE,
        CVAR_MAX_VALUE,
        CVAR_RANGE_SHIFT,
        CVAR_OPACITY,
        CVAR_BACKGROUND_OPACITY,
        CVAR_LABEL_SIZE,
        CVAR_SHOW_VERTICES,
        CVAR_SLIDER_LINK_INVERT,
        CVAR_TEXT_SHADOW,
        CVAR_WRAP_TEXT,
        CVAR_SHOULD_RENDER,
        CVAR_SHOULD_ANIMATE
    };

    if(!CheckIfValidCvars(Cvars)) { return false; }

    PositionTop         = cvarManager->getCvar(CVAR_POSITION_TOP).getIntValue();
    PositionLeft        = cvarManager->getCvar(CVAR_POSITION_LEFT).getIntValue();
    Width               = cvarManager->getCvar(CVAR_WIDTH).getIntValue();
    Height              = cvarManager->getCvar(CVAR_HEIGHT).getIntValue();
    Steps               = cvarManager->getCvar(CVAR_STEPS).getIntValue();
    MinMaxPrecision     = cvarManager->getCvar(CVAR_MINMAX_PRECISION).getIntValue();
    ReferenceSteps      = cvarManager->getCvar(CVAR_REFERENCE_STEPS).getIntValue();
    BarThickness        = cvarManager->getCvar(CVAR_BAR_THICKNESS).getFloatValue();
    MinValue            = cvarManager->getCvar(CVAR_MIN_VALUE).getFloatValue();
    MaxValue            = cvarManager->getCvar(CVAR_MAX_VALUE).getFloatValue();
    RangeShift          = cvarManager->getCvar(CVAR_RANGE_SHIFT).getFloatValue();
    Opacity             = cvarManager->getCvar(CVAR_OPACITY).getFloatValue();
    BackgroundOpacity   = cvarManager->getCvar(CVAR_BACKGROUND_OPACITY).getFloatValue();
    LabelSize           = cvarManager->getCvar(CVAR_LABEL_SIZE).getFloatValue();
    bDrawVertices       = cvarManager->getCvar(CVAR_SHOW_VERTICES).getBoolValue();
    bSliderLockInverted = cvarManager->getCvar(CVAR_SLIDER_LINK_INVERT).getBoolValue();
    bTextShadow         = cvarManager->getCvar(CVAR_TEXT_SHADOW).getBoolValue();
    bWrapText           = cvarManager->getCvar(CVAR_WRAP_TEXT).getBoolValue();
    bShouldRender       = cvarManager->getCvar(CVAR_SHOULD_RENDER).getBoolValue();
    bShouldAnimate      = cvarManager->getCvar(CVAR_SHOULD_ANIMATE).getBoolValue();

    //Lock MinValue and MaxValue together, inverted
    if(bSliderLockInverted)
    {
        MaxValue = abs(MaxValue);
        MinValue = -MaxValue;
    }

    //Adjust the range based on the offset slider
    MinValue -= RangeShift;
    MaxValue -= RangeShift;

    //Make sure Min and Max are correct
    if(MinValue > MaxValue)
    {
        std::swap(MinValue, MaxValue);
    }

    return true;
}
