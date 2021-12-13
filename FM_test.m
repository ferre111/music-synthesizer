clc; close all; clear;

fc = 261;
fm = 261 * 2;
B = 2;
fs = 48000;

t = 0:1/fs:20 - 1/fs;
L = length(t);
f = fs*(0:(L/2))/L;

x = sin(2 * pi * fc * t + B * sin(2 * pi * fm * t));
X = fft(x);

%soundsc(x,fs)

P2 = abs(X) / L;
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

figure(2)
plot(f,P1) 
title('Single-Sided Amplitude Spectrum of X(t)')
xlabel('f (Hz)')
ylabel('|P1(f)|')