#include "GraphingPluginExamples.h"
#include "bakkesmod\wrappers\includes.h"

BAKKESMOD_PLUGIN(GraphingPluginExamples, "Graphing Plugin Examples", "2.0", PLUGINTYPE_FREEPLAY)

void GraphingPluginExamples::onLoad()
{
    graphs = std::make_shared<BMGraphs>(cvarManager, gameWrapper);

    cvarManager->registerNotifier(NOTIFIER_START_TEST, [this](std::vector<std::string> params) {GraphTestBegin(params);}, "Display graph examples | Usage: GRAPHTEST <num 0-3> | (0 Line), (1 Bar), (2 Multibar), (3 Splitbar)", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_END_TEST, [this](std::vector<std::string> params) {GraphTestEnd();}, "End graph examples display", PERMISSION_ALL);

    GenerateSettingsFile();
}
void GraphingPluginExamples::onUnload() {}
bool GraphingPluginExamples::ShouldRun()
{
    if(!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining()) { return false; }

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    if(server.IsNull()) { return false; }

    CarWrapper car = gameWrapper->GetLocalCar();
    BallWrapper ball = server.GetBall();
    if(car.IsNull() || ball.IsNull()) { return false; }

    return true;
}

void GraphingPluginExamples::GraphTestBegin(std::vector<std::string> params)
{
    if(params.size() < 2) { return; }

    GraphTestEnd();

    GraphInitData InitData;

    int Command = stoi(params.at(1));
    switch(Command)
    {
        case 0:
        {
            InitData.Type = EGraphType::GRAPH_Line;
            InitData.Title = "Car and Ball Location";
            InitData.Labels = 
            {
                LabelInfo{"Car X Location",  LinearColor{255, 0,   0,   255}},
                LabelInfo{"Car Y Location",  LinearColor{0,   255, 0,   255}},
                LabelInfo{"Car Z Location",  LinearColor{0,   0,   255, 255}},
                LabelInfo{"Ball X Location", LinearColor{255, 180, 180, 255}},
                LabelInfo{"Ball Y Location", LinearColor{180, 255, 180, 255}},
                LabelInfo{"Ball Z Location", LinearColor{180, 180, 255, 255}}
            };

            break;
        }
        case 1:
        {
            InitData.Type = EGraphType::GRAPH_Bar;
            InitData.Title = "Distance Between Car and Ball";
            InitData.Labels = 
            {
                LabelInfo{"Distance to ball: X",    LinearColor{255, 0,   0,   255}},
                LabelInfo{"Distance to ball: Y",    LinearColor{0,   255, 0,   255}},
                LabelInfo{"Distance to ball: Z",    LinearColor{0,   0,   255, 255}},
                LabelInfo{"Total distance to ball", LinearColor{255, 255, 255, 255}}
            };

            break;
        }
        case 2:
        {
            InitData.Type = EGraphType::GRAPH_Multibar;
            InitData.Title = "Wheel Info";
            InitData.Labels = 
            {
                LabelInfo{"Wheel 0", LinearColor{255, 0,   0,   255}},
                LabelInfo{"Wheel 1", LinearColor{0,   255, 0,   255}},
                LabelInfo{"Wheel 2", LinearColor{0,   0,   255, 255}},
                LabelInfo{"Wheel 3", LinearColor{255, 255, 255, 255}}
            };
            InitData.MultibarValueLabels = 
            {
                "Steer Amount",
                "Suspension Distance",
                "Spin Speed"
            };

            break;
        }
        case 3:
        {
            InitData.Type = EGraphType::GRAPH_Splitbar;
            InitData.Title = "Car and Ball Velocity";
            InitData.Labels = 
            {
                LabelInfo{"X Speed",     LinearColor{255, 0,   0,   255}},
                LabelInfo{"Y Speed",     LinearColor{0,   255, 0,   255}},
                LabelInfo{"Z Speed",     LinearColor{0,   0,   255, 255}},
                LabelInfo{"Total Speed", LinearColor{255, 255, 255, 255}}
            };
            InitData.SplitbarAName = "Car";
            InitData.SplitbarBName = "Ball";

            break;
        }
        default:
        {
            cvarManager->log("Invalid choice. Choices are 0-3: (0 Line), (1 Bar), (2 Multibar), (3 Splitbar)");
            return;
        }
    }
        
    graphs->BeginRender(InitData);
    gameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", std::bind(&GraphingPluginExamples::InputData, this, InitData.Type));
}
void GraphingPluginExamples::GraphTestEnd()
{
    graphs->EndRender();
    gameWrapper->UnhookEvent("Function TAGame.PlayerInput_TA.PlayerInput");
}

void GraphingPluginExamples::InputData(EGraphType GraphType)
{
    if(!ShouldRun()) { return; }

    switch(GraphType)
    {
        case EGraphType::GRAPH_Line:     return InputLineGraphData();
        case EGraphType::GRAPH_Bar:      return InputBarGraphData();
        case EGraphType::GRAPH_Multibar: return InputMultibarGraphData();
        case EGraphType::GRAPH_Splitbar: return InputSplitBarGraphData();
        default: return;
    }
}

void GraphingPluginExamples::InputLineGraphData()
{
    //LINE GRAPH DATA
    //Show car and ball locations

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    CarWrapper car = gameWrapper->GetLocalCar();
    BallWrapper ball = server.GetBall();

    Vector carLocation = car.GetLocation();
    Vector ballLocation = ball.GetLocation();

    std::vector<LineGraphDataSingle> LineData;
    LineData.push_back({ "Car X Location",  carLocation.X  });
    LineData.push_back({ "Car Y Location",  carLocation.Y  });
    LineData.push_back({ "Car Z Location",  carLocation.Z  });
    LineData.push_back({ "Ball X Location", ballLocation.X });
    LineData.push_back({ "Ball Y Location", ballLocation.Y });
    LineData.push_back({ "Ball Z Location", ballLocation.Z });

    graphs->InputData(LineData);
}

void GraphingPluginExamples::InputBarGraphData()
{
    //BAR GRAPH DATA
    //Show distance between car and ball

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    CarWrapper car = gameWrapper->GetLocalCar();
    BallWrapper ball = server.GetBall();

    Vector carLocation = car.GetLocation();
    Vector ballLocation = ball.GetLocation();
            
    float xDist = abs(ballLocation.X - carLocation.X);
    float yDist = abs(ballLocation.Y - carLocation.Y);
    float zDist = abs(ballLocation.Z - carLocation.Z);
    float totalDistance = sqrtf(xDist*xDist + yDist*yDist + zDist*zDist);

    std::vector<BarGraphData> BarData;
    BarData.push_back({ "Distance to ball: X",    xDist });
    BarData.push_back({ "Distance to ball: Y",    yDist });
    BarData.push_back({ "Distance to ball: Z",    zDist });
    BarData.push_back({ "Total distance to ball", totalDistance });

    graphs->InputData(BarData);
}

void GraphingPluginExamples::InputMultibarGraphData()
{
    //MULTIBAR GRAPH DATA
    //Compare different values across all 4 wheels
    
    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    CarWrapper car = gameWrapper->GetLocalCar();
    BallWrapper ball = server.GetBall();
                
    VehicleSimWrapper carSim = car.GetVehicleSim();
    ArrayWrapper<WheelWrapper> wheels = carSim.GetWheels();

    std::string SteerValueLabel = "Steer Amount";
    std::vector<MultibarGraphDataSingle> SteerGroup;
    SteerGroup.push_back({ "Wheel 0", abs(wheels.Get(0).GetSteer2() * 57.296f) });
    SteerGroup.push_back({ "Wheel 1", abs(wheels.Get(1).GetSteer2() * 57.296f) });
    SteerGroup.push_back({ "Wheel 2", abs(wheels.Get(2).GetSteer2() * 57.296f) });
    SteerGroup.push_back({ "Wheel 3", abs(wheels.Get(3).GetSteer2() * 57.296f) });
    
    std::string SuspensionValueLabel = "Suspension Distance";
    std::vector<MultibarGraphDataSingle> SuspensionGroup;
    SuspensionGroup.push_back({ "Wheel 0", abs(wheels.Get(0).GetSuspensionDistance()) });
    SuspensionGroup.push_back({ "Wheel 1", abs(wheels.Get(1).GetSuspensionDistance()) });
    SuspensionGroup.push_back({ "Wheel 2", abs(wheels.Get(2).GetSuspensionDistance()) });
    SuspensionGroup.push_back({ "Wheel 3", abs(wheels.Get(3).GetSuspensionDistance()) });

    std::string SpinValueLabel = "Spin Speed";
    std::vector<MultibarGraphDataSingle> SpinGroup;
    SpinGroup.push_back({ "Wheel 0", abs(wheels.Get(0).GetSpinSpeed()) });
    SpinGroup.push_back({ "Wheel 1", abs(wheels.Get(1).GetSpinSpeed()) });
    SpinGroup.push_back({ "Wheel 2", abs(wheels.Get(2).GetSpinSpeed()) });
    SpinGroup.push_back({ "Wheel 3", abs(wheels.Get(3).GetSpinSpeed()) });

    std::vector<MultibarGraphData> MultibarData;
    MultibarData.push_back({SteerValueLabel, SteerGroup});
    MultibarData.push_back({SuspensionValueLabel, SuspensionGroup});
    MultibarData.push_back({SpinValueLabel, SpinGroup});

    graphs->InputData(MultibarData);
}

void GraphingPluginExamples::InputSplitBarGraphData()
{
    //SPLITBAR GRAPH DATA
    //Compare car velocity to ball velocity

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    CarWrapper car = gameWrapper->GetLocalCar();
    BallWrapper ball = server.GetBall();
                
    Vector carVelocity = car.GetVelocity();
    Vector ballVelocity = ball.GetVelocity();
    
    float carSpeed = carVelocity.magnitude();
    float ballSpeed = ballVelocity.magnitude();

    std::vector<SplitbarGraphData> SplitbarData;
    SplitbarData.push_back({ "X Speed", abs(carVelocity.X), abs(ballVelocity.X) });
    SplitbarData.push_back({ "Y Speed", abs(carVelocity.Y), abs(ballVelocity.Y) });
    SplitbarData.push_back({ "Z Speed", abs(carVelocity.Z), abs(ballVelocity.Z) });
    SplitbarData.push_back({ "Total Speed", carSpeed, ballSpeed });

    graphs->InputData(SplitbarData);
}
