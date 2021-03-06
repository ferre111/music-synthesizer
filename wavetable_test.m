clc; close all; clear;

fc = 261;
fm = 261 * 2;
B = 2;
fs = 48000;

t = 0:1/fs:20-1/fs;
L = length(t);
f = fs*(0:(L/2))/L;

sin_wavetable = sin(2*pi*1*t);

sample_c = 1;
sample_m = 1;
for i = 1:1:length(t)
    sample_c = round(sample_c + B * fm * sin_wavetable(sample_m) + fc);

    if (sample_c > 48000)
        sample_c = mod(sample_c, 48000);
    end

    if (sample_c < 1)
        sample_c = 48000 + sample_c;
    end    

    x(i) = sin_wavetable(sample_c);

    sample_m = round(sample_m + fm);
    if (sample_m > 48000)
        sample_m = mod(sample_m, 48000);
    end
end

%soundsc(x,fs)

%figure (1)
%plot(tx, x)

X = fft(x);

P2 = abs(X) / L;
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

figure(2)
plot(f,P1) 
title('Single-Sided Amplitude Spectrum of X(t)')
xlabel('f (Hz)')
ylabel('|P1(f)|')