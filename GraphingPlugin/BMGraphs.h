#pragma once
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include "bakkesmod\wrappers\gamewrapper.h"
#include "bakkesmod\wrappers\cvarmanagerwrapper.h"

/*	WARNING!!!!!!
*	Make a backup of this file if you have made any changes.
*	This file will be automatically overwritten if there are any updates
*/

class BMGraphs
{
private:
	int		graphTop = 0;
	int		graphLeft = 0;
	int		graphWidth = 0;
	int		graphHeight = 0;
	int		graphSteps = 0;
	float	graphBarThickness = 0.67f;
	float	graphMin = 0.0f;
	float	graphMax = 0.0f;
	float	graphRangeOffset = 0.0f;
	float	graphOpacity = 1.0f;
	bool	shouldDrawVertices;
	bool	sliderUnlock;
	bool	canRender = false;

	vector<vector<float>>	GraphData;
	vector<string>			GraphLabels;
	vector<LinearColor>		GraphColors;

	//LINE, BAR, MULTIBAR, SPLITBAR
	//Instructions about each graph style are in BMGraphs.cpp in the GraphRender function
	string	graphStyle = "LINE";
	string	graphTitle = "";
	int		numObjects = 0;

	void GraphRender(CanvasWrapper canvas);
	inline int ClampToGraph(float valueToClamp);
	inline Vector2 GraphPlotPoint(int i, float dataPoint);
	inline float GraphFindY(float dataPoint);


public:
	void Setup(std::shared_ptr<CVarManagerWrapper> CVMW, std::shared_ptr<GameWrapper> GW);
	void BeginRender(string title, string graphType);
	void EndRender();
	void InputData(vector<float> data, vector<string> labels, vector<LinearColor> colors, int numBars);//numBars only matters for the MULTIBAR type

	std::shared_ptr<CVarManagerWrapper> cvarManager;
	std::shared_ptr<GameWrapper> gameWrapper;
};
