clc; close all; clear;

ACC_COUNT = 4800000;

fc = 1.36;
fs = 48000;

t = 0:1/fs:20-1/fs;
L = length(t);


sin_wavetable = sin(2*pi*1*t);

sample = 1;
for i = 1:1:length(t)
    x(i) = sin_wavetable(round(sample / 100) + 1);
    
        sample = round(sample + fc * 100);

    if (sample > ACC_COUNT)
        sample = mod(sample, ACC_COUNT);
    end
end

%soundsc(x,fs)

%figure (1)
%plot(tx, x)
x = [x, x];
L = length(x);
f = fs*(0:(L/2))/L;
X = fft(x);

P2 = abs(X) / L;
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

figure(2)
plot(f,P1) 
title('Single-Sided Amplitude Spectrum of X(t)')
xlabel('f (Hz)')
ylabel('|P1(f)|')