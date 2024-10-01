%read all and harm Analysis all


clc;clear all;close all;



% 预设音频文件名格式
baseFileName = 'hulusiRec%s.wav';

baseFreq=zeros(10,1);
HarmScale=zeros(20,10);
baseFreqRight=[329.6276;391.9954;440.00;493.8833;523.2511;587.3295;659.2551;698.4565;783.9909;880.0000];

% 循环遍历音频文件
for k = 1:10
    % 生成两位数的文件编号
    fileNumber = sprintf('%02d', k);
    
    % 生成完整的文件名
    fileName = sprintf(baseFileName, fileNumber);
    
    % 检查文件是否存在
    if exist(fileName, 'file')
        % 读取音频数据和采样率
        [y, Fs] = audioread(fileName);
        [baseFreq(k),HarmScale(:,k)]=harmAnalysis(y,Fs);

        % % 播放音频
        % sound(y, Fs);
        % 
        % % 显示当前播放的文件名
        % fprintf('Playing %s\n', fileName);
        % 
        % % 等待音频播放完毕再继续
        % pause(length(y)/Fs);




    else
        % 如果文件不存在，显示错误信息
        fprintf('File %s not found.\n', fileName);
    end
end

%   

figure;
colors = lines(10); % 生成10种不同的颜色
for k = 1:10
    x = baseFreq(k) * (1:20); % 计算 x 坐标
    y = HarmScale(:, k); % 获取 y 坐标
    scatter(x, y, 'filled', 'MarkerFaceColor', colors(k,:)); % 绘制散点图，颜色不同
    hold on;
   end
hold off;
