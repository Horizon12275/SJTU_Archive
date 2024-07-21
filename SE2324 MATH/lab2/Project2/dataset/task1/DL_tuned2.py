import tensorflow as tf
import numpy as np
import scipy.io
import cmath

# 加载数据集
train_data = scipy.io.loadmat('./PA_data_train.mat')
test_data = scipy.io.loadmat('./PA_data_test.mat')

# 从复数数据中提取实部和虚部，转换为二维向量
train_input = train_data['paInput']
train_output = train_data['paOutput']

# Convert complex numbers to magnitude and phase
train_input_mag = np.abs(train_input)
train_input_phase = np.angle(train_input)
train_output_mag = np.abs(train_output)
train_output_phase = np.angle(train_output)

# Flatten the arrays to 1D
train_input_mag = train_input_mag.flatten()
train_input_phase = train_input_phase.flatten()
train_output_mag = train_output_mag.flatten()
train_output_phase = train_output_phase.flatten()

# 定义时间延迟（记忆深度）
memory_depth = 5

# 构建输入序列（考虑记忆深度）
def build_input_sequence(data_mag, data_phase, memory_depth):
    input_sequence = []
    for i in range(memory_depth, len(data_mag)):
        input_sequence.append(np.column_stack((data_mag[i-memory_depth:i], data_phase[i-memory_depth:i])))
    return np.array(input_sequence)

# Check if train_input_mag and train_input_phase have more elements than memory_depth
train_input_sequence = build_input_sequence(train_input_mag, train_input_phase, memory_depth)
train_output = np.column_stack((train_output_mag[memory_depth:], train_output_phase[memory_depth:]))

# 构建神经网络模型
model = tf.keras.Sequential([
    tf.keras.layers.LSTM(128, input_shape=(memory_depth, 2), return_sequences=False),
    tf.keras.layers.Dense(512, activation='relu'),
    tf.keras.layers.Dense(2)
])

def custom_loss(y_true, y_pred):
    # Split the output into magnitude and phase
    y_true_mag = y_true[:, 0]
    y_true_phase = y_true[:, 1]
    y_pred_mag = y_pred[:, 0]
    y_pred_phase = y_pred[:, 1]

    # Calculate magnitude error
    magnitude_error = tf.keras.losses.MeanSquaredError()(y_true_mag, y_pred_mag)

    # Calculate phase error, considering the circular nature of phase
    phase_error = tf.keras.losses.MeanSquaredError()(tf.math.cos(y_true_phase), tf.math.cos(y_pred_phase))

    # Return the sum of magnitude and phase errors
    return magnitude_error + phase_error

# 编译模型
model.compile(optimizer='adam', loss=custom_loss)

# 训练模型
model.fit(train_input_sequence, train_output, epochs=10, batch_size=32)

# 测试数据处理与评估
# 创建测试数据的输入序列
test_input = test_data['paInput']
test_output = test_data['paOutput']

# Convert complex numbers to magnitude and phase
test_input_mag = np.abs(test_input)
test_input_phase = np.angle(test_input)
test_output_mag = np.abs(test_output)
test_output_phase = np.angle(test_output)

# Flatten the arrays to 1D
test_input_mag = test_input_mag.flatten()
test_input_phase = test_input_phase.flatten()
test_output_mag = test_output_mag.flatten()
test_output_phase = test_output_phase.flatten()

test_input_sequence = build_input_sequence(test_input_mag, test_input_phase, memory_depth)
test_output = np.column_stack((test_output_mag[memory_depth:], test_output_phase[memory_depth:]))

# 在测试集上进行预测
predictions = model.predict(test_input_sequence)

def calculate_nmse(y_true, y_pred):
    numerator = np.sum(np.square(y_true - y_pred))
    denominator = np.sum(np.square(y_true))
    nmse = 10 * np.log10(numerator / denominator)
    return nmse

# Convert predictions from magnitude and phase back to complex numbers
predictions_complex = predictions[:, 0] * np.exp(1j * predictions[:, 1])

# 将test_output从幅度和相位转换为复数
test_output_complex = test_output[:, 0] * np.exp(1j * test_output[:, 1])

# 计算NMSE
nmse = calculate_nmse(test_output_complex, predictions_complex)
print('Test NMSE:', nmse)

# 将test_input flatten
test_input = test_input.flatten()

# 将预测结果保存为.mat文件
mat_dict = {"paOutput": predictions_complex, "paInput": test_input[memory_depth:]}

# 使用savemat函数保存.mat文件
scipy.io.savemat("output.mat", mat_dict)