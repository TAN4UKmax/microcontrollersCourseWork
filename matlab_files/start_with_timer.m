% function start_with_timer()
clc, clear, close all
port = 'COM9';
global tmrTime lastTimeParam paramFileName maxTime tmr
global freq_line current_line DC_voltage_line voltage_line PF_line torque_line speed_line power_line temp_line % lines
lastTimeParam = 0;
paramFileName = 'paramTable.txt';

% Create time and frequency arrays

% signalEditor('DataSource','untitled.mat');

% sig = load('untitled.mat');
% time = sig.Signal.Values.Time;
% frequency = sig.Signal.Values.Data;

step = 0.5;
t_n = 0;
t_k = 10;
f_n = 0;
f_k = 50;
t_e = (t_n + step):step:t_k;
f_e = f_n + (f_k - f_n) .* (1 - exp(-(5 .* (t_e - t_n)) ./ (t_k-t_n)));

time = [0, t_e, 12, 14, 16, 18, 19, 21, 24];
frequency = [0, f_e, 10, 10, -20, -20, -40, -40, 0];

% time = [0, 5, 10, 15, 20, 25, 30, 35, 40];
% frequency = [0, 50, -50, 50, -50, 20, 20, 30, 30];

maxTime = max(time);

%% Create figures
fg_freq = figure('Name','Motor frequency');
fg_freq.WindowState = 'maximized';
% fg_freq.MenuBar = 'none';
plot(time, frequency, '--k', 'LineWidth', 1);
xlabel('t, с');
ylabel('f, Гц');
grid on;
grid minor;
xlim([0 (maxTime * 1.01)]);
ylim([(min(frequency) * 1.1), (max(frequency) * 1.1)]);
freq_line = animatedline;
freq_line.LineWidth = 1;
freq_line.Color = 'red';
addpoints(freq_line, 0, 0);
% title('Частота двигателя');
legend('Задание', 'Выходная');

% create current figure
fg_cur = figure('Name','Motor current');
fg_cur.WindowState = 'maximized';
% fg_cur.MenuBar = 'none';
xlabel('t, с');
ylabel('I, А');
grid on;
grid minor;
% title('Ток двигателя');
xlim([0 (maxTime * 1.01)]);
current_line = animatedline;
current_line.LineWidth = 1;
current_line.Color = 'black';
addpoints(current_line, 0, 0);

% % create DC voltage figure
% fg_DC_vol = figure('Name','DC voltage');
% fg_DC_vol.WindowState = 'maximized';
% fg_DC_vol.MenuBar = 'none';
% xlabel('t, с');
% ylabel('U, В');
% grid on;
% grid minor;
% title('Входное напряжение');
% xlim([0 (maxTime * 1.01)]);
% DC_voltage_line = animatedline;
% DC_voltage_line.LineWidth = 1;
% DC_voltage_line.Color = 'black';
% addpoints(DC_voltage_line, 0, 0);

% create out voltage figure
fg_vol = figure('Name','Motor voltage');
fg_vol.WindowState = 'maximized';
% fg_vol.MenuBar = 'none';
xlabel('t, с');
ylabel('U, В');
grid on;
grid minor;
% title('Напряжение двигателя');
xlim([0 (maxTime * 1.01)]);
voltage_line = animatedline;
voltage_line.LineWidth = 1;
voltage_line.Color = 'black';
addpoints(voltage_line, 0, 0);

% % create Power factor figure
% fg_PF = figure('Name','Power factor');
% fg_PF.WindowState = 'maximized';
% fg_PF.MenuBar = 'none';
% xlabel('t, с');
% grid on;
% grid minor;
% title('Коэффициент мощности');
% xlim([0 (maxTime * 1.01)]);
% PF_line = animatedline;
% PF_line.LineWidth = 1;
% PF_line.Color = 'black';
% addpoints(PF_line, 0, 0);


% create motor speed figure
fg_PF = figure('Name','Motor speed');
fg_PF.WindowState = 'maximized';
% fg_PF.MenuBar = 'none';
xlabel('t, с');
ylabel('n, об/мин');
grid on;
grid minor;
% title('Скорость двигателя');
xlim([0 (maxTime * 1.01)]);
speed_line = animatedline;
speed_line.LineWidth = 1;
speed_line.Color = 'black';
addpoints(speed_line, 0, 0);

% % create torque figure
% fg_torque = figure('Name','Motor torque');
% fg_torque.WindowState = 'maximized';
% fg_torque.MenuBar = 'none';
% xlabel('t, с');
% ylabel('Н*м');
% grid on;
% grid minor;
% title('Момент двигателя');
% xlim([0 (maxTime * 1.01)]);
% torque_line = animatedline;
% torque_line.LineWidth = 1;
% torque_line.Color = 'black';
% addpoints(torque_line, 0, 0);

% % create power figure
% fg_vol = figure('Name','Motor power');
% fg_vol.WindowState = 'maximized';
% fg_vol.MenuBar = 'none';
% xlabel('t, с');
% ylabel('P, кВт');
% grid on;
% grid minor;
% title('Выходная мощность');
% xlim([0 (maxTime * 1.01)]);
% power_line = animatedline;
% power_line.LineWidth = 1;
% power_line.Color = 'black';
% addpoints(power_line, 0, 0);

% % create temperature figure
% fg_temp = figure('Name','VFD temperature');
% fg_temp.WindowState = 'maximized';
% fg_temp.MenuBar = 'none';
% xlabel('t, с');
% ylabel('T, °C');
% grid on;
% grid minor;
% title('Температура радиатора');
% xlim([0 (maxTime * 1.01)]);
% temp_line = animatedline;
% temp_line.LineWidth = 1;
% temp_line.Color = 'black';
% addpoints(temp_line, 0, 0);

%% Create file
outFileName = 'coords.txt';
[fileID, err_msg] = fopen(outFileName, 'wt');
header_string = 'Time\tFrequency\n';
% Write file
fprintf(fileID, header_string);
for i = 1:numel(time)
    fprintf(fileID, '%.4g\t%.4g\n', time(i), frequency(i));
end
fclose(fileID);

pause(1); % wait a bit until figure opens
%% Launch program
programName = 'VFDMotorControlWindows.exe';
% CMDcommand = [programName, ' --port ', port, ...
%     ' --file ', outFileName, ...
%     ' --get OutFrequency &'];
% CMDcommand = [programName, ' --port ', port, ...
%     ' --file ', outFileName, ...
%     ' --get OutFrequency --get OutCurrent --get DCVoltage --get OutVoltage --get PowerFactor --get OutTorque --get MotorSpeed --get OutPower --get VFDTemperature &'];

CMDcommand = [programName, ' --port ', port, ...
    ' --file ', outFileName, ...
    ' --get OutFrequency --get OutCurrent --get OutVoltage --get MotorSpeed &'];

system(CMDcommand);

%% Prepare all data, create and launch timer

tmr = timer( ...
    'BusyMode', 'queue', ...
    'ExecutionMode', 'fixedRate', ...
    'Period', 0.040, ...
    'TimerFcn', @tmrCallback);

tmrTime = tic;
figure(fg_freq); % show figure with frequency here
% shg;
start(tmr);
% end

function tmrCallback(tmrObj, ~)
%TMRCALLBACK is called by timer every second to print new data
global tmrTime maxTime lastTimeParam paramFileName
global freq_line current_line DC_voltage_line voltage_line PF_line torque_line speed_line power_line temp_line % lines

% Проверять есть ли файл
if isfile(paramFileName)
    % ждать немного когда появился файл
    pause(0.001);
    params = readtable(paramFileName);
    delete(paramFileName);
    tmrTime = tic;
    lastTimeParam =  params.Time(1);
    %     disp(lastTimeParam);
    addpoints(freq_line, params.Time(1), params.OutFrequency(1));
    addpoints(current_line, params.Time(1), params.OutCurrent(1));
%     addpoints(DC_voltage_line, params.Time(1), params.DCVoltage(1));
    addpoints(voltage_line, params.Time(1), params.OutVoltage(1));
%     addpoints(PF_line, params.Time(1), params.PowerFactor(1));
%     addpoints(torque_line, params.Time(1), params.OutTorque(1));
    addpoints(speed_line, params.Time(1), params.MotorSpeed(1));
%     addpoints(power_line, params.Time(1), params.OutPower(1));
%     addpoints(temp_line, params.Time(1), params.VFDTemperature(1));
    return
end

%     копировать в другой файл
% if copyfile(paramFileName, 'paramTableTmp.txt', 'f')
%     delete(paramFileName);
%     tmrTime = tic;
%     params = readtable('paramTableTmp.txt');
%     lastTimeParam =  params.Time(1);
%     %             disp(lastTimeParam);
%     addpoints(freq_line, params.Time(1), params.OutFrequency(1));
%     addpoints(current_line, params.Time(1), params.OutCurrent(1));
%     addpoints(DC_voltage_line, params.Time(1), params.DCVoltage(1));
%     addpoints(voltage_line, params.Time(1), params.OutVoltage(1));
%     addpoints(PF_line, params.Time(1), params.PowerFactor(1));
%     addpoints(torque_line, params.Time(1), params.OutTorque(1));
%     addpoints(speed_line, params.Time(1), params.MotorSpeed(1));
%     addpoints(power_line, params.Time(1), params.OutPower(1));
%     addpoints(temp_line, params.Time(1), params.VFDTemperature(1));
%     return
% end

curTime = toc(tmrTime); % теперь это время относительно последнего принта
% end of diagram event
if lastTimeParam + curTime > maxTime
    disp('end of diagram');
    CMDcommand = 'TASKKILL /F /IM cmd.exe';
    system(CMDcommand);
    
    stop(tmrObj);
    %     delete(tmrObj);
    
    % connection lost event
elseif curTime > 1.5 % прошло больше 1.5 секунды от последнего времени чтения
    msgbox('No connection!', 'Error','error')
    disp('No connection');
    CMDcommand = 'TASKKILL /F /IM cmd.exe';
    system(CMDcommand);
    
    stop(tmrObj);
    %     delete(tmrObj);
end

end
