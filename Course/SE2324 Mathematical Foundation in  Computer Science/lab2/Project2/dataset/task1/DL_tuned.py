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

# Split the data based on the magnitude of the input data
train_input_magnitude = np.sqrt(train_input_real**2 + train_input_imag**2)
indices_1 = np.where(train_input_magnitude <= 0.5)[0]
indices_2 = np.where(train_input_magnitude > 0.5)[0]

train_input_sequence_1 = build_input_sequence(train_input_real[indices_1], train_input_imag[indices_1], memory_depth)
train_output_1 = np.column_stack((train_output_real[indices_1][memory_depth:], train_output_imag[indices_1][memory_depth:]))

train_input_sequence_2 = build_input_sequence(train_input_real[indices_2], train_input_imag[indices_2], memory_depth)
train_output_2 = np.column_stack((train_output_real[indices_2][memory_depth:], train_output_imag[indices_2][memory_depth:]))

def custom_loss(y_true, y_pred):
    mse = tf.keras.losses.MeanSquaredError()(y_true, y_pred)
    return mse

# 构建神经网络模型
def build_model():
    model = tf.keras.Sequential([
        tf.keras.layers.LSTM(128, input_shape=(memory_depth, 2), return_sequences=False),
        tf.keras.layers.Dense(128, activation='relu'),
        tf.keras.layers.Dense(2)
    ])
    model.compile(optimizer='adam', loss=custom_loss)
    return model

# Train two models
model_1 = build_model()
model_1.fit(train_input_sequence_1, train_output_1, epochs=10, batch_size=32)

model_2 = build_model()
model_2.fit(train_input_sequence_2, train_output_2, epochs=10, batch_size=32)

# 测试数据处理与评估
# 创建测试数据的输入序列
test_input_real = test_data['paInput'].real
test_input_imag = test_data['paInput'].imag
test_output_real = test_data['paOutput'].real
test_output_imag = test_data['paOutput'].imag

# Flatten the arrays to 1D
test_input_real = test_input_real.flatten()
test_input_imag = test_input_imag.flatten()
test_output_real = test_output_real.flatten()
test_output_imag = test_output_imag.flatten()

test_input_sequence = build_input_sequence(test_input_real, test_input_imag, memory_depth)
test_output = np.column_stack((test_output_real[memory_depth:], test_output_imag[memory_depth:]))

# 在测试集上进行预测
test_input_magnitude = np.sqrt(test_input_real[memory_depth:]**2 + test_input_imag[memory_depth:]**2)
predictions_1 = model_1.predict(test_input_sequence[test_input_magnitude <= 0.5])
predictions_2 = model_2.predict(test_input_sequence[test_input_magnitude > 0.5])

# 评估模型
def calculate_nmse(y_true, y_pred):
    numerator = np.sum(np.square(y_true - y_pred))
    denominator = np.sum(np.square(y_true))
    nmse = 10 * np.log10(numerator / denominator)
    return nmse

# 计算NMSE
nmse_1 = calculate_nmse(test_output[test_input_magnitude <= 0.5], predictions_1)
nmse_2 = calculate_nmse(test_output[test_input_magnitude > 0.5], predictions_2)
print('Test NMSE for model 1:', nmse_1)
print('Test NMSE for model 2:', nmse_2)

# 将预测结果重新转换为复数，并保存为.mat文件
new_predictions_1 = predictions_1[:, 0] + 1j * predictions_1[:, 1]
new_predictions_2 = predictions_2[:, 0] + 1j * predictions_2[:, 1]

# Flatten the arrays to 1D
test_input = test_data['paInput'].flatten()

# Create the test_input_magnitude array considering the memory_depth
test_input_magnitude = np.sqrt(test_input[memory_depth:]**2)

# 取出测试集中的输入数据、排除前记忆深度个数据
new_test_input_1 = test_input[memory_depth:][test_input_magnitude <= 0.5]
new_test_input_2 = test_input[memory_depth:][test_input_magnitude > 0.5]
new_test_input_1 = np.transpose(new_test_input_1)
new_test_input_2 = np.transpose(new_test_input_2)

# 创建一个字典，其中键是你想在.mat文件中使用的变量名
mat_dict_1 = {"paOutput": new_predictions_1, "paInput": new_test_input_1}
mat_dict_2 = {"paOutput": new_predictions_2, "paInput": new_test_input_2}

# 使用savemat函数保存.mat文件
scipy.io.savemat("output_1.mat", mat_dict_1)
scipy.io.savemat("output_2.mat", mat_dict_2)