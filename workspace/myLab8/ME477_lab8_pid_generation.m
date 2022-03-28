clear all; close all; clc

kvi = 0.41;     % A/volt
kt = 0.11;      % N-m/A
J = 3.8e-4;     % N-m-s^2/rad
fb = 8;         % hz

% 1. plant model
plant = tf([kvi*kt], [J, 0, 0]);

% 2. PIDF controller using pidtune()
options = pidtuneOptions('DesignFocus', 'reference-tracking');
wc = 2*pi*fb;
c = pidtune(plant, 'pidf', wc, options);    % controller (parallel form)

% 3. check performance of controller
cl_sys = feedback(c*plant, 1);
figure
hold on
    step(plant)
    step(cl_sys)
    legend(["Uncompensated", "Compensated"])
    xlim([0, 0.6])
    ylim([0, 1.2])

% 4. convert controller to discreet form
T = 0.005;   % sample time
cdp = c2d(c,T, 'tustin');   % discreet parallel form
cd = tf(cdp);   % discreet TF form
[b, a] = tfdata(cd, 'v');   % numerator and denominator coefficients
sos = tf2sos(b, a);     % second order sections

% 5. save the controller to a header file (.h)
HeaderFileName = 'C:\Users\alicl\workspace\myLab8\myPIDF.h';
fid = fopen(HeaderFileName, 'w');
comment = 'Biquad Second Order Controller';
sos2header(fid, sos, 'PIDF', T, comment);
fclose(fid);

