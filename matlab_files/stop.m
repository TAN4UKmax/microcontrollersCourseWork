% function stop()
% %STOP Stops motor
% port = 'COM9';
% command = ['VFDMotorControlWindows.exe --port ', port, ' --stop'];
% system(command);
global tmr

disp('stop program');
CMDcommand = 'TASKKILL /F /IM VFDMotorControlWindows.exe';
system(CMDcommand);
port = 'COM9';

if (strcmp(tmr.Running, 'on'))
stop(tmr);
% delete(tmr);
end

CMDcommand = ['VFDMotorControlWindows.exe --port ', port, ' --stop'];
system(CMDcommand);
CMDcommand = 'TASKKILL /F /IM cmd.exe';
system(CMDcommand);



% end
