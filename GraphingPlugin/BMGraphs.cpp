#include "BMGraphs.h"

/*	WARNING!!!!!!
*	Make a backup of this file if you have made any changes.
*	This file will be automatically overwritten if there are any updates
*/

void BMGraphs::Setup(std::shared_ptr<CVarManagerWrapper> CVMW, std::shared_ptr<GameWrapper> GW)
{
	this->cvarManager = CVMW;
	this->gameWrapper = GW;

	cvarManager->executeCommand("plugin load graphingplugin; sleep 10; cl_settings_refreshplugins");
}

void BMGraphs::BeginRender(string title, string graphType)
{
	gameWrapper->UnregisterDrawables();//End any other rendering functions so this one can do its thing

	GraphData.clear();
	GraphLabels.clear();
	GraphColors.clear();

	graphTitle = title;
	graphStyle = graphType;

	this->canRender = false;
	gameWrapper->RegisterDrawable(bind(&BMGraphs::GraphRender, this, std::placeholders::_1));
}

void BMGraphs::EndRender()
{
	GraphData.clear();
	GraphLabels.clear();
	GraphColors.clear();

	gameWrapper->UnregisterDrawables();
}

void BMGraphs::InputData(vector<float> data, vector<string> labels, vector<LinearColor> colors, int numBars)//numBars only matters for the MULTIBAR type
{
	if(cvarManager->getCvar("GraphToggleRender").getBoolValue() && cvarManager->getCvar("GraphTogglePlayback").getBoolValue())
	{
		numObjects = numBars;
		graphSteps = cvarManager->getCvar("Graph_MaxSteps").getIntValue();

		//Fill vectors with temp data to avoid out of range errors (only fires if vectors are too small)
		while(GraphData.size() < data.size())
		{
			vector<float> tempData;
			GraphData.push_back(tempData);		
		}
		while(GraphLabels.size() < labels.size())
		{
			string tempString = "";
			GraphLabels.push_back(tempString);
		}
		while(GraphColors.size() < colors.size())
		{
			LinearColor tempColor = {255, 255, 255, 255};
			GraphColors.push_back(tempColor);
		}

		//Fill vectors with input data
		int graphDataSize = GraphData.size();
		for(int i=0; i<data.size(); i++)
		{
			if(i<graphDataSize)
			{
				if(i < labels.size())
					GraphLabels[i] = labels[i];
				if(i < colors.size())
					GraphColors[i] = colors[i];
				if(graphStyle.compare("LINE") == 0)
					GraphData[i].push_back(data[i]);
				else
				{
					if(GraphData[i].size() == 0)
						GraphData[i].push_back(0.0f);

					GraphData[i][0] = data[i];
				}
			}
		}

		//Delete old data on left from line graph to make room for new data on the right
		if(graphStyle.compare("LINE") == 0)
		{
			while (GraphData[0].size() > graphSteps)
			{
				for(int i=0; i<GraphData.size(); i++)
					GraphData[i].erase(GraphData[i].begin());
			}
		}

		this->canRender = true;
	}
}

void BMGraphs::GraphRender(CanvasWrapper canvas)
{
	if (cvarManager->getCvar("GraphToggleRender").getBoolValue() && canRender)
	{
		graphTop = cvarManager->getCvar("Graph_Top").getIntValue();
		graphLeft = cvarManager->getCvar("Graph_Left").getIntValue();
		graphWidth = cvarManager->getCvar("Graph_Width").getIntValue();
		graphHeight = cvarManager->getCvar("Graph_Height").getIntValue();
		graphBarThickness = cvarManager->getCvar("Graph_Bar_Thickness").getFloatValue();
		graphMin = cvarManager->getCvar("Graph_MinVal").getFloatValue();
		graphMax = cvarManager->getCvar("Graph_MaxVal").getFloatValue();
		graphRangeOffset = cvarManager->getCvar("Graph_Adjust_Range").getFloatValue();
		graphOpacity = cvarManager->getCvar("Graph_Opacity").getFloatValue() / 100.0f;
		shouldDrawVertices = cvarManager->getCvar("GraphToggleVertices").getBoolValue();
		sliderUnlock = cvarManager->getCvar("GraphToggleSliderInvert").getBoolValue();

		if(!sliderUnlock)//Lock min/max values inverted from each other
		{
			graphMax = abs(graphMax);
			graphMin = -graphMax;
		}

		//Adjust the range based on the offset slider
		graphMin -= graphRangeOffset;
		graphMax -= graphRangeOffset;

		//Draw title centered above graph
		int charWidth = 8;
		int titleSize = graphTitle.size();
		canvas.SetColor(255, 255, 255, 255);
		canvas.SetPosition(Vector2{graphLeft + ((graphWidth/2) - ((titleSize*charWidth)/2)), graphTop - 15});
		canvas.DrawString(graphTitle);



		/*	LINE
		*
		*	Each line gets one float, string, and color
		*	Refer to the InputData parameters to see the correct data types
		*	i.e.
		*		data.push_back(car.GetLocation().X);
		*		labels.push_back("Car X Location");
		*		colors.push_back(LinearColor{255,0,0,255});
		*/
		if(graphStyle.compare("LINE") == 0)
		{
			//Draw background box
			canvas.SetPosition(Vector2{graphLeft, graphTop});
			canvas.SetColor(100, 100, 100, 100 * graphOpacity);
			canvas.FillBox(Vector2{graphWidth, graphHeight});

			//Draw horizontal line at 0
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			int threshold = 0;
			Vector2 thresholdLineStart = {graphLeft , GraphFindY(threshold)};
			Vector2 thresholdLineEnd = {graphLeft + graphWidth , GraphFindY(threshold)};
			canvas.DrawLine(thresholdLineStart, thresholdLineEnd);

			//Draw data points
			for(int i=0; i<GraphData.size(); i++)
			{
				canvas.SetColor(GraphColors[i].R, GraphColors[i].G, GraphColors[i].B, GraphColors[i].A * graphOpacity);
				canvas.SetPosition(Vector2{graphLeft + 10, graphTop + 15 * i + 5});
				canvas.DrawString(GraphLabels[i]);

				for(int j=0; j<GraphData[0].size()-1; j++)
				{
					//Draw the line
					Vector2 lineStart = GraphPlotPoint(j, GraphData[i][j]);
					Vector2 lineEnd = GraphPlotPoint(j+1, GraphData[i][j+1]);
					canvas.DrawLine(lineStart, lineEnd);

					//Draw vertices
					if(shouldDrawVertices)
					{
						canvas.DrawLine(Vector2{lineEnd.X-1, lineEnd.Y - 2}, Vector2{lineEnd.X-1, lineEnd.Y + 1});
						canvas.DrawLine(Vector2{lineEnd.X, lineEnd.Y - 2}, Vector2{lineEnd.X, lineEnd.Y + 1});
						canvas.DrawLine(Vector2{lineEnd.X+1, lineEnd.Y - 2}, Vector2{lineEnd.X+1, lineEnd.Y + 1});
					}
				}
			}

			//Draw outline rectangle
			canvas.SetPosition(Vector2{graphLeft - 2, graphTop - 2});
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			canvas.DrawBox(Vector2{graphWidth + 4, graphHeight + 4});
		}



		/*	BAR
		*
		*	Data input works exactly the same as LINE up above
		*/
		if(graphStyle.compare("BAR") == 0)
		{
			float maxBarLength = graphWidth;
			int numSpaces = GraphData.size() + 1;

			//Draw background box
			canvas.SetPosition(Vector2{graphLeft, graphTop});
			canvas.SetColor(100, 100, 100, 100 * graphOpacity);
			canvas.FillBox(Vector2{graphWidth, graphHeight - (graphHeight / (int)(GraphData.size() + 1))});
		
			//Loop through the values and draw their bars
			for(int i=0; i<GraphData.size(); i++)
			{
				if(GraphData[i].size() > 0)
				{
					//Clamp bar to max value
					if(GraphData[i][0] > graphMax)
						GraphData[i][0] = graphMax;

					int barMidline = graphTop + (graphHeight / numSpaces) * (i+1) - (graphHeight / (numSpaces * 2));
					int barThickness = (graphHeight / (numSpaces * 2)) * graphBarThickness;

					//Draw bars
					canvas.SetColor(GraphColors[i].R, GraphColors[i].G, GraphColors[i].B, GraphColors[i].A * graphOpacity);
					canvas.SetPosition(Vector2{graphLeft, barMidline - barThickness});
					canvas.FillBox(Vector2{(int)((GraphData[i][0] / graphMax) * maxBarLength), barThickness * 2});

					//Draw labels
					canvas.SetPosition(Vector2{graphLeft + graphWidth + 10, barMidline - 4});
					canvas.DrawString(GraphLabels[i]);
				}
			}

			//Draw outline rectangle
			canvas.SetPosition(Vector2{graphLeft - 2, graphTop - 2});
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			canvas.DrawBox(Vector2{graphWidth + 4, graphHeight + 4 - (graphHeight / (int)(GraphData.size() + 1))});
		}



		/*	MULTIBAR
		*
		*	IMPORTANT:  For this graph type you must supply InputData with the correct numObjects variable
		*				That way this method will know how to split the data set its given
		*
		*	This graph type will take data in contiguous chunks instead of putting comparisons next to each other
		*	i.e. data = {car1X, car1Y, car1Z, car2X, car2Y, car2Z, car3X, car3Y, car3Z, car4X, car4Y, car4Z}
		*	instead of  {car1X, car2X, car3X, car4X, car1Y, car2Y, car3Y, car4Y, car1Z, car2Z, car3Z, car4Z}
		*	
		*	It is also important to supply the correct number of labels and colors for this type
		*	The number of colors should be the same as the value in the numBars variable
		*	Using the example above, there would be 3 labels (X, Y, Z) and 4 colors (car1, car2, car3, car4)
		*/
		if(graphStyle.compare("MULTIBAR") == 0)
		{
			float maxBarLength = graphWidth;
			int numDataPoints = GraphData.size() / numObjects;
			int numSpaces = numDataPoints + 1;

			//Draw background box
			canvas.SetPosition(Vector2{graphLeft, graphTop});
			canvas.SetColor(100, 100, 100, 100 * graphOpacity);
			canvas.FillBox(Vector2{graphWidth, graphHeight - (graphHeight / numSpaces)});

			//Loop through each data point
			for(int i=0; i<numDataPoints; i++)
			{
				if(GraphData[i].size() > 0)
				{
					int groupMidline = graphTop + (graphHeight / numSpaces) * (i+1) - (graphHeight / (numSpaces * 2));
					int groupThickness = (graphHeight / (numSpaces * 2)) * graphBarThickness;
					int individualBarThickness = (groupThickness * 2) / numObjects;

					//Loop through each object
					for(int j=0; j<numObjects; j++)
					{
						//Clamp bar to max value
						if(GraphData[i + (j*numDataPoints)][0] > graphMax)
							GraphData[i + (j*numDataPoints)][0] = graphMax;

						//Draw bar
						canvas.SetColor(GraphColors[j].R, GraphColors[j].G, GraphColors[j].B, GraphColors[j].A * graphOpacity);
						canvas.SetPosition(Vector2{graphLeft, groupMidline - groupThickness + (individualBarThickness * j)});
						canvas.FillBox(Vector2{(int)((GraphData[i + (j*numDataPoints)][0] / graphMax) * maxBarLength), individualBarThickness});
					}

					//Draw labels
					canvas.SetColor(255, 255, 255, 255 * graphOpacity);
					canvas.SetPosition(Vector2{graphLeft + graphWidth + 10, groupMidline - 4});
					canvas.DrawString(GraphLabels[i]);
				}
			}

			//Draw outline rectangle
			canvas.SetPosition(Vector2{graphLeft - 2, graphTop - 2});
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			canvas.DrawBox(Vector2{graphWidth + 4, graphHeight + 4 - (graphHeight / numSpaces)});
		}



		/*	SPLITBAR
		*
		*	The first half of the data set will be used for the bars on the left.
		*	The second half will be used for the bars on the right.
		*	i.e. data  = {car1X, car1Y, car1Z, car2X, car2Y, car2Z} will be split as shown below
		*		 Left  = {car1X, car1Y, car1Z}
		*		 Right = {car2X, car2Y, car2Z}
		*/
		if(graphStyle.compare("SPLITBAR") == 0)
		{
			int midline = graphLeft + (graphWidth / 2);
			float maxBarLength = midline - graphLeft;
			int graphDataSize = GraphData.size();
			int graphDataMiddleIndex = graphDataSize / 2;
			int numSpaces = (graphDataSize/2) + 1;

			//Draw background box
			canvas.SetPosition(Vector2{graphLeft, graphTop});
			canvas.SetColor(100, 100, 100, 100 * graphOpacity);
			canvas.FillBox(Vector2{graphWidth, graphHeight - (graphHeight / numSpaces)});
		
			//Loop through the values and draw their bars
			for(int i=0; i<graphDataMiddleIndex; i++)
			{
				if(GraphData[i].size() > 0)
				{
					//Clamp bar to max value
					if(GraphData[i][0] > graphMax)
						GraphData[i][0] = graphMax;
					if(GraphData[i+graphDataMiddleIndex][0] > graphMax)
						GraphData[i+graphDataMiddleIndex][0] = graphMax;

					int barMidline = graphTop + (graphHeight / numSpaces) * (i+1) - (graphHeight / (numSpaces * 2));
					int barThickness = (graphHeight / (numSpaces * 2)) * graphBarThickness;

					//Left bars
					canvas.SetColor(GraphColors[i].R, GraphColors[i].G, GraphColors[i].B, GraphColors[i].A * graphOpacity);
					canvas.SetPosition(Vector2{(int)(midline - (GraphData[i][0] / graphMax) * maxBarLength), barMidline - barThickness});
					canvas.FillBox(Vector2{(int)((GraphData[i][0] / graphMax) * maxBarLength), barThickness * 2});

					//Right bars
					int adjustedIndex = i+graphDataMiddleIndex;
					canvas.SetColor(GraphColors[adjustedIndex].R, GraphColors[adjustedIndex].G, GraphColors[adjustedIndex].B, GraphColors[adjustedIndex].A * graphOpacity);
					canvas.SetPosition(Vector2{midline, barMidline - barThickness});
					canvas.FillBox(Vector2{(int)((GraphData[adjustedIndex][0] / graphMax) * maxBarLength), barThickness * 2});

					//Draw labels
					canvas.SetPosition(Vector2{graphLeft + graphWidth + 10, barMidline - 4});
					canvas.DrawString(GraphLabels[i]);
				}
			}
		
			//Draw midline
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			canvas.DrawLine(Vector2{midline, graphTop}, Vector2{midline, graphTop + graphHeight - (graphHeight / numSpaces)});

			//Draw outline rectangle
			canvas.SetPosition(Vector2{graphLeft - 2, graphTop - 2});
			canvas.SetColor(255, 255, 255, 255 * graphOpacity);
			canvas.DrawBox(Vector2{graphWidth + 4, graphHeight + 4 - (graphHeight / numSpaces)});
		}
		

		/*	PIE
		*
		*	I *might* add this in the future, but it might get messy
		*	Conceptually, I would draw many triangles arrayed in a circle
		*	It would get messy when one of the triangles isn't completely one color
		*	I'd need to split the triangle into two or more along its edge and maintain the outer shape
		*/
	}
}

//UTILITY FUNCTIONS TO HELP WITH LINE GRAPHING
inline int BMGraphs::ClampToGraph(float value)
{
	if(value < graphTop)
		value = graphTop;
	if(value > (graphTop + graphHeight))
		value = graphTop + graphHeight;

	return value;
}
inline Vector2 BMGraphs::GraphPlotPoint(int i, float dataPoint)
{
	float horizontalRange = (float)graphWidth / (float)graphSteps;
	float Xvalue = horizontalRange * i + graphLeft;

	float Yvalue = GraphFindY(dataPoint);

	return Vector2{(int)Xvalue, (int)Yvalue};
}
inline float BMGraphs::GraphFindY(float dataPoint)
{
	float verticalRange = (float)graphHeight / ((float)graphMax - (float)graphMin);
	int graphFloor = graphTop + graphHeight;
	return ClampToGraph(graphFloor - ((dataPoint - graphMin) * verticalRange));
}
