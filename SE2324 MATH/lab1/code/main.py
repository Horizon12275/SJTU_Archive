import cvxpy as cp
import numpy as np
import matplotlib.pyplot as plt

def create_image(size, radius):
    image = np.ones((size, size)) * 0.99
    center = size // 2
    Y, X = np.ogrid[:size, :size]
    dist_from_center = np.sqrt((X - center)**2 + (Y - center)**2)
    mask = dist_from_center <= radius
    image[mask] = 0.01
    return image

size = 100  # 图像大小
radius = 30  # 圆圈半径
image = create_image(size, radius)

# # Set image as a square image
# size = 100
# image = np.ones((size, size)) * 0.99
# image[25:75, 25:75] = 0.01

# # Set image as a triangle
# size = 100
# image = np.ones((size, size)) * 0.99
# for i in range(25, 75):
#     for j in range(25, i+1):
#         image[i, j] = 0.01

# 假设image是已经给定的图像
x = cp.Variable((size, size), boolean=True)  # 定义优化变量，boolean=True 表示这是一个二进制变量
# 目标函数是要将图像中的黑色区域和白色区域分开
objective = cp.sum(cp.square(cp.multiply(image,x))) + cp.sum(cp.square(cp.multiply(1 - image, 1 - x)))
# 定义优化问题
problem = cp.Problem(cp.Minimize(objective))
# 解决优化问题
problem.solve(solver=cp.ECOS_BB)  # 使用 ECOS_BB 求解器，支持布尔变量
# 获取最优解
optimal_x = x.value
# 可视化原始图像
plt.imshow(image, cmap='gray')
plt.title('Original Image')
plt.show()
# 可视化最优解
plt.imshow(optimal_x.astype(float), cmap='gray')
plt.title('Background Image')
plt.show()