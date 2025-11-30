import tensorflow as tf
import numpy as np
import scipy.io

# 加载数据集
train_data = scipy.io.loadmat('./PA_data_train.mat')
test_data = scipy.io.loadmat('./PA_data_test.mat')

# 从复数数据中提取实部和虚部，转换为二维向量
train_input_real = train_data['paInput'].real
train_input_imag = train_data['paInput'].imag
train_output_real = train_data['paOutput'].real
train_output_imag = train_data['paOutput'].imag

# Flatten the arrays to 1D
train_input_real = train_input_real.flatten()
train_input_imag = train_input_imag.flatten()
train_output_real = train_output_real.flatten()
train_output_imag = train_output_imag.flatten()

# 定义时间延迟（记忆深度）
memory_depth = 6

# 构建输入序列（考虑记忆深度）
def build_input_sequence(data_real, data_imag, memory_depth):
    input_sequence = []
    for i in range(memory_depth, len(data_real)):
        input_sequence.append(np.column_stack((data_real[i-memory_depth:i], data_imag[i-memory_depth:i])))
    return np.array(input_sequence)

# 创建训练数据的输入序列和输出序列
train_input_sequence = build_input_sequence(train_input_real, train_input_imag, memory_depth)
train_output = np.column_stack((train_output_real[memory_depth:], train_output_imag[memory_depth:]))

# # 构建神经网络模型
model = tf.keras.Sequential([
    tf.keras.layers.LSTM(128, input_shape=(memory_depth, 2), return_sequences=False),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(32, activation='relu'),
    tf.keras.layers.Dense(2),
])

def custom_loss(y_true, y_pred):
    mse = tf.keras.losses.MeanSquaredError()(y_true, y_pred)
    # Split the output into real and imaginary parts
    y_pred_real = y_pred[:, 0]
    y_pred_imag = y_pred[:, 1]
    # Calculate the magnitude of the complex numbers
    y_pred_mag = tf.sqrt(tf.square(y_pred_real) + tf.square(y_pred_imag))
    # 如果预测的幅度大于1.2，则增加惩罚
    penalty = tf.reduce_mean(tf.square(tf.maximum(0.0, y_pred_mag - 1.2)))
    return mse + penalty

# 编译模型
model.compile(optimizer='adam', loss=custom_loss)

# 训练模型
model.fit(train_input_sequence, train_output, epochs=50, batch_size=16)

# 测试数据处理与评估
# 创建测试数据的输入序列
test_input_real = test_data['paInput'].real
test_input_imag = test_data['paInput'].imag
test_output_real = test_data['paOutput'].real
test_output_imag = test_data['paOutput'].imag

# 将复数平展为1D
test_input_real = test_input_real.flatten()
test_input_imag = test_input_imag.flatten()
test_output_real = test_output_real.flatten()
test_output_imag = test_output_imag.flatten()

test_input_sequence = build_input_sequence(test_input_real, test_input_imag, memory_depth)
test_output = np.column_stack((test_output_real[memory_depth:], test_output_imag[memory_depth:]))

# 评估模型
def calculate_nmse(y_true, y_pred):
    numerator = np.sum(np.square(y_true - y_pred))
    denominator = np.sum(np.square(y_true))
    nmse = 10 * np.log10(numerator / denominator)
    return nmse

# 在测试集上进行预测
predictions = model.predict(test_input_sequence)

# 如果预测的幅度大于1.195，则将这个复数的幅度缩放到1.195,通过调整实部和虚部的比例来实现
magnitude = np.sqrt(np.square(predictions[:, 0]) + np.square(predictions[:, 1]))
mask = magnitude > 1.195
predictions[mask, 0] = 1.195 * predictions[mask, 0] / magnitude[mask]
predictions[mask, 1] = 1.195 * predictions[mask, 1] / magnitude[mask]

# 计算NMSE
nmse = calculate_nmse(test_output, predictions)
print('Test NMSE:', nmse)

# 将预测结果重新转换为复数，并保存为.mat文件
new_predictions = np.zeros((len(predictions), 1), dtype=complex)
new_predictions = predictions[:, 0] + 1j * predictions[:, 1]

# 取出测试集中的输入数据、排除前记忆深度个数据
new_test_input = np.transpose(test_data['paInput'])
# new_test_input = np.transpose(train_data['paInput'])
new_test_input = new_test_input[memory_depth:]
new_test_input = np.transpose(new_test_input)

# 创建一个字典，其中键是你想在.mat文件中使用的变量名
mat_dict = {"paOutput": new_predictions, "paInput": new_test_input}

# 使用savemat函数保存.mat文件
scipy.io.savemat("output.mat", mat_dict)