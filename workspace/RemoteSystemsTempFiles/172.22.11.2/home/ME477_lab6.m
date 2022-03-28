close all; clc

plot(input)
hold on
plot(output, '--')
title("Emulated Waveforms")
legend(["Input", "Output"])
xlabel("time (ms)")