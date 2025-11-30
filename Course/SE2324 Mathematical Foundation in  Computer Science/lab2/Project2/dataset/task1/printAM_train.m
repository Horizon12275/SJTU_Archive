% 读取数据文件
load('PA_data_train.mat'); % 修改为你的数据文件名

% 提取输入和输出数据
inputData = paInput;
outputData = paOutput;

% 计算归一化幅度/幅度
normalizedInputAmplitude = abs(inputData);
normalizedOutputAmplitude = abs(outputData);

% 绘制AM/AM图
figure;
subplot(2,1,1); % 创建两个子图中的第一个子图，用于绘制前18000个点
plot(normalizedInputAmplitude(1:18000), normalizedOutputAmplitude(1:18000), 'b.');
xlabel('Normalized Input Amplitude');
ylabel('Normalized Output Amplitude');
title('AM/AM Response of Power Amplifier (First 18000 points)');
grid on;

subplot(2,1,2); % 创建两个子图中的第二个子图，用于绘制剩余的点
plot(normalizedInputAmplitude(18001:end), normalizedOutputAmplitude(18001:end), 'b.');
xlabel('Normalized Input Amplitude');
ylabel('Normalized Output Amplitude');
title('AM/AM Response of Power Amplifier (Remaining points)');
grid on;
