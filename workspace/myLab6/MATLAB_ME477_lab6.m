clear all; close all; clc

load("Lab6.mat")

plot(input)
hold on
plot(output, '--')
title("Emulated Waveforms")
legend(["Input", "Output"])
xlabel("time (ms)")