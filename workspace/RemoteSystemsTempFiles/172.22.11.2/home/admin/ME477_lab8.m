clear all; close all; clc

load('Lab8.mat')

% plant
kvi = 0.41;     % A/volt
kt = 0.11;      % N-m/A
J = 3.8e-4;     % N-m-s^2/rad
fb = 8;         % hz
T = 0.005;   % sample time

% forgot to grab PIDF from c code
PIDF_num = [4.620281e+01, -9.135472e+01, 4.515400e+01];
PIDF_den = [1.000000e+00, -1.492420e+00, 4.924205e-01];

% discreet plant
plant = tf([kvi*kt], [J, 0, 0]);
pdp = c2d(plant, T, 'tustin');   % discreet parallel form
pd = tf(pdp);   % discreet TF form

% discreet controller
c = tf(PIDF_num, PIDF_den, T, 'Variable', 'z');

% closed loop models
pos_sys = feedback(c*pd, 1);    % p_act(z) / p_ref(z)
torque_sys = feedback(c, pd);   % T(z) / p_ref(z)

% plots plots plots
t = [0:T:19.995];
subplot(2,2,[1,2])
hold on
    ptheory = lsim(pos_sys, pref, t);
    plot(t, ptheory)
    plot(t, pref, 'k--')
    plot(t, pact, 'r')
    legend(["Linear Simulation", "Reference Position", "Actual Position"])
    xlabel("Time (s)")
    ylabel("Position (rev)")
    title("Position Results")
    grid on

subplot(2,2,3)
hold on
    plot(t, pref-ptheory)
    plot(t, pref-pact, 'r')
    legend(["Theoretical", "Experimental"])
    xlabel("Time (s)")
    ylabel("Error (rev)")
    title("Error Results")
    grid on


subplot(2,2,4)
hold on
    ttheory = lsim(torque_sys, pref, t);
    plot(t, ttheory)
    plot(t, torque, 'r.', 'MarkerSize',1)
    legend(["Theoretical", "Experimental"])
    xlabel("Time (s)")
    ylabel("Torque (N-m)")
    title("Torque Results")
    grid on






