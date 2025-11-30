import cv2
import numpy as np
import os

class ImageComparisonLibrary:
    @staticmethod
    def compare_images(baseline_path, current_path, threshold=0.99):
        """
        使用 SSIM 对比两张图片的相似度
        :param baseline_path: 基准图片路径
        :param current_path: 当前图片路径
        :param threshold: 相似度阈值（默认 0.95）
        :return: True（相似）或 False（不相似）
        """
        # 读取图片
        baseline = cv2.imread(baseline_path)
        current = cv2.imread(current_path)

        if baseline is None or current is None:
            raise FileNotFoundError("基准图片或当前图片不存在")

        # 转换为灰度图
        gray_baseline = cv2.cvtColor(baseline, cv2.COLOR_BGR2GRAY)
        gray_current = cv2.cvtColor(current, cv2.COLOR_BGR2GRAY)

        # 计算 SSIM
        from skimage.metrics import structural_similarity as ssim
        score, _ = ssim(gray_baseline, gray_current, full=True)
        return score >= threshold

    @staticmethod
    def highlight_differences(baseline_path, current_path, output_path):
        """
        高亮显示两张图片的差异，并保存结果
        :param baseline_path: 基准图片路径
        :param current_path: 当前图片路径
        :param output_path: 输出差异图片路径
        """
        baseline = cv2.imread(baseline_path)
        current = cv2.imread(current_path)

        # 计算差异
        difference = cv2.absdiff(baseline, current)
        gray_difference = cv2.cvtColor(difference, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray_difference, 30, 255, cv2.THRESH_BINARY)

        # 高亮差异区域
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        for contour in contours:
            (x, y, w, h) = cv2.boundingRect(contour)
            cv2.rectangle(current, (x, y), (x + w, y + h), (0, 0, 255), 2)

        # 保存结果
        cv2.imwrite(output_path, current)