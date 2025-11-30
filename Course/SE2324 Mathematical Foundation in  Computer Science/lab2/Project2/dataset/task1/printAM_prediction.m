% 读取数据文件
load('output.mat'); % 修改为你的数据文件名

% 提取输入和输出数据
inputData = paInput;
outputData = paOutput;

% 计算归一化幅度/幅度
normalizedInputAmplitude = abs(inputData);
normalizedOutputAmplitude = abs(outputData);

% 绘制AM/AM图
figure;
plot(normalizedInputAmplitude, normalizedOutputAmplitude, 'b.');
xlabel('Normalized Input Amplitude');
ylabel('Normalized Output Amplitude');
title('AM/AM Response of Power Amplifier');
grid on;
