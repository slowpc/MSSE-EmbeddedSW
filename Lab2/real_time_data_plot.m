%% Real time data collection example
% This MATLAB script generates a real time plot of a PD controller over the
% serial port. It also has control to set PD constants and the relative
% target position.
% 
% Example COM ports:-
%
% On Linux:     serialPort = '/dev/ttyS0';
%
% On MacOS:     serialPort = '/dev/tty.KeySerial1';
%
% On Windows:   serialPort = 'COM1';
%
%
% To generate a report of this entire script, you may use the PUBLISH
% command at the MATLAB(R) command line as follows: 
% 
% publish('function_name.m');
% 
% Author: Kyle Rutlege
% Copyright 2013

function real_time_data_plot
    %% Serial port constants
    COM_PORT = 'COM7';
    BAUD_RATE = 256000;
    MEM_PREALLOCATE = 100000;
    GUI_REFRESH_MS_DEFAULT = 200;
    GUI_REFRESH_MS_MIN = 150;
    GUI_REFRESH_MS_MAX = 1000;
    MS_PER_S = 1000;
    
    %% Preallocate variables to graph
    curX  = nan(1, MEM_PREALLOCATE);
    curPe = nan(1, MEM_PREALLOCATE);
    curKp = nan(1, MEM_PREALLOCATE);
    curPr = nan(1, MEM_PREALLOCATE);
    curPm = nan(1, MEM_PREALLOCATE);
    curT  = nan(1, MEM_PREALLOCATE);
    curVm = nan(1, MEM_PREALLOCATE);
    curKd = nan(1, MEM_PREALLOCATE);

    lastKp = 0;
    lastPr = 0;
    lastPm = 0;
    lastT  = 0;
    lastVm = 0;
    lastKd = 0;
    lastPe = 0;

    %% Set up the figure window
    figureHandle = figure('NumberTitle','off',...
        'Position', [150, 75, 1500, 900], ...
        'Name','PD outputs',...
        'Visible','off');

    % Set axes
    axesPe = subplot(3,2,1, 'Parent', figureHandle);
    axesKp = subplot(3,2,2, 'Parent', figureHandle);
    axesP  = subplot(3,2,3, 'Parent', figureHandle);
    axesT  = subplot(3,2,4, 'Parent', figureHandle);
    axesV  = subplot(3,2,5, 'Parent', figureHandle);
    axesKd = subplot(3,2,6, 'Parent', figureHandle);
    
    %% Subplot strings
    
    ylabel(axesPe, 'Integer');
    title(axesPe, 'Position Error (Pe)');
    
    ylabel(axesKp, 'Integer');
    title(axesKp, 'K - Proportional (Kp)');
    
    ylabel(axesP, 'Integer');
    title(axesP, 'Position (Pr,Pm)');
    
    ylabel(axesT, 'Integer');
    title(axesT, 'Torque (T)');
    
    ylabel(axesV, 'Integer');
    title(axesV, 'Velocity (Vm)');
    
    ylabel(axesKd, 'Integer');
    title(axesKd, 'K - Derivative (Kd)');

    %% Other plot data

%     legend( plotPe, 'Position Error',   'Location', 'SouthWest');
%     legend( plotKp, 'K - Prop',         'Location', 'SouthWest');
%     legend( plotT,  'Torque',           'Location', 'SouthWest');
%     legend( plotVm, 'Velocity',         'Location', 'SouthWest');
%     legend( plotKd, 'K - Deriv',        'Location', 'SouthWest');
    
    %% GUI display constants
    buttonWidth = 100;
    buttonHeight = 50;
    textWidth = 60;
    textHeight = 25;
    valueWidth = 75;
    valueHeight = 25;
    
    %% Buttons
    cumButtonHeight = 0;
    closeButton = uicontrol( figureHandle, ...
        'Style', 'pushbutton', ...
        'String', 'Close', ...
        'Callback', @stopReading, ...
        'Position', [valueWidth cumButtonHeight buttonWidth buttonHeight]);
    cumButtonHeight = cumButtonHeight + buttonHeight;
    clearButton = uicontrol( figureHandle, ...
        'Style', 'pushbutton', ...
        'String', 'Clear', ...
        'Callback', @clearGraph, ...
        'Position', [valueWidth cumButtonHeight buttonWidth buttonHeight]);
    cumButtonHeight = cumButtonHeight + buttonHeight;
    
    function stopReading(hObj,event)
        % COM ports
        fclose(com);
        delete(com);
        clear com;

        % Figures
        close(figureHandle);

        % Timers
        stop(timerPlot);
        delete(timerPlot);
    end
    
    function clearGraph(hObj,event)
        %% Clear the graph array contents
        tick  = 1;
        curX  = nan(1, MEM_PREALLOCATE);
        curPe = nan(1, MEM_PREALLOCATE);
        curKp = nan(1, MEM_PREALLOCATE);
        curPr = nan(1, MEM_PREALLOCATE);
        curPm = nan(1, MEM_PREALLOCATE);
        curT  = nan(1, MEM_PREALLOCATE);
        curVm = nan(1, MEM_PREALLOCATE);
        curKd = nan(1, MEM_PREALLOCATE);
    end

    %% Texts
    cumTextHeight = cumButtonHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Kd', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Vm', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pr', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'T', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pm', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Kp', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pe', ...
        'Position', [valueWidth cumTextHeight textWidth textHeight]);
    cumTextHeight = cumTextHeight + textHeight;
    

    %% Values
    cumValueHeight = cumButtonHeight;
    KdValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    VmValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    PrValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    TValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    PmValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    KpValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    PeValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 cumValueHeight valueWidth valueHeight]);
    cumValueHeight = cumValueHeight + valueHeight;
    
    %% Plotting timer
    timerPlot = timer('TimerFcn', @updateGraph, 'Period', 1.0/8, 'ExecutionMode', 'fixedRate');
    
    function updateGraph(obj, event)
        % Update graphs
        set(axesPe,'NextPlot','replacechildren');
        plot(axesPe, curX, curPe, 'b');
        set(axesKp,'NextPlot','replacechildren');
        plot(axesKp, curX, curKp, 'b');
        set(axesP,'NextPlot','replacechildren');
        plot(axesP,  curX, curPr, 'r', curX, curPm, 'b');
        set(axesT,'NextPlot','replacechildren');
        plot(axesT,  curX, curT, 'b');
        set(axesT,'NextPlot','replacechildren');
        plot(axesV,  curX, curVm, 'b');
        set(axesT,'NextPlot','replacechildren');
        plot(axesKd, curX, curKd, 'b');

        % Update text fields
        set(PeValue, 'String', lastPe);
        set(KpValue, 'String', lastKp);
        set(PrValue, 'String', lastPr);
        set(PmValue, 'String', lastPm);
        set(TValue,  'String', lastT);
        set(VmValue, 'String', lastVm);
        set(KdValue, 'String', lastKd);

        drawnow;
    end

    %% GUI refresh

    cumXpos = 200;
    
    xWidthText = 75;
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'GUI Refresh', ...
        'Position', [cumXpos 0 xWidthText valueHeight]);
    cumXpos = cumXpos + xWidthText + 10;

    xWidthSlider = 200;
    GUIRefreshSlider = uicontrol( figureHandle, ...
        'Style', 'slider', ...
        'Min', 0,...
        'Max', GUI_REFRESH_MS_MAX,...
        'Value', GUI_REFRESH_MS_DEFAULT,...
        'SliderStep',[0.025 0.1],...
        'Callback', @updateGUIRefresh,...
        'Position', [cumXpos 0 xWidthSlider valueHeight]);
    cumXpos = cumXpos + xWidthSlider + 5;

    xWidthTextBox = 50;
    GUIRefreshText = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'String', GUI_REFRESH_MS_DEFAULT,...
        'Callback', @updateGUIRefresh,...
        'Position', [cumXpos 0 xWidthTextBox valueHeight]);
    cumXpos = cumXpos + xWidthTextBox;

    function updateGUIRefresh(obj, event, valRaw)
        style = get(obj, 'Style');
        isSlider = strcmpi(style, 'slider');
        
        % Check where to get the data from
        if ( isSlider )
            valRaw = get(obj, 'Value');
        else
            valRaw = str2double(get(obj,'String'));
            
            % Check if it is actually a number
            if ( ~isnumeric(valRaw) || isnan(valRaw) )
                valRaw = GUI_REFRESH_MS_DEFAULT;
            end
        end
 
        if (valRaw < GUI_REFRESH_MS_MIN)
            valRaw = GUI_REFRESH_MS_MIN;
        end
        if (valRaw > GUI_REFRESH_MS_MAX)
            valRaw = GUI_REFRESH_MS_MAX;
        end
        valRounded = round(valRaw);
        val = valRounded / MS_PER_S;

        stop(timerPlot);
        set(timerPlot, 'Period', val);
        start(timerPlot);

        set(GUIRefreshSlider, 'Value', valRounded);
        set(GUIRefreshText, 'String', num2str(valRounded));

        drawnow
    end

    %% Setup the serial port
    
    % Open ther COM port
    com = serial(COM_PORT);
    set(com, 'BaudRate', BAUD_RATE)
    fclose(instrfind);
    fopen(com);
    
    %% Initialization before collecting data
    tick = 1;
    set(figureHandle, 'Visible','on');
    start( timerPlot );
    
    %% Setup callbacks for receiving data
    set(com, 'BytesAvailableFcnMode', 'terminator');
    set(com, 'BytesAvailableFcn', @newCOMData);

    function newCOMData(obj, event)
        while ( obj.BytesAvailable )
            com_input = fscanf(obj,'%d,%d,%d,%d,%d,%d');

            % Parse the data
            curX(tick) = tick;
            lastKp = com_input(1);
            lastPr = com_input(1);
            lastPm = com_input(2);
            lastT  = com_input(1);
            lastVm = com_input(1);
            lastKd = com_input(1);
            lastPe = lastPm - lastPr;

            % Add to the graph arrays
            curPe(tick) = lastPe;
            curKp(tick) = lastKp;
            curPr(tick) = lastPr;
            curPm(tick) = lastPm;
            curT(tick)  = lastT;
            curVm(tick) = lastVm;
            curKd(tick) = lastKd;

            tick = tick + 1;
        end
    end
end