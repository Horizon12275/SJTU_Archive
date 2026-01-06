from openai import OpenAI
from datasets import load_from_disk
import json
import argparse
import base64
from tqdm import tqdm
import os
from PIL import Image, ImageEnhance, ImageFilter
import numpy as np
import cv2

client = OpenAI(
    base_url="http://47.242.151.133:24576/v1/",
    api_key="ml2025",
)

def load_data(path):
    ds = load_from_disk(path)
    return ds

def preprocess_image(example):
    '''
    针对黑白文档的优化预处理：
    1. 增强对比度突出灰白文字
    2. 锐化模糊文字边缘
    3. 自适应二值化处理
    '''
    image = example["image"].convert('L')  # 确保灰度图像
    
    # 对比度增强（针对灰白文字）
    enhancer = ImageEnhance.Contrast(image)
    image = enhancer.enhance(2.5)  # 增强对比度
    
    # 锐化处理（针对模糊文字）
    sharpener = ImageEnhance.Sharpness(image)
    image = sharpener.enhance(3.0)
    
    # 自适应阈值二值化
    image_np = np.array(image)
    image_np = cv2.adaptiveThreshold(
        image_np, 255, 
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
        cv2.THRESH_BINARY, 11, 2
    )
    return Image.fromarray(image_np)

def generate_answer(example):
    image = preprocess_image(example)
    
    # 图像转base64
    tmp_dir = "./tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    tmp_file = os.path.join(tmp_dir, "tmp_image.png")
    image.save(tmp_file, format="PNG")

    with open(tmp_file, "rb") as f:
        encoded_image = base64.b64encode(f.read())
    encoded_image_text = encoded_image.decode("utf-8")
    base64_image = f"data:image;base64,{encoded_image_text}"

    # 优化后的提示词（针对文档问答特点）
    prompt = (
        "Instruction: Analyze this document image and provide ONLY the exact answer "
        "to the following question."
        f"Question: {example['question']}\n"
    )

    chat_response = client.chat.completions.create(
        model="Qwen/Qwen2.5-VL-3B-Instruct",
        messages=[
            {
                "role": "system", 
                "content": "You are a helpful assistant specialized in image understanding."
            },
            {
                "role": "user",
                "content": [
                    {"type": "image_url", "image_url": {"url": base64_image}},
                    {"type": "text", "text": prompt}
                ],
            },
        ],
        temperature=0.1,  # 降低随机性
        max_tokens=32,    # 限制输出长度
    )
    return chat_response.choices[0].message.content.strip()

# ... (保留evaluate_results和main函数不变)

def evaluate_results(results):
    '''
    Evaluate the results
    Args:
        results: list, the results
    Returns:
        score: float, the score
    '''
    # Calculate the score
    score = 0
    for result in results:
        lower_answers = [answer.lower() for answer in result["answers"]]
        if result["generation"].lower() in lower_answers:
            score += 1
    return round(score / len(results), 2)


def main(args):
    if not args.eval_only:
        # Load data
        ds = load_data(args.data_path)

        # Generate
        results = []
        for example in tqdm(ds, desc="Generating answers", total=len(ds), leave=True, position=0):
            print(example)
            answer = generate_answer(example)
            print(answer)
            results.append({"generation": answer, "answers": example['answers']})

        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")

        # Save results to disk
        output_dir = os.path.dirname(args.output_path)
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        with open(args.output_path, "w") as f:
            json.dump(results, f)
    else:
        # Load results
        with open(args.output_path, "r") as f:
            results = json.load(f)
        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", type=str, default="./data/docvqa_100")
    parser.add_argument("--output_path", type=str, default="./results/optimized_results.json")
    parser.add_argument("--eval_only", action="store_true")
    args = parser.parse_args()
    main(args)