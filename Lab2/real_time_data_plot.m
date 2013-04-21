%% Real time data collection example
% This MATLAB script generates a real time plot of voltage values collected
% from an Agilent(TM) 34401A Digital Multimeter over the serial
% port(RS232). The data is collected and plotted until the predefined stop
% time is reached. This example also demonstrates automating a query based
% interaction with an instrument while monitoring the results live.
%
% This script can be modified to be used on any platform by changing the
% serialPort variable.
% 
% Example:-
%
% On Linux:     serialPort = '/dev/ttyS0';
%
% On MacOS:     serialPort = '/dev/tty.KeySerial1';
%
% On Windows:   serialPort = 'COM1';
%
%
% The script may also be updated to use any instrument/device to collect
% real time data. You may need to update the SCPI commands based on
% information in the instrument's programming manual.
%
% To generate a report of this entire script, you may use the PUBLISH
% command at the MATLAB(R) command line as follows: 
% 
% publish(real_time_data_plot);
% 
% Author: Ankit Desai
% Copyright 2009 - The MathWorks, Inc.

function real_time_data_plot

    %% Set up the figure window
    x = nan(1, 10000);
    Pe = nan(1, 10000);
    Kp = nan(1, 10000);
    Pr = nan(1, 10000);
    Pm = nan(1, 10000);
    T = nan(1, 10000);
    Vm = nan(1, 10000);
    Kd = nan(1, 10000);
    
    figureHandle = figure('NumberTitle','off',...
        'Position', [200, 100, 1500, 900], ...
        'Name','PD outputs',...
        'Visible','off');

    % Set axes
    axesPe = subplot(3,2,1, 'Parent', figureHandle, 'Visible', 'off');
    axesKp = subplot(3,2,2, 'Parent', figureHandle, 'Visible', 'off');
    axesP  = subplot(3,2,3, 'Parent', figureHandle, 'Visible', 'off');
    axesT  = subplot(3,2,4, 'Parent', figureHandle, 'Visible', 'off');
    axesV  = subplot(3,2,5, 'Parent', figureHandle, 'Visible', 'off');
    axesKd = subplot(3,2,6, 'Parent', figureHandle, 'Visible', 'off');

    hold on;

    plotPr = plot(axesP, 0,0, 'color', 'blue');
    plotPm = plot(axesP, 0,0, 'color', 'red');
%     legend( [plotPr,plotPm], 'Position Ref', 'Position Target',  'Location', 'SouthWest');

    hold off;

    plotPe = plot(axesPe,0,0, 'color', 'blue');
    plotKp = plot(axesKp,0,0, 'color', 'blue');
    plotT  = plot(axesT, 0,0, 'color', 'blue');
    plotVm = plot(axesV, 0,0, 'color', 'blue');
    plotKd = plot(axesKd,0,0, 'color', 'blue');

    legend( plotPe, 'Position Error',   'Location', 'SouthWest');
    legend( plotKp, 'K - Prop',         'Location', 'SouthWest');
    legend( plotT,  'Torque',           'Location', 'SouthWest');
    legend( plotVm, 'Velocity',         'Location', 'SouthWest');
    legend( plotKd, 'K - Deriv',        'Location', 'SouthWest');

    % Create xlabel
    xlabel('Time');

    % Create ylabel
    ylabel('Integer');

    % Create title
    title('Data From Organutan');

    %% Create the serial object
    com = serial('COM7');
    set(com, 'BaudRate', 256000)
    
    %% Button to stop the program
    
    % Buttons
    closeButton = uicontrol( figureHandle, ...
        'Style', 'pushbutton', ...
        'String', 'Close', ...
        'Callback', {@stopReading,com}, ...
        'Position', [75 0 75 25]);
    clearButton = uicontrol( figureHandle, ...
        'Style', 'pushbutton', ...
        'String', 'Clear', ...
        'Callback', {@clearGraph,plotPe,plotKp,plotPr,plotPm,plotT,plotVm,plotKd}, ...
        'Position', [75 25 75 25]);
    
    % Texts
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pe', ...
        'Position', [75 50 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Kp', ...
        'Position', [75 75 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pr', ...
        'Position', [75 100 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Pm', ...
        'Position', [75 125 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'T', ...
        'Position', [75 150 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Vm', ...
        'Position', [75 175 75 25]);
    uicontrol( figureHandle, ...
        'Style', 'text', ...
        'String', 'Kd', ...
        'Position', [75 200 75 25]);

    % Values
    PeValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 50 75 25]);
    KpValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 75 75 25]);
    PrValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 100 75 25]);
    PmValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 125 75 25]);
    TValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 150 75 25]);
    VmValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 175 75 25]);
    KdValue = uicontrol( figureHandle, ...
        'Style', 'edit', ...
        'Enable', 'off', ...
        'Position', [0 200 75 25]);

    %%
    
    fclose(instrfind);
    fopen(com);
    
    %% Collect data
    enableLoop = 1;
    tick = 1;

    while ( enableLoop == 1 )
            com_input = fscanf(com,'%d,%d,%d');

            x1 = get(plotPe, 'XData');
            Kp1 = get(plotKp, 'YData');
%             Pr1 = get(plotPr, 'YData');
%             Pm1 = get(plotPm, 'YData');
            T1 = get(plotT, 'YData');
            Vm1 = get(plotVm, 'YData');
            Kd1 = get(plotKd, 'YData');

            if isempty(x1) || isempty(Kp1) || isempty(T1) || isempty(Vm1) || isempty(Kd1) % isempty(Pr1) || isempty(Pm1)
                tick = 1;

                x = zeros(1, 10000);
                Pe = zeros(1, 10000);
                Kp = zeros(1, 10000);
                Pr = zeros(1, 10000);
                Pm = zeros(1, 10000);
                T = zeros(1, 10000);
                Vm = zeros(1, 10000);
                Kd = zeros(1, 10000);
            end
    
            x(tick) = tick;

            Kp(tick) = com_input(1);
            Pr(tick) = com_input(3);
            Pm(tick) = com_input(1);
            T(tick) = com_input(2);
            Vm(tick) = com_input(3);
            Kd(tick) = com_input(3);

            Pe(tick) = Pm(tick) - Pr(tick);
            
            if ( mod(tick,4) == 0 )
                set(plotPe,'YData',Pe,'XData',x);
                set(plotKp,'YData',Kp,'XData',x);
    %             set(plotPr,'YData',Pr,'XData',x);
    %             set(plotPm,'YData',Pm,'XData',x);
                set(plotT,'YData',T,'XData',x);
                set(plotVm,'YData',Vm,'XData',x);
                set(plotKd,'YData',Kd,'XData',x);
                set(figureHandle,'Visible','on');

                set(PeValue, 'String', com_input(1)-com_input(2));
                set(KpValue, 'String', com_input(1));
    %             set(PrValue, 'String', com_input(3));
    %             set(PmValue, 'String', com_input(1));
                set(TValue, 'String', com_input(2));
                set(VmValue, 'String', com_input(3));
                set(KdValue, 'String', com_input(3));
            end

            clear x1;
            clear Kp1;
            clear Pr1;
            clear Pm1;
            clear TV1;
            clear Vm1;
            clear Kd1;
            
            tick = tick + 1;
    end
    
    %% Clean up the serial object
    fclose(com);
    delete(com);
    clear com;
    
    %% Clean up the figure
    close(figureHandle);
end

function stopReading(hObj,event,serial)
    %% Stop reading
    fclose(serial);
end

function clearGraph(hObj,event,p1,p2,p3,p4,p5,p6,p7)
    %% Clear contents of data used to graph
    set(p1,'XData',[],'YData',[]);
    set(p2,'XData',[],'YData',[]);
%     set(p3,'XData',[],'YData',[]);
%     set(p4,'XData',[],'YData',[]);
    set(p5,'XData',[],'YData',[]);
    set(p6,'XData',[],'YData',[]);
    set(p7,'XData',[],'YData',[]);
end