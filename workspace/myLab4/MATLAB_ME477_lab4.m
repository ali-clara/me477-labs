clear all; close all; clc

load('Lab4.mat')

v_ss = 650; % found by inspection
t = linspace(0,length(vel)*0.005,length(vel));  % assuming each data point took 0.005 sec to add to buffer

vel_1_tau = (650*0.63);     % velocity at one time constant, 63% of steady state

% find the index of the time constant
limit = 0.5;
for i = 1:length(vel)
    if abs(vel(i)-vel_1_tau) < limit
        index = i;
        break
    end
end

tau=t(index);

plot(t, vel)
    hold on
    yline(v_ss)
    yline(vel_1_tau, '--')
    plot(tau, vel_1_tau, 'o')
    legend(["Data", "V_{ss} = 650 rpm", "0.63V_{ss}", "\tau = 1.12 sec"], location = "southeast")
    title("Velocity Response")
    xlabel("Time (sec)")
    ylabel("RPM")

