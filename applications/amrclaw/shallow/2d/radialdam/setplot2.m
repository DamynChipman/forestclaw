OutputFlag = 'ForestClaw';         % default value.
OutputDir = './';            % Default (reassign them here anyway)

ForestClaw = 1;     % Plot using ForestClaw preferences.

mq = 1;
UserVariable = 0;
UserVariableFile = ' ';
MaxFrames = 1000;
MaxLevels = 30;
Manifold = 0;
PlotData =  ones(1,MaxLevels);
PlotGrid =  zeros(1,MaxLevels);
PlotGridEdges = zeros(1,MaxLevels); 

% PlotType = 1;  % Pseudo-color plot
PlotType = 4;  % Scatter plot (compare with rad1d)

MappedGrid = 0;

% for contour plots (PlotType==2):
ContourValues = [];

% for scatter plot (PlotType==4):
  x0 = 0;
  y0 = 0;
  ScatterStyle = setplotstyle('ro','m*','cs','gp','ro','k*');

PlotParallelPartitions=0;

