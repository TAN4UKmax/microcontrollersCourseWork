% function start_with_timer()
clc, clear, close all

global tmrTime lastTimeParam paramFileName maxTime tmr
global freq_line current_line voltage_line temp_line % lines
lastTimeParam = 0;
paramFileName = 'paramTable.txt';

% Create time and frequency arrays
time = [0, 5, 10, 15, 20, 25, 30, 35, 40];
frequency = [0, 50, -50, 50, -50, 20, 20, 30, 30];
maxTime = max(time);
fg_freq = figure('Name','Motor frequency');
% fg = figure('Name','Motor frequency', 'Visible', 'off');
fg_freq.WindowState = 'maximized';
fg_freq.MenuBar = 'none';
fg_freq.ToolBar = 'none';
plot(time, frequency, 'b');
grid on;
grid minor;

xlim([0 (maxTime * 1.01)]);
ylim([(min(frequency) * 1.1), (max(frequency) * 1.1)]);

freq_line = animatedline;
freq_line.LineWidth = 1;
freq_line.Color = 'red';
addpoints(freq_line, 0, 0);
legend('Command', 'Out');

% create current figure
fg_cur = figure('Name','Motor current');
fg_cur.WindowState = 'maximized';
fg_cur.MenuBar = 'none';
fg_cur.ToolBar = 'none';
grid on;
grid minor;
xlim([0 (maxTime * 1.01)]);
current_line = animatedline;
current_line.LineWidth = 1;
current_line.Color = 'black';
addpoints(current_line, 0, 0);

% create voltage figure
fg_vol = figure('Name','Motor voltage');
fg_vol.WindowState = 'maximized';
fg_vol.MenuBar = 'none';
fg_vol.ToolBar = 'none';
grid on;
grid minor;
xlim([0 (maxTime * 1.01)]);
voltage_line = animatedline;
voltage_line.LineWidth = 1;
voltage_line.Color = 'black';
addpoints(voltage_line, 0, 0);

% create temperature figure
fg_temp = figure('Name','VFD temperature');
fg_temp.WindowState = 'maximized';
fg_temp.MenuBar = 'none';
fg_temp.ToolBar = 'none';
grid on;
grid minor;
xlim([0 (maxTime * 1.01)]);
temp_line = animatedline;
temp_line.LineWidth = 1;
temp_line.Color = 'black';
addpoints(temp_line, 0, 0);

%% Create file
outFileName = 'coords.txt';
[fileID, err_msg] = fopen(outFileName, 'wt');
header_string = 'Time\tFrequency\n';
% Write file
fprintf(fileID, header_string);
for i = 1:numel(time)
    fprintf(fileID, '%g\t%g\n', time(i), frequency(i));
end
fclose(fileID);

pause(1); % wait a bit until figure opens
%% Launch program
programName = 'VFDMotorControlWindows.exe';
port = 'COM9';
CMDcommand = [programName, ' --port ', port, ...
    ' --file ', outFileName, ...
    ' --get OutFrequency --get OutCurrent --get OutVoltage --get VFDTemperature &'];
%     ' --get FrequencyCommand --get OutFrequency --get OutCurrent --get OutVoltage &'];
system(CMDcommand);

%% Prepare all data, create and launch timer


tmr = timer( ...
    'BusyMode', 'queue', ...
    'ExecutionMode', 'fixedRate', ...
    'Period', 0.050, ...
    'TimerFcn', @tmrCallback);

tmrTime = tic;
figure(fg_freq); % show figure with frequency here
% shg;
start(tmr);
% end

function tmrCallback(tmrObj, ~)
%TMRCALLBACK is called by timer every second to print new data
global tmrTime maxTime lastTimeParam paramFileName
global freq_line current_line voltage_line temp_line % lines

% Проверять есть ли файл
% if isfile(paramFileName)
%     % ждать немного когда появился файл
%     %         pause(0.001);
%     params = readtable(paramFileName);
%     delete(paramFileName);
%     tmrTime = tic;
%     lastTimeParam =  params.Time(1);
%     %     disp(lastTimeParam);
%     addpoints(freq_line, params.Time(1), params.OutFrequency(1));
%     addpoints(current_line, params.Time(1), params.OutCurrent(1));
%     addpoints(voltage_line, params.Time(1), params.OutVoltage(1));
%     addpoints(temp_line, params.Time(1), params.VFDTemperature(1));
%     return
% end

%     копировать в другой файл
if copyfile(paramFileName, 'paramTableTmp.txt', 'f')
    delete(paramFileName);
    tmrTime = tic;
    params = readtable('paramTableTmp.txt');
    lastTimeParam =  params.Time(1);
    %             disp(lastTimeParam);
    addpoints(freq_line, params.Time(1), params.OutFrequency(1));
    addpoints(current_line, params.Time(1), params.OutCurrent(1));
    addpoints(voltage_line, params.Time(1), params.OutVoltage(1));
    addpoints(temp_line, params.Time(1), params.VFDTemperature(1));
    return
end

curTime = toc(tmrTime); % теперь это время относительно последнего принта
% end of diagram event
if lastTimeParam + curTime > maxTime
    disp('end of diagram');
    CMDcommand = 'TASKKILL /F /IM cmd.exe';
    system(CMDcommand);
    
    stop(tmrObj);
    %     delete(tmrObj);
    
    % connection lost event
elseif curTime > 2.0 % прошло больше 1 секунды от последнего времени чтения
    msgbox('Connection lost', 'Error','error')
    disp('connection lost');
    CMDcommand = 'TASKKILL /F /IM cmd.exe';
    system(CMDcommand);
    
    stop(tmrObj);
    %     delete(tmrObj);
end

end
