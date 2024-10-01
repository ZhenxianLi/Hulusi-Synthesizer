
%   function find base Freq and Harmric Scale
%   use fft analysis the audio 

function [baseFreq,HarmScale]=harmAnalysis(y,Fs)
%fft and plot
Y = fft(y);         % Fast Fourier Transform
P2 = abs(Y/length(y));
P1 = P2(1:length(y)/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(length(y)/2))/length(y);
HarmScale=zeros(20,1);

%play the sound
%soundsc(y,Fs);

% plot
% figure;
% plot(f,P1);
% title('Single-Sided Amplitude Spectrum of y(t)');
% xlabel('Frequency (Hz)');
% ylabel('|P1(f)|');

%find harmonics
%findpeaks(P1,'MinPeakDistance',200,'MinPeakHeight',max(P1)/200);
[pks,locs] = findpeaks(P1,'MinPeakDistance',200,'MinPeakHeight',max(P1)/200);
locs=locs';
locs=f(locs);   % get the right freq locations

% use first 3 order calcul the base freq
baseFreq=sum(locs(1:3))/6; % Order 1+2+3=6
orderN=round(locs/baseFreq);
% get scale
loudness = acousticLoudness(y,Fs);  %Sone, sacle by loudness
scale=pks/loudness*100000;

for n=1:length(orderN)
    HarmScale(orderN(n))=scale(n);
end

end




