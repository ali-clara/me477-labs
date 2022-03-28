clear all; close all; clc

load('Lab7.mat')

% transfer function parameters
kvi = 0.41;
kt = 0.11;
k = kvi*kt;
J = 3.8e-4;
wn = sqrt(ki*k/J);
damping = kp/2*sqrt(k/(J*ki));
tau = kp/ki;
kd = 1/(ki*k);
ku = J/k;

% transfer functions
s = tf('s');
denominator = (s^2/wn^2 + s*2*damping/wn + 1);  
T1 = (tau*s + 1) / denominator;     % vact/vref
T2 = s*kd / denominator;            % vact/td
T3 = s*ku*(tau*s+1) / denominator;  % u/vref

% plotting data
data_points = 250;
t = linspace(0, data_points*btiLength/1000, data_points);

subplot(1,2,1)
plot(t, vact)
    hold on
    [Y,T] = step(T1, t);
    plot(t, Y*400 - 200)
    yline(currentVref/2/pi*60, '--')
    title("Velocity Response")
    ylabel("Motor Speed (rpm)")
    xlabel("Time (s)")
    xlim([0, t(end)])
    legend(["Actual Velocity", "Analytical TF", "Final Reference Value"])

subplot(1,2,2)
plot(t, torque)
    hold on
    ylabel("Torque (N-M)")
    yyaxis right
    ylabel("Voltage (V)")
    [Y,T] = step(T3, t);
    plot(t, Y)
    title("Torque and Voltage")
    xlabel("Time (s)")
    xlim([0, t(end)])
    legend(["Experimental Torque", "Analytic TF, Control Voltage"])
