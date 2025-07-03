% 读取数据文件
load('PA_data_train.mat'); % 修改为你的数据文件名

% 提取输入和输出数据
inputData = paInput;
outputData = paOutput;

% 设置记忆深度
m = 100; % 修改为你需要的记忆深度

% 构建输入和输出矩阵
X_real = [];
X_imag = [];
Y_real = real(outputData(m+1:end));
Y_imag = imag(outputData(m+1:end));

for i = m+1:length(inputData)
    X_real = [X_real; real(inputData(i-m:i-1))];
    X_imag = [X_imag; imag(inputData(i-m:i-1))];
end

% 使用支持向量机回归模型进行建模
model_real = fitrsvm(X_real, Y_real, 'KernelFunction', 'rbf', 'Standardize', true, 'KernelScale', 'auto');
model_imag = fitrsvm(X_imag, Y_imag, 'KernelFunction', 'rbf', 'Standardize', true, 'KernelScale', 'auto');

% 预测输出值
predictedOutput_real = predict(model_real, X_real);
predictedOutput_imag = predict(model_imag, X_imag);
predictedOutput = predictedOutput_real + 1i*predictedOutput_imag;

% 计算归一化均方误差（NMSE）
NMSE = 10*log10(sum(abs(outputData(m+1:end) - predictedOutput).^2) / sum(abs(outputData(m+1:end)).^2));

% 打印 NMSE 值
fprintf('Normalized Mean Squared Error (NMSE): %.2f dB\n', NMSE);

% 绘制预测结果与实际输出的比较图
figure;
plot(1:length(outputData(m+1:end)), abs(outputData(m+1:end)), 'b', 'LineWidth', 2);
hold on;
plot(1:length(predictedOutput), abs(predictedOutput), 'r--', 'LineWidth', 2);
xlabel('Sample');
ylabel('Normalized Output Amplitude');
title('Comparison of Predicted and Actual Output');
legend('Actual Output', 'Predicted Output');
grid on;
